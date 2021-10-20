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
#include "player.h"
#include "view.h"
#include "status.h"
#include "sound.h"
#include "mapinfo.h"
#include <string.h>
#include <assert.h>


BEGIN_PS_NS

short NearSector[kMaxSectors] = { 0 };

short nPushBlocks;

// TODO - moveme?
short overridesect;
short NearCount = -1;

DExhumedActor* nBodySprite[50];

int hihit, sprceiling, sprfloor, lohit;

enum
{
	kMaxPushBlocks	= 100,
	kMaxMoveChunks	= 75
};

// think this belongs in init.c?
BlockInfo sBlockInfo[kMaxPushBlocks];

DExhumedActor *nChunkSprite[kMaxMoveChunks];

FSerializer& Serialize(FSerializer& arc, const char* keyname, BlockInfo& w, BlockInfo* def)
{
    if (arc.BeginObject(keyname))
    {
        arc("at8", w.field_8)
            ("sprite", w.nSprite)
            ("x", w.x)
            ("y", w.y)
            .EndObject();
    }
    return arc;
}

void SerializeMove(FSerializer& arc)
{
    if (arc.BeginObject("move"))
    {
        arc("nearcount", NearCount)
            .Array("nearsector", NearSector, NearCount)
            ("pushcount", nPushBlocks)
            .Array("blocks", sBlockInfo, nPushBlocks)
            ("chunkcount", nCurChunkNum)
            .Array("chunks", nChunkSprite, kMaxMoveChunks)
            ("overridesect", overridesect)
            ("hihit", hihit)
            ("lohit", lohit)
            ("sprceiling", sprceiling)
            ("sprfloor", sprfloor)
            .Array("bodysprite", nBodySprite, 50)
            .EndObject();
    }
}

signed int lsqrt(int a1)
{
    int v1;
    int v2;
    signed int result;

    v1 = a1;
    v2 = a1 - 0x40000000;

    result = 0;

    if (v2 >= 0)
    {
        result = 32768;
        v1 = v2;
    }
    if (v1 - ((result << 15) + 0x10000000) >= 0)
    {
        v1 -= (result << 15) + 0x10000000;
        result += 16384;
    }
    if (v1 - ((result << 14) + 0x4000000) >= 0)
    {
        v1 -= (result << 14) + 0x4000000;
        result += 8192;
    }
    if (v1 - ((result << 13) + 0x1000000) >= 0)
    {
        v1 -= (result << 13) + 0x1000000;
        result += 4096;
    }
    if (v1 - ((result << 12) + 0x400000) >= 0)
    {
        v1 -= (result << 12) + 0x400000;
        result += 2048;
    }
    if (v1 - ((result << 11) + 0x100000) >= 0)
    {
        v1 -= (result << 11) + 0x100000;
        result += 1024;
    }
    if (v1 - ((result << 10) + 0x40000) >= 0)
    {
        v1 -= (result << 10) + 0x40000;
        result += 512;
    }
    if (v1 - ((result << 9) + 0x10000) >= 0)
    {
        v1 -= (result << 9) + 0x10000;
        result += 256;
    }
    if (v1 - ((result << 8) + 0x4000) >= 0)
    {
        v1 -= (result << 8) + 0x4000;
        result += 128;
    }
    if (v1 - ((result << 7) + 4096) >= 0)
    {
        v1 -= (result << 7) + 4096;
        result += 64;
    }
    if (v1 - ((result << 6) + 1024) >= 0)
    {
        v1 -= (result << 6) + 1024;
        result += 32;
    }
    if (v1 - (32 * result + 256) >= 0)
    {
        v1 -= 32 * result + 256;
        result += 16;
    }
    if (v1 - (16 * result + 64) >= 0)
    {
        v1 -= 16 * result + 64;
        result += 8;
    }
    if (v1 - (8 * result + 16) >= 0)
    {
        v1 -= 8 * result + 16;
        result += 4;
    }
    if (v1 - (4 * result + 4) >= 0)
    {
        v1 -= 4 * result + 4;
        result += 2;
    }
    if (v1 - (2 * result + 1) >= 0)
        result += 1;

    return result;
}

void MoveThings()
{
    thinktime.Reset();
    thinktime.Clock();

    UndoFlashes();
    DoLights();

    if (nFreeze)
    {
        if (nFreeze == 1 || nFreeze == 2) {
            DoSpiritHead();
        }
    }
    else
    {
        actortime.Reset();
        actortime.Clock();
        runlist_ExecObjects();
        runlist_CleanRunRecs();
        actortime.Unclock();
    }

    DoBubbleMachines();
    DoDrips();
    DoMovingSects();
    DoRegenerates();

    if (currentLevel->gameflags & LEVEL_EX_COUNTDOWN)
    {
        DoFinale();
        if (lCountDown < 1800 && nDronePitch < 2400 && !lFinaleStart)
        {
            nDronePitch += 64;
            BendAmbientSound();
        }
    }

    thinktime.Unclock();
}

void ResetMoveFifo()
{
    movefifoend = 0;
    movefifopos = 0;
}

// not used
void clipwall()
{

}

