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
#include "player.h"
#include "exhumed.h"
#include "sound.h"
#include "status.h"
#include "engine.h"
#include "input.h"
#include "mapinfo.h"

BEGIN_PS_NS

struct AnimInfo
{
    short a;
    short repeat;
};

AnimInfo nItemAnimInfo[] = {
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { 6, 64 },
    { -1, 48 },
    { 0, 64 },
    { 1, 64 },
    { -1, 32 },
    { 4, 64 },
    { 5, 64 },
    { 16, 64 },
    { 10, 64 },
    { -1, 32 },
    { 8, 64 },
    { 9, 64 },
    { -1, 40 },
    { -1, 32 },
    { 7, 64 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { 14, 64 },
    { 15, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { 17, 48 },
    { 18, 48 },
    { 19, 48 },
    { 20, 48 },
    { 24, 64 },
    { 21, 64 },
    { 23, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { 11, 30 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 },
    { -1, 32 }
};

short nItemMagic[] = { 500, 1000, 100, 500, 400, 200, 700, 0 };

/*

short something
short x/y repeat

*/

short nRegenerates;
short nFirstRegenerate;
short nMagicCount;

void SerializeItems(FSerializer& arc)
{
    if (arc.BeginObject("items"))
    {
        arc("regenerates", nRegenerates)
            ("first", nFirstRegenerate)
            ("magiccount", nMagicCount)
            .EndObject();
    }
}

void BuildItemAnim(short nSprite)
{
	auto pSprite = &sprite[nSprite];

    int nItem = pSprite->statnum - 906;

    if (nItemAnimInfo[nItem].a >= 0)
    {
        auto pAnimActor = BuildAnim(&exhumedActors[nSprite], 41, nItemAnimInfo[nItem].a, pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum, nItemAnimInfo[nItem].repeat, 20);

        if (nItem == 44) {
            pAnimActor->s().cstat |= 2;
        }

        ChangeActorStat(pAnimActor, pSprite->statnum);
        pAnimActor->s().hitag = pSprite->hitag;
        pSprite->owner = 0;
    }
    else
    {
        pSprite->owner = -1;
        pSprite->yrepeat = (uint8_t)nItemAnimInfo[nItem].repeat;
        pSprite->xrepeat = (uint8_t)nItemAnimInfo[nItem].repeat;
    }
}

void DestroyItemAnim(short nSprite)
{
    if (sprite[nSprite].owner == 0) 
        DestroyAnim(&exhumedActors[nSprite]);
}

void ItemFlash()
{
    TintPalette(16, 16, 16);
}

void FillItems(short nPlayer)
{
    for (int i = 0; i < 6; i++)
    {
        PlayerList[nPlayer].items[i] = 5;
    }

    PlayerList[nPlayer].nMagic = 1000;

    if (nPlayer == nLocalPlayer)
    {
        ItemFlash();
    }

    if (PlayerList[nPlayer].nItem == -1) {
        PlayerList[nPlayer].nItem = 0;
    }
}

static bool UseEye(short nPlayer)
{
    if (PlayerList[nPlayer].nInvisible >= 0) 
        PlayerList[nPlayer].nInvisible = 900;

    int nSprite = PlayerList[nPlayer].nSprite;
	auto pSprite = &sprite[nSprite];

    pSprite->cstat |= 0x8000;

    if (nPlayerFloorSprite[nPlayer] >= 0) {
        pSprite->cstat |= 0x8000;
    }

    if (nPlayer == nLocalPlayer)
    {
        ItemFlash();
        D3PlayFX(StaticSound[kSound31], nSprite);
    }
    return true;
}

static bool UseMask(short nPlayer)
{
    PlayerList[nPlayer].nMaskAmount = 1350;
    PlayerList[nPlayer].nAir = 100;

    if (nPlayer == nLocalPlayer)
    {
        D3PlayFX(StaticSound[kSound31], PlayerList[nPlayer].nSprite);
    }
    return true;
}

bool UseTorch(short nPlayer)
{
    if (!PlayerList[nPlayer].nTorch) 
    {
        SetTorch(nPlayer, 1);
    }

    PlayerList[nPlayer].nTorch = 900;
    return true;
}

bool UseHeart(short nPlayer)
{
    if (PlayerList[nPlayer].nHealth < kMaxHealth) {
        PlayerList[nPlayer].nHealth = kMaxHealth;

        if (nPlayer == nLocalPlayer)
        {
            ItemFlash();
            D3PlayFX(StaticSound[kSound31], PlayerList[nPlayer].nSprite);
        }
        return true;
    }
    return false;
}

// invincibility
bool UseScarab(short nPlayer)
{
    if (PlayerList[nPlayer].invincibility >= 0 && PlayerList[nPlayer].invincibility < 900)
        PlayerList[nPlayer].invincibility = 900;

    if (nPlayer == nLocalPlayer)
    {
        ItemFlash();
        D3PlayFX(StaticSound[kSound31], PlayerList[nPlayer].nSprite);
    }
    return true;
}

// faster firing
static bool UseHand(short nPlayer)
{
    PlayerList[nPlayer].nDouble = 1350;

    if (nPlayer == nLocalPlayer)
    {
        ItemFlash();
        D3PlayFX(StaticSound[kSound31], PlayerList[nPlayer].nSprite);
    }
    return true;
}

void UseItem(short nPlayer, short nItem)
{
    bool didit = false;
    switch (nItem)
    {
        case 0:
            didit = UseHeart(nPlayer);
            break;
        case 1:
            didit = UseScarab(nPlayer);
            break;
        case 2:
            didit = UseTorch(nPlayer);
            break;
        case 3:
            didit = UseHand(nPlayer);
            break;
        case 4:
            didit = UseEye(nPlayer);
            break;
        case 5:
            didit = UseMask(nPlayer);
            break;
        default:
            break;
    }
    if (!didit) return;

    PlayerList[nPlayer].items[nItem]--;
    int nItemCount = PlayerList[nPlayer].items[nItem];

    int nMagic = nItemMagic[nItem];

    if (!nItemCount)
    {
        for (nItem = 0; nItem < 6; nItem++)
        {
            if (PlayerList[nPlayer].items[nItem] > 0) {
                break;
            }
        }

        if (nItem == 6) {
            nItem = -1;
        }
    }

    PlayerList[nPlayer].nMagic -= nMagic;
    PlayerList[nPlayer].nItem = nItem;
}

// TODO - bool return type?
int GrabItem(short nPlayer, short nItem)
{
    if (PlayerList[nPlayer].items[nItem] >= 5) {
        return 0;
    }

    PlayerList[nPlayer].items[nItem]++;

    if (PlayerList[nPlayer].nItem < 0 || nItem == PlayerList[nPlayer].nItem) {
        PlayerList[nPlayer].nItem = nItem;
    }

    return 1;
}

void DropMagic(short nSprite)
{
	auto pSprite = &sprite[nSprite];

    if (lFinaleStart) {
        return;
    }

    nMagicCount--;

    if (nMagicCount <= 0)
    {
        auto pAnimActor = BuildAnim(
            nullptr,
            64,
            0,
            pSprite->x,
            pSprite->y,
            pSprite->z,
            pSprite->sectnum,
            48,
            4);

        if (pAnimActor)
        {
            AddFlash(pAnimActor->s().sectnum, pAnimActor->s().x, pAnimActor->s().y, pAnimActor->s().z, 128);
            ChangeActorStat(pAnimActor, 950);
        }
        nMagicCount = RandomSize(2);
    }
}

void InitItems()
{
    nRegenerates = 0;
    nFirstRegenerate = -1;
    nMagicCount = 0;
}

void StartRegenerate(short nSprite)
{
    spritetype *pSprite = &sprite[nSprite];

    int edi = -1;

    int nReg = nFirstRegenerate;

    int i = 0;

//	for (int i = 0; i < nRegenerates; i++)
    while (1)
    {
        if (i >= nRegenerates)
        {
            // ?? CHECKME
            pSprite->xvel = pSprite->xrepeat;
            pSprite->zvel = pSprite->shade;
            pSprite->yvel = pSprite->pal;
            break;
        }
        else
        {
            if (nReg != nSprite)
            {
                edi = nReg;
                nReg = sprite[nReg].ang;
                i++;
                continue;
            }
            else
            {
                if (edi == -1)
                {
                    nFirstRegenerate = pSprite->ang;
                }
                else
                {
                    sprite[edi].ang = pSprite->ang;
                }

                nRegenerates--;
            }
        }
    }

    pSprite->extra = 1350;
    pSprite->ang = nFirstRegenerate;

    if (!(currentLevel->gameflags & LEVEL_EX_MULTI))
    {
        pSprite->ang /= 5;
    }

    pSprite->cstat = 0x8000;
    pSprite->xrepeat = 1;
    pSprite->yrepeat = 1;
    pSprite->pal = 1;

    nRegenerates++;
    nFirstRegenerate = nSprite;
}

void DoRegenerates()
{
    int nSprite = nFirstRegenerate;
	auto pSprite = &sprite[nSprite];

    for (int i = nRegenerates; i > 0; i--, nSprite = pSprite->ang)
    {
        if (pSprite->extra > 0)
        {
            pSprite->extra--;

            if (pSprite->extra <= 0)
            {
                BuildAnim(nullptr, 38, 0, pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum, 64, 4);
                D3PlayFX(StaticSound[kSoundTorchOn], nSprite);
            }
            else {
                continue;
            }
        }
        else
        {
            if (pSprite->xrepeat < pSprite->xvel)
            {
                pSprite->xrepeat += 2;
                pSprite->yrepeat += 2;
                continue;
            }
        }

        pSprite->zvel = 0;
        pSprite->yrepeat = (uint8_t)pSprite->xvel;
        pSprite->xrepeat = (uint8_t)pSprite->xvel;
        pSprite->pal  = (uint8_t)pSprite->yvel;
        pSprite->yvel = pSprite->zvel; // setting to 0
        pSprite->xvel = pSprite->zvel; // setting to 0
        nRegenerates--;

        if (pSprite->statnum == kStatExplodeTrigger) {
            pSprite->cstat = 0x101;
        }
        else {
            pSprite->cstat = 0;
        }

        if (nRegenerates == 0) {
            nFirstRegenerate = -1;
        }
    }
}
END_PS_NS
