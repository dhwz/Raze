//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2000, 2003 - Matt Saettler (EDuke Enhancements)
Copyright (C) 2020 - Christoph Oelckers

This file is part of Enhanced Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms

EDuke enhancements integrated: 04/13/2003 - Matt Saettler

Note: EDuke source was in transition.  Changes are in-progress in the
source as it is released.

*/
//-------------------------------------------------------------------------


#include "ns.h"
#include "global.h"
#include "prediction.h"
#include "dukeactor.h"
#include "gamefuncs.h"
#include "models/modeldata.h"

EXTERN_CVAR(Bool, wt_commentary)

BEGIN_DUKE_NS

void animatesprites_d(tspriteArray& tsprites, const DVector2& viewVec, DAngle viewang, double interpfrac)
{
	tspritetype* t;
	DDukeActor* h;

	for (unsigned j = 0; j < tsprites.Size(); j++)
	{
		t = tsprites.get(j);
		h = static_cast<DDukeActor*>(t->ownerActor);

		if ((!(h->flags2 & SFLAG2_FORCESECTORSHADE) && (t->cstat & CSTAT_SPRITE_ALIGNMENT_WALL)) || (badguy(static_cast<DDukeActor*>(t->ownerActor)) && t->extra > 0) || t->statnum == STAT_PLAYER)
		{
			if (h->sector()->shadedsector == 1 && h->spr.statnum != STAT_ACTOR)
			{
				t->shade = 16;
			}
			continue;
		}

		if (t->sectp != nullptr)
			t->shade = clamp<int>(t->sectp->ceilingstat & CSTAT_SECTOR_SKY ? t->sectp->ceilingshade : t->sectp->floorshade, -127, 127);
	}


	//Between drawrooms() and drawmasks() is the perfect time to animate sprites
	for (unsigned j = 0; j < tsprites.Size(); j++)  
	{
		t = tsprites.get(j);

		h = static_cast<DDukeActor*>(t->ownerActor);
		auto OwnerAc = h->GetOwner();

		if (iseffector(h))
		{
			if (t->lotag == SE_27_DEMO_CAM && ud.recstat == 1)
			{
				t->setspritetexture(TexMan.CheckForTexture("DEMOCAM", ETextureType::Any));
				t->cstat |= CSTAT_SPRITE_YCENTER;
			}
			else
				t->scale = DVector2(0, 0);
			continue;
		}

		if (t->statnum == STAT_TEMP) continue;

		const auto p = getPlayer(h->PlayerIndex());
		const auto spp = getPlayer(screenpeek);

		if ((h->spr.statnum != STAT_ACTOR && h->isPlayer() && p->newOwner == nullptr && h->GetOwner()) || !(h->flags1 & SFLAG_NOINTERPOLATE))
		{
			t->pos = h->interpolatedpos(interpfrac);
			t->Angles.Yaw = h->interpolatedyaw(interpfrac);
		}


		auto sectp = h->sector();
		bool res = CallAnimate(h, t);
		// some actors have 4, some 6 rotation frames - in true Build fashion there's no pointers what to do here without flagging it.
		if ((h->flags2 & SFLAG2_ALWAYSROTATE1) || (t->clipdist & TSPR_ROTATE8FRAMES))
			applyRotation1(h, t, viewang);
		else if ((h->flags2 & SFLAG2_ALWAYSROTATE2) || (t->clipdist & TSPR_ROTATE12FRAMES))
			applyRotation2(h, t, viewang);
		if (sectp->floorpal && !(h->flags2 & SFLAG2_NOFLOORPAL) && !(t->clipdist & TSPR_NOFLOORPAL))
			copyfloorpal(t, sectp);

		if (res)
		{
			if (h->dispictex.isValid())
				h->dispictex = t->spritetexture();
			continue;
		}

		if (h->isPlayer())
		{
			if (t->pal == 1) t->pos.Z -= 18;

			if (p->over_shoulder_on > 0 && p->newOwner == nullptr)
			{
				t->cstat |= CSTAT_SPRITE_TRANSLUCENT;
#if 0 // multiplayer only
				if (screenpeek == myconnectindex && numplayers >= 2)
				{
					t->pos = interpolatedvalue(omypos, mypos, interpfrac).plusZ(gs_playerheight);
					t->angle = interpolatedvalue(omyang, myang, interpfrac);
					t->sector = mycursectnum;
				}
#endif
			}

			if ((display_mirror == 1 || spp != p || !h->GetOwner()) && ud.multimode > 1 && cl_showweapon && h->spr.extra > 0 && p->curr_weapon > 0)
			{
				auto newtspr = tsprites.newTSprite();
				*newtspr = *t;

				newtspr->statnum = STAT_TEMP;

				newtspr->scale.Y = (max(t->scale.Y * 0.125, 0.0625));

				newtspr->shade = t->shade;
				newtspr->cstat = 0;

				const char* texname = nullptr;
				switch (p->curr_weapon)
				{
				case PISTOL_WEAPON:      texname = "FIRSTGUNSPRITE";       break;
				case SHOTGUN_WEAPON:     texname = "SHOTGUNSPRITE";        break;
				case CHAINGUN_WEAPON:    texname = "CHAINGUNSPRITE";       break;
				case RPG_WEAPON:         texname = "RPGSPRITE";            break;
				case HANDREMOTE_WEAPON:	 
				case HANDBOMB_WEAPON:    texname = "HEAVYHBOMB";           break;
				case TRIPBOMB_WEAPON:    texname = "TRIPBOMBSPRITE";       break;
				case GROW_WEAPON:        texname = "GROWSPRITEICON";       break;
				case SHRINKER_WEAPON:    texname = "SHRINKERSPRITE";       break;
				case FREEZE_WEAPON:      texname = "FREEZESPRITE";         break;
				case FLAMETHROWER_WEAPON:texname = "FLAMETHROWERSPRITE";   break;
				case DEVISTATOR_WEAPON:  texname = "DEVISTATORSPRITE";     break;
				}
				t->setspritetexture(TexMan.CheckForTexture(texname, ETextureType::Any));

				if (h->GetOwner()) newtspr->pos.Z = h->getOffsetZ() - 12;
				else newtspr->pos.Z = h->spr.pos.Z - 51;
				if (p->curr_weapon == HANDBOMB_WEAPON)
				{
					newtspr->scale = DVector2(0.15625, 0.15625);
				}
				else
				{
					newtspr->scale = DVector2(0.25, 0.25);
				}
				newtspr->pal = 0;
			}

			if (!h->GetOwner())
			{
				FTextureID base = FNullTextureID();
				if (t->sectp->lotag == ST_2_UNDERWATER) base = TexMan.CheckForTexture("APLAYERSWIMMING", ETextureType::Any);
				else if ((h->floorz - h->spr.pos.Z) > 64) base = TexMan.CheckForTexture("APLAYERJUMP", ETextureType::Any);
				if (!base.isValid()) base = h->spr.spritetexture();

				applyRotation1(h, t, viewang, base);


				t->pal = p->palookup;
				continue;
			}
			if (p->on_crane == nullptr && (h->sector()->lotag & 0x7ff) != 1)
			{
				double v = h->spr.pos.Z - h->floorz + 3;
				if (v > 4 && h->spr.scale.Y > 0.5 && h->spr.extra > 0)
					h->spr.yoffset = (int8_t)(v / h->spr.scale.Y);
				else h->spr.yoffset = 0;
			}

			if (ud.cameraactor == nullptr && p->newOwner == nullptr)
				if (h->GetOwner() && display_mirror == 0 && p->over_shoulder_on == 0)
					if (ud.multimode < 2 || (ud.multimode > 1 && p == spp))
					{
						t->ownerActor = nullptr;
						t->scale = DVector2(0, 0);
						continue;
					}


			if (sectp->floorpal)
				copyfloorpal(t, sectp);

			if (t->pos.Z > h->floorz && t->scale.X < 0.5)
				t->pos.Z = h->floorz;

		}

		applyanimations(t, h, viewVec, viewang);

		if (h->spr.statnum == STAT_DUMMYPLAYER || badguy(h) || (h->isPlayer() && h->GetOwner()))
		{
			drawshadows(tsprites, t, h);
			if (spp->heat_amount > 0 && spp->heat_on)
			{
				t->pal = 6;
				t->shade = 0;
			}
		}

		h->dispictex = t->spritetexture();
		if (t->sectp->floortexture == mirrortex)
			t->scale = DVector2(0, 0);
	}
}


END_DUKE_NS