void BuildNear(int x, int y, int walldist, int nSector)
{
    NearSector[0] = nSector;
    NearCount = 1;

    int i = 0;

    while (i < NearCount)
    {
        short nSector = NearSector[i];

        short nWall = sector[nSector].wallptr;
        short nWallCount = sector[nSector].wallnum;

        while (1)
        {
            nWallCount--;
            if (nWallCount < 0)
            {
                i++;
                break;
            }

            short nNextSector = wall[nWall].nextsector;

            if (nNextSector >= 0)
            {
                int j = 0;
                for (; j < NearCount; j++)
                {
                    // loc_14F4D:
                    if (nNextSector == NearSector[j])
                        break;
                }

                if (j >= NearCount)
                {
                    vec2_t pos = { x, y };
                    if (clipinsidebox(&pos, nWall, walldist))
                    {
                        NearSector[NearCount] = wall[nWall].nextsector;
                        NearCount++;
                    }
                }
            }

            nWall++;
        }
    }
}

int BelowNear(DExhumedActor* pActor)
{
	auto pSprite = &pActor->s();
    short nSector = pSprite->sectnum;
    int z = pSprite->z;

    int var_24, z2;

    if ((lohit & 0xC000) == 0xC000)
    {
        var_24 = lohit & 0xC000;
        z2 = sprite[lohit & 0x3FFF].z;
    }
    else
    {
        var_24 = 0x20000;
        z2 = sector[nSector].floorz + SectDepth[nSector];

        if (NearCount > 0)
        {
            short edx;

            for (int i = 0; i < NearCount; i++)
            {
                int nSect2 = NearSector[i];

                while (nSect2 >= 0)
                {
                    edx = nSect2;
                    nSect2 = SectBelow[nSect2];
                }

                int ecx = sector[edx].floorz + SectDepth[edx];
                int eax = ecx - z;

                if (eax < 0 && eax >= -5120)
                {
                    z2 = ecx;
                    nSector = edx;
                }
            }
        }
    }

    if (z2 < pSprite->z)
    {
        pSprite->z = z2;
        overridesect = nSector;
        pSprite->zvel = 0;

        bTouchFloor = true;

        return var_24;
    }
    else
    {
        return 0;
    }
}

int movespritez(short nSprite, int z, int height, int, int clipdist)
{
    spritetype* pSprite = &sprite[nSprite];
    short nSector = pSprite->sectnum;
    assert(nSector >= 0 && nSector < kMaxSectors);

    overridesect = nSector;
    short edi = nSector;

    // backup cstat
    uint16_t cstat = pSprite->cstat;

    pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;

    int nRet = 0;

    short nSectFlags = SectFlag[nSector];

    if (nSectFlags & kSectUnderwater) {
        z >>= 1;
    }

    int spriteZ = pSprite->z;
    int floorZ = sector[nSector].floorz;

    int ebp = spriteZ + z;
    int eax = sector[nSector].ceilingz + (height >> 1);

    if ((nSectFlags & kSectUnderwater) && ebp < eax) {
        ebp = eax;
    }

    // loc_151E7:
    while (ebp > sector[pSprite->sectnum].floorz && SectBelow[pSprite->sectnum] >= 0)
    {
        edi = SectBelow[pSprite->sectnum];

        mychangespritesect(nSprite, edi);
    }

    if (edi != nSector)
    {
        pSprite->z = ebp;

        if (SectFlag[edi] & kSectUnderwater)
        {
            if (nSprite == PlayerList[nLocalPlayer].nSprite) {
                D3PlayFX(StaticSound[kSound2], nSprite);
            }

            if (pSprite->statnum <= 107) {
                pSprite->hitag = 0;
            }
        }
    }
    else
    {
        while ((ebp < sector[pSprite->sectnum].ceilingz) && (SectAbove[pSprite->sectnum] >= 0))
        {
            edi = SectAbove[pSprite->sectnum];

            mychangespritesect(nSprite, edi);
        }
    }

    // This function will keep the player from falling off cliffs when you're too close to the edge.
    // This function finds the highest and lowest z coordinates that your clipping BOX can get to.
    getzrange_old(pSprite->x, pSprite->y, pSprite->z - 256, pSprite->sectnum,
        &sprceiling, &hihit, &sprfloor, &lohit, 128, CLIPMASK0);

    int mySprfloor = sprfloor;

    if ((lohit & 0xC000) != 0xC000) {
        mySprfloor += SectDepth[pSprite->sectnum];
    }

    if (ebp > mySprfloor)
    {
        if (z > 0)
        {
            bTouchFloor = true;

            if ((lohit & 0xC000) == 0xC000)
            {
                // Path A
                short nFloorSprite = lohit & 0x3FFF;

                if (pSprite->statnum == 100 && sprite[nFloorSprite].statnum != 0 && sprite[nFloorSprite].statnum < 100)
                {
                    short nDamage = (z >> 9);
                    if (nDamage)
                    {
                        runlist_DamageEnemy(nFloorSprite, nSprite, nDamage << 1);
                    }

                    pSprite->zvel = -z;
                }
                else
                {
                    if (sprite[nFloorSprite].statnum == 0 || sprite[nFloorSprite].statnum > 199)
                    {
                        nRet |= 0x20000;
                    }
                    else
                    {
                        nRet |= lohit;
                    }

                    pSprite->zvel = 0;
                }
            }
            else
            {
                // Path B
                if (SectBelow[pSprite->sectnum] == -1)
                {
                    nRet |= 0x20000;

                    short nSectDamage = SectDamage[pSprite->sectnum];

                    if (nSectDamage != 0)
                    {
                        if (pSprite->hitag < 15)
                        {
                            IgniteSprite(nSprite);
                            pSprite->hitag = 20;
                        }
                        nSectDamage >>= 2;
                        nSectDamage = nSectDamage - (nSectDamage>>2);
                        if (nSectDamage) {
                            runlist_DamageEnemy(nSprite, -1, nSectDamage);
                        }
                    }

                    pSprite->zvel = 0;
                }
            }
        }

        // loc_1543B:
        ebp = mySprfloor;
        pSprite->z = mySprfloor;
    }
    else
    {
        if ((ebp - height) < sprceiling && ((hihit & 0xC000) == 0xC000 || SectAbove[pSprite->sectnum] == -1))
        {
            ebp = sprceiling + height;
            nRet |= 0x10000;
        }
    }

    if (spriteZ <= floorZ && ebp > floorZ)
    {
        if ((SectDepth[nSector] != 0) || (edi != nSector && (SectFlag[edi] & kSectUnderwater)))
        {
            assert(nSector >= 0 && nSector < kMaxSectors);
            BuildSplash(nSprite, nSector);
        }
    }

    pSprite->cstat = cstat; // restore cstat
    pSprite->z = ebp;

    if (pSprite->statnum == 100)
    {
        BuildNear(pSprite->x, pSprite->y, clipdist + (clipdist / 2), pSprite->sectnum);
        nRet |= BelowNear(&exhumedActors[nSprite]);
    }

    return nRet;
}

