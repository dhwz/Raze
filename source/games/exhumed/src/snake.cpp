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
#include "engine.h"
#include "exhumed.h"
#include "aistuff.h"
#include "status.h"
#include "player.h"
#include "sequence.h"
#include "input.h"
#include "sound.h"
#include <string.h>
#include <assert.h>

BEGIN_PS_NS

FreeListArray<Snake, kMaxSnakes> SnakeList;

int16_t nPlayerSnake[kMaxPlayers];

size_t MarkSnake()
{
    for (int i = 0; i < kMaxSnakes; i++)
    {
        GC::Mark(SnakeList[i].pEnemy);
        GC::MarkArray(SnakeList[i].pSprites, kSnakeSprites);
    }
    return kMaxSnakes * (1 + kSnakeSprites);
}

FSerializer& Serialize(FSerializer& arc, const char* keyname, Snake& w, Snake* def)
{
    if (arc.BeginObject(keyname))
    {
        arc("enemy", w.pEnemy)
			("countdown", w.nCountdown)
            .Array("sprites", w.pSprites, kSnakeSprites)
            ("run", w.nRun)
            .Array("c", w.c, countof(w.c))
            ("se", w.nAngle)
            ("player", w.nSnakePlayer)
            .EndObject();
    }
    return arc;
}

void SerializeSnake(FSerializer& arc)
{
    arc("snake", SnakeList);
    arc.Array("playersnake", nPlayerSnake, PlayerCount);
}


void InitSnakes()
{
    SnakeList.Clear();
    memset(nPlayerSnake, 0, sizeof(nPlayerSnake));
}

int GrabSnake()
{
    return SnakeList.Get();
}

void DestroySnake(int nSnake)
{
    int nRun = SnakeList[nSnake].nRun;
    runlist_SubRunRec(nRun);

    for (int i = 0; i < kSnakeSprites; i++)
    {
        DExhumedActor* pSnake = SnakeList[nSnake].pSprites[i];
        if (!pSnake) continue;

        runlist_DoSubRunRec(pSnake->spr.lotag - 1);
        runlist_DoSubRunRec(pSnake->spr.intowner);

        DeleteActor(pSnake);
    }

    SnakeList.Release(nSnake);

    if (nSnake == nSnakeCam)
    {
        nSnakeCam = -1;
    }
}

void ExplodeSnakeSprite(DExhumedActor* pActor, int nPlayer)
{
    int nDamage = BulletInfo[kWeaponStaff].nDamage;

    if (PlayerList[nPlayer].nDouble > 0) {
        nDamage *= 2;
    }

    // take a copy of this, to revert after call to runlist_RadialDamageEnemy()
    DExhumedActor* nOwner = pActor->pTarget;
    pActor->pTarget = PlayerList[nPlayer].pActor;

    runlist_RadialDamageEnemy(pActor, nDamage, BulletInfo[kWeaponStaff].nRadius);

    pActor->pTarget = nOwner;

    BuildAnim(nullptr, 23, 0, pActor->spr.pos, pActor->sector(), 40, 4);

    AddFlash(pActor->sector(), pActor->spr.pos, 128);

    StopActorSound(pActor);
}

