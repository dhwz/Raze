//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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
#include "ns.h"
#include "aistuff.h"
#include "sequence.h"
#include "sound.h"
#include "exhumed.h"
#include <assert.h>
#include "engine.h"

BEGIN_PS_NS

static actionSeq MummySeq[] = {
    {8, 0},
    {0, 0},
    {16, 0},
    {24, 0},
    {32, 1},
    {40, 1},
    {48, 1},
    {50, 0}
};


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void BuildMummy(DExhumedActor* pActor, const DVector3& pos, sectortype* pSector, DAngle nAngle)
{
    if (pActor == nullptr)
    {
        pActor = insertActor(pSector, 102);
		pActor->spr.pos = pos;
    }
    else
    {
        nAngle = pActor->spr.Angles.Yaw;
        ChangeActorStat(pActor, 102);
    }

    pActor->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;
    pActor->spr.shade = -12;
    pActor->clipdist = 8;
    pActor->vel.X = 0;
    pActor->vel.Y = 0;
    pActor->vel.Z = 0;
	pActor->spr.scale = DVector2(0.65625, 0.65625);
    pActor->spr.pal = pActor->sector()->ceilingpal;
    pActor->spr.xoffset = 0;
    pActor->spr.yoffset = 0;
    pActor->spr.Angles.Yaw = nAngle;
    pActor->spr.picnum = 1;
    pActor->spr.hitag = 0;
    pActor->spr.lotag = runlist_HeadRun() + 1;
    pActor->spr.extra = -1;

//	GrabTimeSlot(3);

    pActor->nAction = 0;
    pActor->nHealth = 640;
    pActor->nFrame = 0;
    pActor->pTarget = nullptr;
    pActor->nCount = 0;
    pActor->nPhase = Counters[kCountMummy]++;

    pActor->spr.intowner = runlist_AddRunRec(pActor->spr.lotag - 1, pActor, 0xE0000);

    pActor->nRun = runlist_AddRunRec(NewRun, pActor, 0xE0000);

    nCreaturesTotal++;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void CheckMummyRevive(DExhumedActor* pActor)
{
    ExhumedStatIterator it(102);
    while (auto pOther = it.Next())
    {
        if (pOther != pActor)
        {
            if (pOther->nAction != 5) {
                continue;
            }
            double x = abs(pOther->spr.pos.X - pActor->spr.pos.X);
            double y = abs(pOther->spr.pos.Y - pActor->spr.pos.Y);

            if (x <= 320 && y <= 320)
            {
                if (cansee(pActor->spr.pos.plusZ(-32), pActor->sector(), pOther->spr.pos.plusZ(-32), pOther->sector()))
                {
                    pOther->spr.cstat = 0;
                    pOther->nAction = 6;
                    pOther->nFrame = 0;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AIMummy::Tick(RunListEvent* ev)
{
    auto pActor = ev->pObjActor;
    if (!pActor) return;

    DExhumedActor* targ = pActor->pTarget;
    auto pTarget = UpdateEnemy(&targ);
    pActor->pTarget = targ;

    int nAction = pActor->nAction;

    Gravity(pActor);

    int nSeq = SeqOffsets[kSeqMummy] + MummySeq[nAction].a;

    pActor->spr.picnum = seq_GetSeqPicnum2(nSeq, pActor->nFrame);

    int nFrame = SeqBase[nSeq] + pActor->nFrame;
    int nFrameFlag = FrameFlag[nFrame];

    seq_MoveSequence(pActor, nSeq, pActor->nFrame);

    bool bVal = false;

    pActor->nFrame++;
    if (pActor->nFrame >= SeqSize[nSeq])
    {
        pActor->nFrame = 0;

        bVal = true;
    }

    if (pTarget != nullptr && nAction < 4)
    {
        if ((!pTarget->spr.cstat) && nAction)
        {
            pActor->nAction = 0;
            pActor->nFrame = 0;
            pActor->vel.X = 0;
            pActor->vel.Y = 0;
        }
    }

    auto nMov = MoveCreatureWithCaution(pActor);

    if (nAction > 7)
        return;

    switch (nAction)
    {
    case 0:
    {
        if ((pActor->nPhase & 0x1F) == (totalmoves & 0x1F))
        {
            pActor->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;

            if (pTarget == nullptr)
            {
                pTarget = FindPlayer(pActor, 100);
                if (pTarget != nullptr)
                {
                    D3PlayFX(StaticSound[kSound7], pActor);
                    pActor->nFrame = 0;
                    pActor->pTarget = pTarget;
                    pActor->nAction = 1;
                    pActor->nCount = 90;

                    pActor->VelFromAngle(-2);
                }
            }
        }
        return;
    }

    case 1:
    {
        if (pActor->nCount > 0)
        {
            pActor->nCount--;
        }

        if ((pActor->nPhase & 0x1F) == (totalmoves & 0x1F))
        {
            pActor->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;

            PlotCourseToSprite(pActor, pTarget);

            if (pActor->nAction == 1)
            {
                if (RandomBit() && pTarget)
                {
                    if (cansee(pActor->spr.pos.plusZ(-GetActorHeight(pActor)), pActor->sector(),
                        pTarget->spr.pos.plusZ(-GetActorHeight(pTarget)), pTarget->sector()))
                    {
                        pActor->nAction = 3;
                        pActor->nFrame = 0;

                        pActor->vel.X = 0;
                        pActor->vel.Y = 0;
                        return;
                    }
                }
            }
        }

        // loc_2B5A8
        if (!pActor->nFrame)
        {
            pActor->VelFromAngle(-1);
        }

        if (pActor->vel.X != 0 || pActor->vel.Y != 0)
        {
            if (pActor->vel.X > 0)
            {
                pActor->vel.X -= 64;
                if (pActor->vel.X < 0) {
                    pActor->vel.X = 0;
                }
            }
            else if (pActor->vel.X < 0)
            {
                pActor->vel.X += 64;
                if (pActor->vel.X > 0) {
                    pActor->vel.X = 0;
                }
            }

            if (pActor->vel.Y > 0)
            {
				pActor->vel.Y -= 64;
                if (pActor->vel.Y < 0) {
                    pActor->vel.Y = 0;
                }
            }
            else if (pActor->vel.Y < 0)
            {
                pActor->vel.Y += 64;
                if (pActor->vel.Y > 0) {
                    pActor->vel.Y = 0;
                }
            }
        }

        switch (nMov.type)
        {
        case kHitWall:
        {
            pActor->spr.Angles.Yaw += DAngle180 + mapangle(RandomWord() & 0x3FF);
            pActor->VelFromAngle(-2);
            return;
        }

        case kHitSprite:
        {
            if (nMov.actor() == pTarget)
            {
                auto nAngDiff = absangle(pActor->spr.Angles.Yaw, (pTarget->spr.pos - pActor->spr.pos).Angle());
                if (nAngDiff < DAngle22_5 / 2)
                {
                    pActor->nAction = 2;
                    pActor->nFrame = 0;

                    pActor->vel.X = 0;
                    pActor->vel.Y = 0;
                }
            }
            return;
        }
        }

        break;
    }

    case 2:
    {
        if (pTarget == nullptr)
        {
            pActor->nAction = 0;
            pActor->nFrame = 0;
        }
        else
        {
            if (PlotCourseToSprite(pActor, pTarget) >= 64)
            {
                pActor->nAction = 1;
                pActor->nFrame = 0;
            }
            else if (nFrameFlag & 0x80)
            {
                runlist_DamageEnemy(pTarget, pActor, 5);
            }
        }
        return;
    }

    case 3:
    {
        if (bVal)
        {
            pActor->nFrame = 0;
            pActor->nAction = 0;
            pActor->nCount = 100;
            pActor->pTarget = nullptr;
            return;
        }
        else if (nFrameFlag & 0x80)
        {
            SetQuake(pActor, 100);

            // low 16 bits of returned var contains the sprite index, the high 16 the bullet number
            auto pBullet = BuildBullet(pActor, 9, -60, pActor->spr.Angles.Yaw, pTarget, 1);
            CheckMummyRevive(pActor);

            if (pBullet)
            {
                if (!RandomSize(3))
                {
                    SetBulletEnemy(pBullet->nPhase, pTarget);
                    pBullet->spr.pal = 5;
                }
            }
        }
        return;
    }

    case 4:
    {
        if (bVal)
        {
            pActor->nFrame = 0;
            pActor->nAction = 5;
        }
        return;
    }

    case 5:
    {
        pActor->nFrame = 0;
        return;
    }

    case 6:
    {
        if (bVal)
        {
            pActor->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;

            pActor->nAction = 0;
            pActor->nHealth = 300;
            pActor->pTarget = nullptr;

            nCreaturesTotal++;
        }
        return;
    }

    case 7:
    {
        if (nMov.exbits)
        {
            pActor->vel.X *= 0.5;
            pActor->vel.Y *= 0.5;
        }

        if (bVal)
        {
            pActor->vel.X = 0;
            pActor->vel.Y = 0;
            pActor->spr.cstat = CSTAT_SPRITE_BLOCK_ALL;

            pActor->nAction = 0;
            pActor->nFrame = 0;
            pActor->pTarget = nullptr;
        }

        return;
    }
    }
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AIMummy::Draw(RunListEvent* ev)
{
    auto pActor = ev->pObjActor;
    if (!pActor) return;
    int nAction = pActor->nAction;

    seq_PlotSequence(ev->nParam, SeqOffsets[kSeqMummy] + MummySeq[nAction].a, pActor->nFrame, MummySeq[nAction].b);
    return;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AIMummy::RadialDamage(RunListEvent* ev)
{
    auto pActor = ev->pObjActor;
    if (!pActor) return;

    if (pActor->nHealth <= 0)
        return;

    ev->nDamage = runlist_CheckRadialDamage(pActor);
    Damage(ev);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void AIMummy::Damage(RunListEvent* ev)
{
    auto pActor = ev->pObjActor;
    if (!pActor) return;

    if (ev->nDamage <= 0)
        return;

    if (pActor->nHealth <= 0) {
        return;
    }

    pActor->nHealth -= dmgAdjust(ev->nDamage);

    if (pActor->nHealth <= 0)
    {
        pActor->nHealth = 0;
        pActor->spr.cstat &= ~CSTAT_SPRITE_BLOCK_ALL;
        nCreaturesKilled++;

        DropMagic(pActor);

        pActor->nFrame = 0;
        pActor->nAction = 4;

        pActor->vel.X = 0;
        pActor->vel.Y = 0;
        pActor->vel.Z = 0;
        pActor->spr.pos.Z = pActor->sector()->floorz;
    }
    else
    {
        if (!RandomSize(2))
        {
            pActor->nAction = 7;
            pActor->nFrame = 0;

            pActor->vel.X = 0;
            pActor->vel.Y = 0;
        }
    }

    return;
}

END_PS_NS