int GetSpriteHeight(int nSprite)
{
	auto pSprite = &sprite[nSprite];
    return tileHeight(pSprite->picnum) * pSprite->yrepeat * 4;
}

int GetActorHeight(DExhumedActor* actor)
{
    return tileHeight(actor->s().picnum) * actor->s().yrepeat * 4;
}

DExhumedActor* insertActor(int sect, int stat)
{
    int ndx = insertsprite(sect, stat);
    return ndx >= 0 ? &exhumedActors[ndx] : nullptr;
}


int movesprite(short nSprite, int dx, int dy, int dz, int, int flordist, unsigned int clipmask)
{
    spritetype *pSprite = &sprite[nSprite];
    bTouchFloor = false;

    int x = pSprite->x;
    int y = pSprite->y;
    int z = pSprite->z;

    int nSpriteHeight = GetSpriteHeight(nSprite);

    int nClipDist = (int8_t)pSprite->clipdist << 2;

    short nSector = pSprite->sectnum;
    assert(nSector >= 0 && nSector < kMaxSectors);

    int floorZ = sector[nSector].floorz;

    int nRet = 0;

    if ((SectFlag[nSector] & kSectUnderwater) || (floorZ < z))
    {
        dx >>= 1;
        dy >>= 1;
    }

    nRet |= movespritez(nSprite, dz, nSpriteHeight, flordist, nClipDist);

    nSector = pSprite->sectnum; // modified in movespritez so re-grab this variable

    if (pSprite->statnum == 100)
    {
        short nPlayer = GetPlayerFromSprite(nSprite);

        int varA = 0;
        int varB = 0;

        CheckSectorFloor(overridesect, pSprite->z, &varB, &varA);

        if (varB || varA)
        {
            nXDamage[nPlayer] = varB;
            nYDamage[nPlayer] = varA;
        }

        dx += nXDamage[nPlayer];
        dy += nYDamage[nPlayer];
    }
    else
    {
        CheckSectorFloor(overridesect, pSprite->z, &dx, &dy);
    }

    nRet |= (uint16_t)clipmove_old(&pSprite->x, &pSprite->y, &pSprite->z, &nSector, dx, dy, nClipDist, nSpriteHeight, flordist, clipmask);

    if ((nSector != pSprite->sectnum) && nSector >= 0)
    {
        if (nRet & 0x20000) {
            dz = 0;
        }

        if ((sector[nSector].floorz - z) < (dz + flordist))
        {
            pSprite->x = x;
            pSprite->y = y;
        }
        else
        {
            mychangespritesect(nSprite, nSector);

            if (pSprite->pal < 5 && !pSprite->hitag)
            {
                pSprite->pal = sector[pSprite->sectnum].ceilingpal;
            }
        }
    }

    return nRet;
}

void Gravity(short nSprite)
{
    auto pSprite = &sprite[nSprite];
    short nSector = pSprite->sectnum;

    if (SectFlag[nSector] & kSectUnderwater)
    {
        if (pSprite->statnum != 100)
        {
            if (pSprite->zvel <= 1024)
            {
                if (pSprite->zvel < 2048) {
                    pSprite->zvel += 512;
                }
            }
            else
            {
                pSprite->zvel -= 64;
            }
        }
        else
        {
            if (pSprite->zvel > 0)
            {
                pSprite->zvel -= 64;
                if (pSprite->zvel < 0) {
                    pSprite->zvel = 0;
                }
            }
            else if (pSprite->zvel < 0)
            {
                pSprite->zvel += 64;
                if (pSprite->zvel > 0) {
                    pSprite->zvel = 0;
                }
            }
        }
    }
    else
    {
        pSprite->zvel += 512;
        if (pSprite->zvel > 16384) {
            pSprite->zvel = 16384;
        }
    }
}