void BuildSnake(int nPlayer, int zVal)
{

    zVal -= 1280;

    auto pPlayerActor = PlayerList[nPlayer].pActor;
    auto pViewSect = PlayerList[nPlayer].pPlayerViewSect;
    int nPic = seq_GetSeqPicnum(kSeqSnakBody, 0, 0);

    int x = pPlayerActor->int_pos().X;
    int y = pPlayerActor->int_pos().Y;
    int z = (pPlayerActor->int_pos().Z + zVal) - 2560;
    int nAngle = pPlayerActor->int_ang();

    HitInfo hit{};
    hitscan(vec3_t( x, y, z ), pPlayerActor->sector(), { bcos(nAngle), bsin(nAngle), 0 }, hit, CLIPMASK1);

    uint32_t yDiff = abs(hit.int_hitpos().Y - y);
    uint32_t xDiff = abs(hit.int_hitpos().X - x);

    uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

    if (sqrtNum > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        sqrtNum = INT_MAX;
    }

    int nSqrt = ksqrt(sqrtNum);

    if (nSqrt < bsin(512, -4))
    {
		hit.hitpos -= pPlayerActor->spr.angle.ToVector() * 0.5;
        auto pActor = insertActor(hit.hitSector, 202);
        pActor->spr.pos = hit.hitpos;

        ExplodeSnakeSprite(pActor, nPlayer);
        DeleteActor(pActor);
        return;
    }
    else
    {
        DExhumedActor* pTarget = nullptr;
        auto hitactor = hit.actor();
        if (hitactor && hitactor->spr.statnum >= 90 && hitactor->spr.statnum <= 199) {
            pTarget = hitactor;
        }
        else if (sPlayerInput[nPlayer].pTarget != nullptr) 
        {
            pTarget = sPlayerInput[nPlayer].pTarget;
        }

        int nSnake = GrabSnake();
        if (nSnake == -1) return;

        //		GrabTimeSlot(3);

        DExhumedActor* sprt = nullptr;

        for (int i = 0; i < kSnakeSprites; i++)
        {
            auto pActor = insertActor(pViewSect, 202);

			pActor->pTarget = pPlayerActor;
            //pActor->spr.intowner = nPlayerSprite;
            pActor->spr.picnum = nPic;

            if (i == 0)
            {
                pActor->spr.pos = pPlayerActor->spr.pos.plusZ(zVal * zinttoworld);
                pActor->spr.xrepeat = 32;
                pActor->spr.yrepeat = 32;
                pViewSect = pActor->sector();
                sprt = pActor;
            }
            else
            {
				pActor->spr.pos = sprt->spr.pos;
                pActor->spr.xrepeat = 40 - 3 * i;
                pActor->spr.yrepeat = 40 - 3 * i;
            }

            pActor->spr.clipdist = 10;
            pActor->spr.cstat = 0;
            pActor->spr.shade = -64;
            pActor->spr.pal = 0;
            pActor->spr.xoffset = 0;
            pActor->spr.yoffset = 0;
            pActor->spr.angle = pPlayerActor->spr.angle;
            pActor->vel.X = 0;
            pActor->vel.Y = 0;
            pActor->vel.Z = 0;
            pActor->spr.hitag = 0;
            pActor->spr.extra = -1;
            pActor->spr.lotag = runlist_HeadRun() + 1;
            pActor->backuppos();

            SnakeList[nSnake].pSprites[i] = pActor;

            pActor->spr.intowner = runlist_AddRunRec(pActor->spr.lotag - 1, ((nSnake << 8) | i), 0x110000);
        }

        SnakeList[nSnake].nRun = runlist_AddRunRec(NewRun, nSnake, 0x110000);
        SnakeList[nSnake].c[1] = 2;
        SnakeList[nSnake].c[5] = 5;
        SnakeList[nSnake].c[2] = 4;
        SnakeList[nSnake].c[3] = 6;
        SnakeList[nSnake].c[4] = 7;
        SnakeList[nSnake].c[6] = 6;
        SnakeList[nSnake].c[7] = 7;
        SnakeList[nSnake].pEnemy = pTarget;
		SnakeList[nSnake].nCountdown = 0;
        SnakeList[nSnake].nAngle = 0;
        SnakeList[nSnake].nSnakePlayer = nPlayer;
        nPlayerSnake[nPlayer] = nSnake;

        if (bSnakeCam)
        {
            if (nSnakeCam < 0) {
                nSnakeCam = nSnake;
            }
        }

        D3PlayFX(StaticSound[kSound6], sprt);
    }
}

DExhumedActor* FindSnakeEnemy(int nSnake)
{
    int nPlayer = SnakeList[nSnake].nSnakePlayer;
	auto pPlayerActor = PlayerList[nPlayer].pActor;

    DExhumedActor* pActor = SnakeList[nSnake].pSprites[0]; // CHECKME
    if (!pActor) return nullptr;

    int nAngle = pActor->int_ang();
    auto pSector =pActor->sector();

    int esi = 2048;

	DExhumedActor* pEnemy = nullptr;

    ExhumedSectIterator it(pSector);
    while (auto pAct2 = it.Next())
    {
        if (pAct2->spr.statnum >= 90 && pAct2->spr.statnum < 150 && (pAct2->spr.cstat & CSTAT_SPRITE_BLOCK_ALL))
        {
            if (pAct2 != pPlayerActor && !(pAct2->spr.cstat & CSTAT_SPRITE_INVISIBLE))
            {
                int nAngle2 = (nAngle - GetAngleToSprite(pActor, pAct2)) & kAngleMask;
                if (nAngle2 < esi)
                {
                    pEnemy = pAct2;
                    esi = nAngle2;
                }
            }
        }
    }

    if (pEnemy)
    {
        SnakeList[nSnake].pEnemy = pEnemy;
        SnakeList[nSnake].nCountdown = 0;
    }
    else
    {
        SnakeList[nSnake].nCountdown--;
        if (SnakeList[nSnake].nCountdown < -25)
        {
            pEnemy = pPlayerActor;
            SnakeList[nSnake].pEnemy = pPlayerActor;
        }
    }

    return pEnemy;
}

