/*
** thingdef-properties.cpp
**
** Actor denitions - properties and flags handling
**
**---------------------------------------------------------------------------
** Copyright 2002-2007 Christoph Oelckers
** Copyright 2004-2007 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. When not used as part of ZDoom or a ZDoom derivative, this code will be
**    covered by the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or (at
**    your option) any later version.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "gi.h"
#include "filesystem.h"
#include "cmdlib.h"
#include "v_text.h"
#include "thingdef.h"
#include "vmbuilder.h"
#include "types.h"
#include "v_video.h"
#include "texturemanager.h"
#include "coreactor.h"
#include "thingdef.h"

//==========================================================================
//
// Gets a class pointer and performs an error check for correct type
//
//==========================================================================
static PClassActor *FindClassTentative(const char *name, PClass *ancestor, bool optional = false)
{
	// "" and "none" mean 'no class'
	if (name == NULL || *name == 0 || !stricmp(name, "none"))
	{
		return NULL;
	}

	PClass *cls = ancestor->FindClassTentative(name);
	assert(cls != NULL);	// cls can not be NULL here
	if (!cls->IsDescendantOf(ancestor))
	{
		I_Error("%s does not inherit from %s\n", name, ancestor->TypeName.GetChars());
	}
	if (cls->Size == TentativeClass && optional)
	{
		cls->bOptional = true;
	}
	return static_cast<PClassActor *>(cls);
}

//==========================================================================
//
// Sets or clears a flag, taking field width into account.
//
//==========================================================================
void ModActorFlag(DCoreActor *actor, FFlagDef *fd, bool set)
{
	// if it's a CSTAT flag, mark it as protected so that map spawned actors do not override it.
	if (fd->varflags & VARF_Protected)
	{
		static_cast<PClassActor*>(actor->GetClass())->ActorInfo()->DefaultCstat |= fd->flagbit;
	}
	// Little-Endian machines only need one case, because all field sizes
	// start at the same address. (Unless the machine has unaligned access
	// exceptions, in which case you'll need multiple cases for it too.)
#ifdef __BIG_ENDIAN__
	if (fd->fieldsize == 4)
#endif
	{
		uint32_t *flagvar = (uint32_t *)((char *)actor + fd->structoffset);
		if (set)
		{
			*flagvar |= fd->flagbit;
		}
		else
		{
			*flagvar &= ~fd->flagbit;
		}
	}
#ifdef __BIG_ENDIAN__
	else if (fd->fieldsize == 2)
	{
		uint16_t *flagvar = (uint16_t *)((char *)actor + fd->structoffset);
		if (set)
		{
			*flagvar |= fd->flagbit;
		}
		else
		{
			*flagvar &= ~fd->flagbit;
		}
	}
	else
	{
		assert(fd->fieldsize == 1);
		uint8_t *flagvar = (uint8_t *)((char *)actor + fd->structoffset);
		if (set)
		{
			*flagvar |= fd->flagbit;
		}
		else
		{
			*flagvar &= ~fd->flagbit;
		}
	}
#endif
}

//==========================================================================
//
// Finds a flag by name and sets or clears it
//
// Returns true if the flag was found for the actor; else returns false
//
//==========================================================================

bool ModActorFlag(DCoreActor *actor, const FString &flagname, bool set, bool printerror)
{
	bool found = false;

	if (actor != NULL)
	{
		const char *dot = strchr(flagname, '.');
		FFlagDef *fd;
		PClassActor* cls = static_cast<PClassActor*>(actor->GetClass());

		if (dot != NULL)
		{
			FString part1(flagname.GetChars(), dot - flagname);
			fd = FindFlag(cls, part1, dot + 1);
		}
		else
		{
			fd = FindFlag(cls, flagname, NULL);
		}

		if (fd != NULL)
		{
			found = true;

			if (fd->structoffset == -1)
			{
				HandleDeprecatedFlags(actor, cls, set, fd->flagbit);
			}
		}
		else if (printerror)
		{
			DPrintf(DMSG_ERROR, "ACS/DECORATE: '%s' is not a flag in '%s'\n", flagname.GetChars(), cls->TypeName.GetChars());
		}
	}

	return found;
}

//==========================================================================
//
// Returns whether an actor flag is true or not.
//
//==========================================================================

INTBOOL CheckActorFlag(DCoreActor *owner, FFlagDef *fd)
{
	if (fd->structoffset == -1)
	{
		return CheckDeprecatedFlags(owner, static_cast<PClassActor*>(owner->GetClass()), fd->flagbit);
	}
	else
#ifdef __BIG_ENDIAN__
	if (fd->fieldsize == 4)
#endif
	{
		return fd->flagbit & *(uint32_t *)(((char*)owner) + fd->structoffset);
	}
#ifdef __BIG_ENDIAN__
	else if (fd->fieldsize == 2)
	{
		return fd->flagbit & *(uint16_t *)(((char*)owner) + fd->structoffset);
	}
	else
	{
		assert(fd->fieldsize == 1);
		return fd->flagbit & *(uint8_t *)(((char*)owner) + fd->structoffset);
	}
#endif
}

INTBOOL CheckActorFlag(DCoreActor *owner, const char *flagname, bool printerror)
{
	const char *dot = strchr (flagname, '.');
	FFlagDef *fd;
	const PClass *cls = owner->GetClass();

	if (dot != NULL)
	{
		FString part1(flagname, dot-flagname);
		fd = FindFlag (cls, part1, dot+1);
	}
	else
	{
		fd = FindFlag (cls, flagname, NULL);
	}

	if (fd != NULL)
	{
		return CheckActorFlag(owner, fd);
	}
	else
	{
		if (printerror) Printf("Unknown flag '%s' in '%s'\n", flagname, cls->TypeName.GetChars());
		return false;
	}
}

//===========================================================================
//
// HandleDeprecatedFlags
//
// Handles the deprecated flags and sets the respective properties
// to appropriate values. This is solely intended for backwards
// compatibility so mixing this with code that is aware of the real
// properties is not recommended
//
//===========================================================================
void HandleDeprecatedFlags(DCoreActor *defaults, PClassActor *info, bool set, int index)
{
}

//===========================================================================
//
// CheckDeprecatedFlags
//
// Checks properties related to deprecated flags, and returns true only
// if the relevant properties are configured exactly as they would have
// been by setting the flag in HandleDeprecatedFlags.
//
//===========================================================================

bool CheckDeprecatedFlags(DCoreActor *actor, PClassActor *info, int index)
{
	return false; // Any entirely unknown flag is not set
}

//==========================================================================
//
// 
//
//==========================================================================
int MatchString (const char *in, const char **strings)
{
	int i;

	for (i = 0; *strings != NULL; i++)
	{
		if (!stricmp(in, *strings++))
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
// Get access to scripted pointers.
// They need a bit more work than other variables.
//
//==========================================================================

static bool PointerCheck(PType *symtype, PType *checktype)
{
	auto symptype = PType::toClassPointer(symtype);
	auto checkptype = PType::toClassPointer(checktype);
	return symptype != nullptr && checkptype != nullptr && symptype->ClassRestriction->IsDescendantOf(checkptype->ClassRestriction);
}

//==========================================================================
//
// Info Property handlers
//
//==========================================================================

//==========================================================================
//
// Default spritetype fields need to set an additional flag
// so we need native handlers for them
// 
//==========================================================================
DEFINE_PROPERTY(pic, S, CoreActor)
{
	PROP_STRING_PARM(str, 0);
	defaults->spr.picnum = 0; // todo
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_PICNUM;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(statnum, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.statnum = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_STATNUM;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(angle, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.ang = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_ANG;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(xvel, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.xvel = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_XVEL;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(yvel, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.yvel = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_YVEL;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(zvel, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.zvel = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_ZVEL;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(lotag, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.lotag = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_LOTAG;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(hitag, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.lotag = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_HITAG;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(extra, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.lotag = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_EXTRA;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(detail, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.detail = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_DETAIL;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(shade, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.shade = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_SHADE;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(pal, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.pal = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_PAL;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(clipdist, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.clipdist = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_CLIPDIST;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(xrepeat, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.xrepeat = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_XREPEAT;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(yrepeat, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.yrepeat = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_YREPEAT;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(xoffset, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.xoffset = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_XOFFSET;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(yoffset, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.yoffset = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_YOFFSET;
}

//==========================================================================
//
//==========================================================================
DEFINE_PROPERTY(owner, I, CoreActor)
{
	PROP_INT_PARM(i, 0);
	defaults->spr.owner = i;
	bag.Info->ActorInfo()->DefaultCstat |= DEFF_OWNER;
}