int MoveCreature(short nSprite)
{
    auto pSprite = &sprite[nSprite];
    return movesprite(nSprite, pSprite->xvel << 8, pSprite->yvel << 8, pSprite->zvel, 15360, -5120, CLIPMASK0);
}

int MoveCreatureWithCaution(int nSprite)
{
    auto pSprite = &sprite[nSprite];
    int x = pSprite->x;
    int y = pSprite->y;
    int z = pSprite->z;
    short nSectorPre = pSprite->sectnum;

    int ecx = MoveCreature(nSprite);

    short nSector = pSprite->sectnum;

    if (nSector != nSectorPre)
    {
        int zDiff = sector[nSectorPre].floorz - sector[nSector].floorz;
        if (zDiff < 0) {
            zDiff = -zDiff;
        }

        if (zDiff > 15360 || (SectFlag[nSector] & kSectUnderwater) || (SectBelow[nSector] > -1 && SectFlag[SectBelow[nSector]]) || SectDamage[nSector])
        {
            pSprite->x = x;
            pSprite->y = y;
            pSprite->z = z;

            mychangespritesect(nSprite, nSectorPre);

            pSprite->ang = (pSprite->ang + 256) & kAngleMask;
            pSprite->xvel = bcos(pSprite->ang, -2);
            pSprite->yvel = bsin(pSprite->ang, -2);
            return 0;
        }
    }

    return ecx;
}

int GetAngleToSprite(int nSprite1, int nSprite2)
{
    if (nSprite1 < 0 || nSprite2 < 0)
        return -1;

	auto pSprite1 = &sprite[nSprite1];
	auto pSprite2 = &sprite[nSprite2];

    return GetMyAngle(pSprite2->x - pSprite1->x, pSprite2->y - pSprite1->y);
}

int PlotCourseToSprite(int nSprite1, int nSprite2)
{
    if (nSprite1 < 0 || nSprite2 < 0)
        return -1;

	auto pSprite1 = &sprite[nSprite1];
	auto pSprite2 = &sprite[nSprite2];
    int x = pSprite2->x - pSprite1->x;
    int y = pSprite2->y - pSprite1->y;

    pSprite1->ang = GetMyAngle(x, y);

    uint32_t x2 = abs(x);
    uint32_t y2 = abs(y);

    uint32_t diff = x2 * x2 + y2 * y2;

    if (diff > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        diff = INT_MAX;
    }

    return ksqrt(diff);
}

int FindPlayer(int nSprite, int nDistance)
{
	auto pSprite = &sprite[nSprite];
    int var_18 = 0;
    if (nSprite >= 0)
        var_18 = 1;

    if (nSprite < 0)
        nSprite = -nSprite;

    if (nDistance < 0)
        nDistance = 100;

    int x = pSprite->x;
    int y = pSprite->y;
    short nSector = pSprite->sectnum;

    int z = pSprite->z - GetSpriteHeight(nSprite);

    nDistance <<= 8;

    short nPlayerSprite;
    int i = 0;

    while (1)
    {
        if (i >= nTotalPlayers)
            return -1;

        nPlayerSprite = PlayerList[i].nSprite;

        if ((sprite[nPlayerSprite].cstat & 0x101) && (!(sprite[nPlayerSprite].cstat & 0x8000)))
        {
            int v9 = abs(sprite[nPlayerSprite].x - x);

            if (v9 < nDistance)
            {
                int v10 = abs(sprite[nPlayerSprite].y - y);

                if (v10 < nDistance && cansee(sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z - 7680, sprite[nPlayerSprite].sectnum, x, y, z, nSector))
                {
                    break;
                }
            }
        }

        i++;
    }

    if (var_18) {
        PlotCourseToSprite(nSprite, nPlayerSprite);
    }

    return nPlayerSprite;
}

void CheckSectorFloor(short nSector, int z, int *x, int *y)
{
    short nSpeed = SectSpeed[nSector];

    if (!nSpeed) {
        return;
    }

    short nFlag = SectFlag[nSector];
    short nAng = nFlag & kAngleMask;

    if (z >= sector[nSector].floorz)
    {
        *x += bcos(nAng, 3) * nSpeed;
        *y += bsin(nAng, 3) * nSpeed;
    }
    else if (nFlag & 0x800)
    {
        *x += bcos(nAng, 4) * nSpeed;
        *y += bsin(nAng, 4) * nSpeed;
    }
}

int GetUpAngle(short nSprite1, int nVal, short nSprite2, int ecx)
{
	auto pSprite1 = &sprite[nSprite1];
	auto pSprite2 = &sprite[nSprite2];
    int x = pSprite2->x - pSprite1->x;
    int y = pSprite2->y - pSprite1->y;

    int ebx = (pSprite2->z + ecx) - (pSprite1->z + nVal);
    int edx = (pSprite2->z + ecx) - (pSprite1->z + nVal);

    ebx >>= 4;
    edx >>= 8;

    ebx = -ebx;

    ebx -= edx;

    int nSqrt = lsqrt(x * x + y * y);

    return GetMyAngle(nSqrt, ebx);
}

