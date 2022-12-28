//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#include "serializer.h"
#include "maptypes.h"

#ifdef NOONE_EXTENSIONS

BEGIN_BLD_NS

TArray<DBloodActor*> getSpritesNearWalls(sectortype* pSrcSect, double nDist);

// - CLASSES ------------------------------------------------------------------

// SPRITES_NEAR_SECTORS
// Intended for move sprites that is close to the outside walls with
// TranslateSector and/or zTranslateSector similar to Powerslave(Exhumed) way
// --------------------------------------------------------------------------
class SPRINSECT
{
public:
    static const int kMaxSprNear = 256;

    struct SPRITES
    {
        unsigned int nSector;
        TArray<TObjPtr<DBloodActor*>> pActors;
    };
private:
    TArray<SPRITES> db;
public:
    void Free() { db.Clear(); }
    void Init(double nDist = 1); // used in trInit to collect the sprites before translation
    void Serialize(FSerializer& pSave);
    TArray<TObjPtr<DBloodActor*>>* GetSprPtr(int nSector);
	void Mark()
	{
		for (auto& entry : db) GC::MarkArray(entry.pActors);
	}

};


// SPRITES_NEAR_SECTORS
// Intended for move sprites that is close to the outside walls with
// TranslateSector and/or zTranslateSector similar to Powerslave(Exhumed) way
// --------------------------------------------------------------------------
SPRINSECT gSprNSect;

void MarkSprInSect()
{
	gSprNSect.Mark();
}

void SPRINSECT::Init(double nDist)
{
    Free();

    int j;
    for(auto&sect : sector)
    {
        if (!isMovableSector(sect.type))
            continue;

        switch (sect.type) {
        case kSectorZMotionSprite:
        case kSectorSlideMarked:
        case kSectorRotateMarked:
            continue;
            // only allow non-marked sectors
        default:
            break;
        }

        auto collected = getSpritesNearWalls(&sect, nDist);

        // exclude sprites that is not allowed
        for (j = collected.Size() - 1; j >= 0; j--)
        {
            auto pActor = collected[j];
            if ((pActor->spr.cstat & CSTAT_SPRITE_MOVE_MASK) && pActor->insector())
            {
                // if *next* sector is movable, exclude to avoid fighting
                if (!isMovableSector(pActor->sector()->type))
                {
                    switch (pActor->spr.statnum) 
                    {
                    default:
                        continue;
                    case kStatMarker:
                    case kStatPathMarker:
                        if (pActor->spr.flags & 0x1) continue;
                        [[fallthrough]];
                    case kStatDude:
                        break;
                    }
                }
            }

            collected.Delete(j);
        }

        if (collected.Size())
        {
            db.Resize(db.Size() + 1);

            SPRITES& pEntry = db.Last();
            pEntry.pActors.Resize(collected.Size());
            for (unsigned ii = 0; ii < collected.Size(); ii++)
                pEntry.pActors[ii] = collected[ii];
            pEntry.nSector = sectindex(&sect);
        }
    }
}

TArray<TObjPtr<DBloodActor*>>* SPRINSECT::GetSprPtr(int nSector)
{
    for (auto &spre : db)
    {
        if (spre.nSector == (unsigned int)nSector && spre.pActors.Size() > 0)
            return &spre.pActors;
    }
    return nullptr;
}


FSerializer& Serialize(FSerializer& arc, const char* key, SPRINSECT::SPRITES& obj, SPRINSECT::SPRITES* defval)
{
    if (arc.BeginObject(key))
    {
        arc("sector", obj.nSector)
            ("actors", obj.pActors)
            .EndObject();
    }
    return arc;
}

void SPRINSECT::Serialize(FSerializer& arc)
{
    arc("db", db);
}



bool isMovableSector(int nType)
{
    return (nType && nType != kSectorDamage && nType != kSectorTeleport && nType != kSectorCounter);
}

bool isMovableSector(sectortype* pSect)
{
    if (isMovableSector(pSect->type) && pSect->hasX())
    {
        return (pSect->xs().busy && !pSect->xs().unused1);
    }
    return false;
}

TArray<DBloodActor*> getSpritesNearWalls(sectortype* pSrcSect, double nDist)
{
    TArray<DBloodActor*> out;

    for(auto& wal : pSrcSect->walls)
    {
        if (!wal.twoSided())
            continue;

		auto wpos = wal.pos;
		auto wlen = wal.delta();
 
        BloodSectIterator it(wal.nextsector);
        while(auto ac = it.Next())
        {
            if (out.Find(ac))
                continue;

			auto spos = ac->spr.pos.XY();
			auto qpos = spos - wpos;
			
			double num = qpos.dot(wlen);
			double den = wal.Length();

            if (num > 0 && num < den)
            {
				auto vi = wpos + wlen * num / den;

                if ((vi - spos).LengthSquared() <= nDist * nDist)
                {
                    out.Push(ac);
                }
            }
        }
    }
    return out;
}

END_BLD_NS

#endif