void AISnake::Tick(RunListEvent* ev)
{
    int nSnake = RunData[ev->nRun].nObjIndex;
    assert(nSnake >= 0 && nSnake < kMaxSnakes);

    DExhumedActor* pActor = SnakeList[nSnake].pSprites[0];
    if (!pActor) return;

    seq_MoveSequence(pActor, SeqOffsets[kSeqSnakehed], 0);

    DExhumedActor* pEnemySprite = SnakeList[nSnake].pEnemy;

    Collision nMov;
    int zVal;

    if (pEnemySprite == nullptr)
    {
    SEARCH_ENEMY:
        nMov = movesprite(pActor,
            600 * bcos(pActor->int_ang()),
            600 * bsin(pActor->int_ang()),
            bsin(SnakeList[nSnake].nAngle, -5),
            0, 0, CLIPMASK1);

        FindSnakeEnemy(nSnake);

        zVal = 0;
    }
    else
    {
        if (!(pEnemySprite->spr.cstat & CSTAT_SPRITE_BLOCK_ALL))
        {
            SnakeList[nSnake].pEnemy = nullptr;
            goto SEARCH_ENEMY;
        }

        zVal = pActor->int_pos().Z;

        nMov = AngleChase(pActor, pEnemySprite, 1200, SnakeList[nSnake].nAngle, 32);

        zVal = pActor->int_pos().Z - zVal;
    }

    if (nMov.type || nMov.exbits)
    {
        int nPlayer = SnakeList[nSnake].nSnakePlayer;
        ExplodeSnakeSprite(SnakeList[nSnake].pSprites[0], nPlayer);

        nPlayerSnake[nPlayer] = -1;
        SnakeList[nSnake].nSnakePlayer = -1;

        DestroySnake(nSnake);
    }
    else
    {
        int nAngle = pActor->int_ang();
        int var_30 = -bcos(nAngle, 6);
        int var_34 = -bsin(nAngle, 6);

        int var_20 = SnakeList[nSnake].nAngle;

        SnakeList[nSnake].nAngle = (SnakeList[nSnake].nAngle + 64) & 0x7FF;

        int var_28 = (nAngle + 512) & kAngleMask;
        auto pSector = pActor->sector();

        for (int i = 7; i > 0; i--)
        {
            DExhumedActor* pActor2 = SnakeList[nSnake].pSprites[i];
            if (!pActor2) continue;

            pActor2->set_int_ang(nAngle);
			pActor2->spr.pos = pActor->spr.pos;

            ChangeActorSect(pActor2, pSector);

            int eax = (bsin(var_20) * SnakeList[nSnake].c[i]) >> 9;

            movesprite(pActor2, var_30 + var_30 * i + eax * bcos(var_28), var_30 + var_34 * i + eax * bsin(var_28),
                -zVal * (i - 1), 0, 0, CLIPMASK1);

            var_20 = (var_20 + 128) & kAngleMask;
        }
    }
}

void AISnake::Draw(RunListEvent* ev)
{
    int nSnake = RunData[ev->nRun].nObjIndex;
    int nSprite = ev->nParam;

    if ((nSnake & 0xFF) == 0) {
        seq_PlotSequence(nSprite, SeqOffsets[kSeqSnakehed], 0, 0);
    }
    else {
        seq_PlotSequence(nSprite, SeqOffsets[kSeqSnakBody], 0, 0);
    }

    ev->pTSprite->ownerActor = nullptr;
}

END_PS_NS