void InitPushBlocks()
{
    nPushBlocks = 0;
}

int GrabPushBlock()
{
    if (nPushBlocks >= kMaxPushBlocks) {
        return -1;
    }

    return nPushBlocks++;
}

void CreatePushBlock(int nSector)
{
    int nBlock = GrabPushBlock();
    int i;

    int startwall = sector[nSector].wallptr;
    int nWalls = sector[nSector].wallnum;

    int xSum = 0;
    int ySum = 0;

    for (i = 0; i < nWalls; i++)
    {
        xSum += wall[startwall + i].x;
        ySum += wall[startwall + i].y;
    }

    int xAvg = xSum / nWalls;
    int yAvg = ySum / nWalls;

    sBlockInfo[nBlock].x = xAvg;
    sBlockInfo[nBlock].y = yAvg;

    int nSprite = insertsprite(nSector, 0);
    auto pSprite = &sprite[nSprite];

    sBlockInfo[nBlock].nSprite = nSprite;

    pSprite->x = xAvg;
    pSprite->y = yAvg;
    pSprite->z = sector[nSector].floorz - 256;
    pSprite->cstat = 0x8000;

    int var_28 = 0;

    for (i = 0; i < nWalls; i++)
    {
        uint32_t xDiff = abs(xAvg - wall[startwall + i].x);
        uint32_t yDiff = abs(yAvg - wall[startwall + i].y);

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);
        if (nSqrt > var_28) {
            var_28 = nSqrt;
        }
    }

    sBlockInfo[nBlock].field_8 = var_28;

    pSprite->clipdist = (var_28 & 0xFF) << 2;
    sector[nSector].extra = nBlock;
}

void MoveSector(short nSector, int nAngle, int *nXVel, int *nYVel)
{
    int i;

    if (nSector == -1) {
        return;
    }

    int nXVect, nYVect;

    if (nAngle < 0)
    {
        nXVect = *nXVel;
        nYVect = *nYVel;
        nAngle = GetMyAngle(nXVect, nYVect);
    }
    else
    {
        nXVect = bcos(nAngle, 6);
        nYVect = bsin(nAngle, 6);
    }

    short nBlock = sector[nSector].extra;
    short nSectFlag = SectFlag[nSector];

    sectortype *pSector = &sector[nSector];
    int nFloorZ = sector[nSector].floorz;
    int startwall = sector[nSector].wallptr;
    int nWalls = sector[nSector].wallnum;

    walltype *pStartWall = &wall[startwall];
    short nNextSector = wall[startwall].nextsector;

    BlockInfo *pBlockInfo = &sBlockInfo[nBlock];

    int x = sBlockInfo[nBlock].x;
    int x_b = sBlockInfo[nBlock].x;

    int y = sBlockInfo[nBlock].y;
    int y_b = sBlockInfo[nBlock].y;

    short nSectorB = nSector;

    int nZVal;
    int z;

    int bUnderwater = nSectFlag & kSectUnderwater;

    if (nSectFlag & kSectUnderwater)
    {
        nZVal = sector[nSector].ceilingz;
        z = sector[nNextSector].ceilingz + 256;

        sector[nSector].ceilingz = sector[nNextSector].ceilingz;
    }
    else
    {
        nZVal = sector[nSector].floorz;
        z = sector[nNextSector].floorz - 256;

        sector[nSector].floorz = sector[nNextSector].floorz;
    }

    clipmove_old((int32_t*)&x, (int32_t*)&y, (int32_t*)&z, &nSectorB, nXVect, nYVect, pBlockInfo->field_8, 0, 0, CLIPMASK1);

    int yvect = y - y_b;
    int xvect = x - x_b;

    if (nSectorB != nNextSector && nSectorB != nSector)
    {
        yvect = 0;
        xvect = 0;
    }
    else
    {
        if (!bUnderwater)
        {
            z = nZVal;
            x = x_b;
            y = y_b;

            clipmove_old((int32_t*)&x, (int32_t*)&y, (int32_t*)&z, &nSectorB, nXVect, nYVect, pBlockInfo->field_8, 0, 0, CLIPMASK1);

            int ebx = x;
            int ecx = x_b;
            int edx = y;
            int eax = xvect;
            int esi = y_b;

            if (eax < 0) {
                eax = -eax;
            }

            ebx -= ecx;
            ecx = eax;
            eax = ebx;
            edx -= esi;

            if (eax < 0) {
                eax = -eax;
            }

            if (ecx > eax)
            {
                xvect = ebx;
            }

            eax = yvect;
            if (eax < 0) {
                eax = -eax;
            }

            ebx = eax;
            eax = edx;

            if (eax < 0) {
                eax = -eax;
            }

            if (ebx > eax) {
                yvect = edx;
            }
        }
    }

    // GREEN
    if (yvect || xvect)
    {
        SectIterator it(nSector);
        while ((i = it.NextIndex()) >= 0)
        {
            if (sprite[i].statnum < 99)
            {
                sprite[i].x += xvect;
                sprite[i].y += yvect;
            }
            else
            {
                z = sprite[i].z;

                if ((nSectFlag & kSectUnderwater) || z != nZVal || sprite[i].cstat & 0x8000)
                {
                    x = sprite[i].x;
                    y = sprite[i].y;
                    nSectorB = nSector;

                    clipmove_old((int32_t*)&x, (int32_t*)&y, (int32_t*)&z, &nSectorB, -xvect, -yvect, 4 * sprite[i].clipdist, 0, 0, CLIPMASK0);

                    if (nSectorB >= 0 && nSectorB < kMaxSectors && nSectorB != nSector) {
                        mychangespritesect(i, nSectorB);
                    }
                }
            }
        }

        it.Reset(nNextSector);
        while ((i = it.NextIndex()) >= 0)
        {
            auto pSprite = &sprite[i];
            if (pSprite->statnum >= 99)
            {
                x = pSprite->x;
                y = pSprite->y;
                z = pSprite->z;
                nSectorB = nNextSector;

                clipmove_old((int32_t*)&x, (int32_t*)&y, (int32_t*)&z, &nSectorB,
                    -xvect - (bcos(nAngle) * (4 * pSprite->clipdist)),
                    -yvect - (bsin(nAngle) * (4 * pSprite->clipdist)),
                    4 * pSprite->clipdist, 0, 0, CLIPMASK0);


                if (nSectorB != nNextSector && (nSectorB == nSector || nNextSector == nSector))
                {
                    if (nSectorB != nSector || nFloorZ >= pSprite->z)
                    {
                        if (nSectorB >= 0 && nSectorB < kMaxSectors) {
                            mychangespritesect(i, nSectorB);
                        }
                    }
                    else
                    {
                        movesprite(i,
                            (xvect << 14) + bcos(nAngle) * pSprite->clipdist,
                            (yvect << 14) + bsin(nAngle) * pSprite->clipdist,
                            0, 0, 0, CLIPMASK0);
                    }
                }
            }
        }

        for (int i = 0; i < nWalls; i++)
        {
            dragpoint(startwall, xvect + pStartWall->x, yvect + pStartWall->y, 0);
            pStartWall++;
            startwall++;
        }

        pBlockInfo->x += xvect;
        pBlockInfo->y += yvect;
    }

    // loc_163DD
    xvect <<= 14;
    yvect <<= 14;

    if (!(nSectFlag & kSectUnderwater))
    {
        SectIterator it(nSector);
        while ((i = it.NextIndex()) >= 0)
        {
            auto pSprite = &sprite[i];
            if (pSprite->statnum >= 99 && nZVal == pSprite->z && !(pSprite->cstat & 0x8000))
            {
                nSectorB = nSector;
                clipmove_old(&pSprite->x, &pSprite->y, &pSprite->z, &nSectorB, xvect, yvect, 4 * pSprite->clipdist, 5120, -5120, CLIPMASK0);
            }
        }
    }

    if (nSectFlag & kSectUnderwater) {
        pSector->ceilingz = nZVal;
    }
    else {
        pSector->floorz = nZVal;
    }

    *nXVel = xvect;
    *nYVel = yvect;

    /* 
        Update player position variables, in case the player sprite was moved by a sector,
        Otherwise these can be out of sync when used in sound code (before being updated in PlayerFunc()). 
        Can cause local player sounds to play off-centre.
        TODO: Might need to be done elsewhere too?
    */
    auto pActor = PlayerList[nLocalPlayer].Actor();
    auto pSprite = &pActor->s();
    initx = pSprite->x;
    inity = pSprite->y;
    initz = pSprite->z;
    inita = pSprite->ang;
    initsect = pSprite->sectnum;
}

void SetQuake(short nSprite, int nVal)
{
    auto pSprite = &sprite[nSprite];
    int x = pSprite->x;
    int y = pSprite->y;

    nVal *= 256;

    for (int i = 0; i < nTotalPlayers; i++)
    {
        int nPlayerSprite = PlayerList[i].nSprite;

        uint32_t xDiff = abs((int32_t)((sprite[nPlayerSprite].x - x) >> 8));
        uint32_t yDiff = abs((int32_t)((sprite[nPlayerSprite].y - y) >> 8));

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);

        int eax = nVal;

        if (nSqrt)
        {
            eax = eax / nSqrt;

            if (eax >= 256)
            {
                if (eax > 3840) {
                    eax = 3840;
                }
            }
            else
            {
                eax = 0;
            }
        }

        if (eax > nQuake[i]) {
            nQuake[i] = eax;
        }
    }
}

int AngleChase(int nSprite, int nSprite2, int ebx, int ecx, int push1)
{
    auto pSprite = &sprite[nSprite];
    int nClipType = pSprite->statnum != 107;

    /* bjd - need to handle cliptype to clipmask change that occured in later build engine version */
    if (nClipType == 1) {
        nClipType = CLIPMASK1;
    }
    else {
        nClipType = CLIPMASK0;
    }

    short nAngle;

    if (nSprite2 < 0)
    {
        pSprite->zvel = 0;
        nAngle = pSprite->ang;
    }
    else
    {
		auto pSprite2 = &sprite[nSprite2];

        int nHeight = tileHeight(pSprite2->picnum) * pSprite2->yrepeat * 2;

        int nMyAngle = GetMyAngle(pSprite2->x - pSprite->x, pSprite2->y - pSprite->y);

        uint32_t xDiff = abs(pSprite2->x - pSprite->x);
        uint32_t yDiff = abs(pSprite2->y - pSprite->y);

        uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

        if (sqrtNum > INT_MAX)
        {
            DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
            sqrtNum = INT_MAX;
        }

        int nSqrt = ksqrt(sqrtNum);

        int var_18 = GetMyAngle(nSqrt, ((pSprite2->z - nHeight) - pSprite->z) >> 8);

        int nAngDelta = AngleDelta(pSprite->ang, nMyAngle, 1024);
        int nAngDelta2 = abs(nAngDelta);

        if (nAngDelta2 > 63)
        {
            nAngDelta2 = abs(nAngDelta >> 6);

            ebx /= nAngDelta2;

            if (ebx < 5) {
                ebx = 5;
            }
        }

        int nAngDeltaC = abs(nAngDelta);

        if (nAngDeltaC > push1)
        {
            if (nAngDelta >= 0)
                nAngDelta = push1;
            else
                nAngDelta = -push1;
        }

        nAngle = (nAngDelta + pSprite->ang) & kAngleMask;
        int nAngDeltaD = AngleDelta(pSprite->zvel, var_18, 24);

        pSprite->zvel = (pSprite->zvel + nAngDeltaD) & kAngleMask;
    }

    pSprite->ang = nAngle;

    int eax = abs(bcos(pSprite->zvel));

    int x = ((bcos(nAngle) * ebx) >> 14) * eax;
    int y = ((bsin(nAngle) * ebx) >> 14) * eax;

    int xshift = x >> 8;
    int yshift = y >> 8;

    uint32_t sqrtNum = xshift * xshift + yshift * yshift;

    if (sqrtNum > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        sqrtNum = INT_MAX;
    }

    int z = bsin(pSprite->zvel) * ksqrt(sqrtNum);

    return movesprite(nSprite, x >> 2, y >> 2, (z >> 13) + bsin(ecx, -5), 0, 0, nClipType);
}

int GetWallNormal(short nWall)
{
    nWall &= kMaxWalls-1;

    int nWall2 = wall[nWall].point2;

    int nAngle = GetMyAngle(wall[nWall2].x - wall[nWall].x, wall[nWall2].y - wall[nWall].y);
    return (nAngle + 512) & kAngleMask;
}

void WheresMyMouth(int nPlayer, int *x, int *y, int *z, short *sectnum)
{
    auto pActor = PlayerList[nPlayer].Actor();
	auto pSprite = &pActor->s();

    *x = pSprite->x;
    *y = pSprite->y;

    int height = GetActorHeight(pActor) / 2;

    *z = pSprite->z - height;
    *sectnum = pSprite->sectnum;

    clipmove_old((int32_t*)x, (int32_t*)y, (int32_t*)z, sectnum,
        bcos(pSprite->ang, 7),
        bsin(pSprite->ang, 7),
        5120, 1280, 1280, CLIPMASK1);
}

void InitChunks()
{
    nCurChunkNum = 0;
    memset(nChunkSprite,   0, sizeof(nChunkSprite));
    memset(nBodyGunSprite, 0, sizeof(nBodyGunSprite));
    memset(nBodySprite,    0, sizeof(nBodySprite));
    nCurBodyNum    = 0;
    nCurBodyGunNum = 0;
    nBodyTotal  = 0;
    nChunkTotal = 0;
}

DExhumedActor* GrabBodyGunSprite()
{
    auto pActor = nBodyGunSprite[nCurBodyGunNum];
	spritetype* pSprite;
    if (pActor == nullptr)
    {
        pActor = insertActor(0, 899);
		pSprite = &pActor->s();
        nBodyGunSprite[nCurBodyGunNum] = pActor;

        pSprite->lotag = -1;
        pSprite->owner = -1;
    }
    else
    {
		auto pSprite = &pActor->s();
        int nAnim = pSprite->owner;

        if (nAnim != -1) {
            DestroyAnim(nAnim);
        }

        pSprite->lotag = -1;
        pSprite->owner = -1;
    }

    nCurBodyGunNum++;
    if (nCurBodyGunNum >= 50) { // TODO - enum/define
        nCurBodyGunNum = 0;
    }

    pSprite->cstat = 0;

    return pActor;
}

DExhumedActor* GrabBody()
{
	DExhumedActor* pActor = nullptr;
    spritetype* pSprite = nullptr;
    do
    {
        pActor = nBodySprite[nCurBodyNum];

        if (pActor == nullptr)
        {
            pActor = insertActor(0, 899);
            pSprite = &pActor->s();
            nBodySprite[nCurBodyNum] = pActor;
            pSprite->cstat = 0x8000;
        }
		else
			pSprite = &pActor->s();


        nCurBodyNum++;
        if (nCurBodyNum >= 50) {
            nCurBodyNum = 0;
        }
    } while (pSprite->cstat & 0x101);

    if (nBodyTotal < 50) {
        nBodyTotal++;
    }

    pSprite->cstat = 0;
    return pActor;
}

DExhumedActor* GrabChunkSprite()
{
    auto pActor = nChunkSprite[nCurChunkNum];

    if (pActor == nullptr)
    {
        pActor = insertActor(0, 899);
		nChunkSprite[nCurChunkNum] = pActor;
    }
    else if (pActor->s().statnum)
    {
// TODO	MonoOut("too many chunks being used at once!\n");
        return nullptr;
    }

    ChangeActorStat(pActor, 899);

    nCurChunkNum++;
    if (nCurChunkNum >= kMaxMoveChunks)
        nCurChunkNum = 0;

    if (nChunkTotal < kMaxMoveChunks)
        nChunkTotal++;

    pActor->s().cstat = 0x80;

    return pActor;
}

int BuildCreatureChunk(int nVal, int nPic)
{
    int var_14;

    auto actor = GrabChunkSprite();

    if (actor == nullptr) {
        return -1;
    }
	auto pSprite = &actor->s();

    if (nVal & 0x4000)
    {
        nVal &= 0x3FFF;
        var_14 = 1;
    }
    else
    {
        var_14 = 0;
    }

    nVal &= 0xFFFF;

    pSprite->x = sprite[nVal].x;
    pSprite->y = sprite[nVal].y;
    pSprite->z = sprite[nVal].z;

    mychangespritesect(actor->GetSpriteIndex(), sprite[nVal].sectnum);

    pSprite->cstat = 0x80;
    pSprite->shade = -12;
    pSprite->pal = 0;

    pSprite->xvel = (RandomSize(5) - 16) << 7;
    pSprite->yvel = (RandomSize(5) - 16) << 7;
    pSprite->zvel = (-(RandomSize(8) + 512)) << 3;

    if (var_14)
    {
        pSprite->xvel *= 4;
        pSprite->yvel *= 4;
        pSprite->zvel *= 2;
    }

    pSprite->xrepeat = 64;
    pSprite->yrepeat = 64;
    pSprite->xoffset = 0;
    pSprite->yoffset = 0;
    pSprite->picnum = nPic;
    pSprite->lotag = runlist_HeadRun() + 1;
    pSprite->clipdist = 40;

//	GrabTimeSlot(3);

    pSprite->extra = -1;
    pSprite->owner = runlist_AddRunRec(pSprite->lotag - 1, actor->GetSpriteIndex(), 0xD0000);
    pSprite->hitag = runlist_AddRunRec(NewRun, actor->GetSpriteIndex(), 0xD0000);

    return actor->GetSpriteIndex();
}

void AICreatureChunk::Tick(RunListEvent* ev)
{
    int nSprite = RunData[ev->nRun].nObjIndex;
    assert(nSprite >= 0 && nSprite < kMaxSprites);
    auto pSprite = &sprite[nSprite];

    Gravity(nSprite);

    int nSector = pSprite->sectnum;
    pSprite->pal = sector[nSector].ceilingpal;

    int nVal = movesprite(nSprite, pSprite->xvel << 10, pSprite->yvel << 10, pSprite->zvel, 2560, -2560, CLIPMASK1);

    if (pSprite->z >= sector[nSector].floorz)
    {
        // re-grab this variable as it may have changed in movesprite(). Note the check above is against the value *before* movesprite so don't change it.
        nSector = pSprite->sectnum;

        pSprite->xvel = 0;
        pSprite->yvel = 0;
        pSprite->zvel = 0;
        pSprite->z = sector[nSector].floorz;
    }
    else
    {
        if (!nVal)
            return;

        short nAngle;

        if (nVal & 0x20000)
        {
            pSprite->cstat = 0x8000;
        }
        else
        {
            if ((nVal & 0x3C000) == 0x10000)
            {
                pSprite->xvel >>= 1;
                pSprite->yvel >>= 1;
                pSprite->zvel = -pSprite->zvel;
                return;
            }
            else if ((nVal & 0x3C000) == 0xC000)
            {
                nAngle = sprite[nVal & 0x3FFF].ang;
            }
            else if ((nVal & 0x3C000) == 0x8000)
            {
                nAngle = GetWallNormal(nVal & 0x3FFF);
            }
            else
            {
                return;
            }

            // loc_16E0C
            int nSqrt = lsqrt(((pSprite->yvel >> 10) * (pSprite->yvel >> 10)
                + (pSprite->xvel >> 10) * (pSprite->xvel >> 10)) >> 8);

            pSprite->xvel = bcos(nAngle) * (nSqrt >> 1);
            pSprite->yvel = bsin(nAngle) * (nSqrt >> 1);
            return;
        }
    }

    runlist_DoSubRunRec(pSprite->owner);
    runlist_FreeRun(pSprite->lotag - 1);
    runlist_SubRunRec(pSprite->hitag);

    changespritestat(nSprite, 0);
    pSprite->hitag = 0;
    pSprite->lotag = 0;
}

void  FuncCreatureChunk(int nObject, int nMessage, int nDamage, int nRun)
{
    AICreatureChunk ai;
    runlist_DispatchEvent(&ai, nObject, nMessage, nDamage, nRun);

}

short UpdateEnemy(short *nEnemy)
{
    if (*nEnemy >= 0)
    {
        if (!(sprite[*nEnemy].cstat & 0x101)) {
            *nEnemy = -1;
        }
    }

    return *nEnemy;
}
END_PS_NS
