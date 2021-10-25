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
#include "engine.h"
#include "player.h"
#include "exhumed.h"
#include "view.h"
#include "status.h"
#include "sound.h"
#include "input.h"
#include <string.h>
#include <assert.h>
#include "v_2ddrawer.h"
#include "sequence.h"

BEGIN_PS_NS

/*
struct Weapon
{
    short nSeq;
    short b[12]; // seq offsets?
    short nAmmoType;
    short c;
    short d;
    short bFireUnderwater;
};
*/

Weapon WeaponInfo[] = {
    { kSeqSword,   { 0, 1, 3,  7, -1,  2,  4, 5, 6, 8, 9, 10 }, 0, 0, 0, true },
    { kSeqPistol,  { 0, 3, 2,  4, -1,  1,  0, 0, 0, 0, 0, 0 },  1, 0, 1, false },
    { kSeqM60,     { 0, 5, 6, 16, -1, 21,  0, 0, 0, 0, 0, 0 },  2, 0, 1, false },
    { kSeqFlamer,  { 0, 2, 5,  5,  6,  1,  0, 0, 0, 0, 0, 0 },  3, 4, 1, false },
    { kSeqGrenade, { 0, 2, 3,  4, -1,  1,  0, 0, 0, 0, 0, 0 },  4, 0, 1, true },
    { kSeqCobra,   { 0, 1, 2,  2, -1,  4,  0, 0, 0, 0, 0, 0 },  5, 0, 1, true },
    { kSeqRavolt,  { 0, 1, 2,  3, -1,  4,  0, 0, 0, 0, 0, 0 },  6, 0, 1, true },
    { kSeqRothands,{ 0, 1, 2, -1, -1, -1,  0, 0, 0, 0, 0, 0 },  7, 0, 0, true },
    { kSeqDead,    { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, false },
    { kSeqDeadEx,  { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, false },
    { kSeqDeadBrn, { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, false }
};

short nTemperature[kMaxPlayers];
short nMinAmmo[] = { 0, 24, 51, 50, 1, 0, 0 };
short isRed = 0;


void SerializeGun(FSerializer& arc)
{
    if (arc.BeginObject("gun"))
    {
        arc.Array("temperature", nTemperature, kMaxPlayers)
            ("isred", isRed)
            .EndObject();
            
    }
}

void RestoreMinAmmo(short nPlayer)
{
    for (int i = 0; i < kMaxWeapons; i++)
    {
        if (i == kWeaponGrenade) {
            continue;
        }

        if ((1 << i) & nPlayerWeapons[nPlayer])
        {
            if (nMinAmmo[i] > PlayerList[nPlayer].nAmmo[i]) {
                PlayerList[nPlayer].nAmmo[i] = nMinAmmo[i];
            }
        }
    }

    CheckClip(nPlayer);
}

void FillWeapons(short nPlayer)
{
    nPlayerWeapons[nPlayer] = 0xFFFF; // turn on all bits

    for (int i = 0; i < kMaxWeapons; i++)
    {
        if (WeaponInfo[i].d) {
            PlayerList[nPlayer].nAmmo[i] = 300;
        }
    }

    CheckClip(nPlayer);
}

void ResetPlayerWeapons(short nPlayer)
{
    for (int i = 0; i < kMaxWeapons; i++)
    {
        PlayerList[nPlayer].nAmmo[i] = 0;
    }

    PlayerList[nPlayer].nCurrentWeapon = 0;
    PlayerList[nPlayer].field_3A = 0;
    PlayerList[nPlayer].field_3FOUR = 0;

    nPlayerGrenade[nPlayer] = -1;
    nPlayerWeapons[nPlayer] = 0x1; // turn on bit 1 only
}

void InitWeapons()
{
    memset(nPlayerGrenade, 0, sizeof(nPlayerGrenade));
    memset(nGrenadePlayer, 0, sizeof(nGrenadePlayer));
}

void SetNewWeapon(short nPlayer, short nWeapon)
{
    if (nWeapon == kWeaponMummified)
    {
        PlayerList[nPlayer].field_3C = PlayerList[nPlayer].nCurrentWeapon;
        PlayerList[nPlayer].bIsFiring = false;
        PlayerList[nPlayer].field_3A = 5;
        SetPlayerMummified(nPlayer, true);

        PlayerList[nPlayer].field_3FOUR = 0;
    }
    else
    {
        if (nWeapon < 0)
        {
            nPlayerOldWeapon[nPlayer] = PlayerList[nPlayer].nCurrentWeapon;
        }
        else if (nWeapon != kWeaponGrenade || PlayerList[nPlayer].nAmmo[kWeaponGrenade] > 0)
        {
            short nCurrentWeapon = PlayerList[nPlayer].nCurrentWeapon;

            if (nCurrentWeapon != kWeaponMummified)
            {
                if (PlayerList[nPlayer].bIsFiring || nWeapon == nCurrentWeapon) {
                    return;
                }
            }
            else
            {
                PlayerList[nPlayer].nCurrentWeapon = nWeapon;
                PlayerList[nPlayer].field_3FOUR = 0;
            }
        }
        else {
            return;
        }
    }

    PlayerList[nPlayer].field_38 = nWeapon;
}

void SetNewWeaponImmediate(short nPlayer, short nWeapon)
{
    SetNewWeapon(nPlayer, nWeapon);

    PlayerList[nPlayer].nCurrentWeapon = nWeapon;
    PlayerList[nPlayer].field_38 = -1;
    PlayerList[nPlayer].field_3FOUR = 0;
    PlayerList[nPlayer].field_3A = 0;
}

void SetNewWeaponIfBetter(short nPlayer, short nWeapon)
{
    if (nWeapon > PlayerList[nPlayer].nCurrentWeapon) {
        SetNewWeapon(nPlayer, nWeapon);
    }
}

void SelectNewWeapon(short nPlayer)
{
    int nWeapon = kWeaponRing; // start at the highest weapon number

    uint16_t di = nPlayerWeapons[nPlayer];
    uint16_t dx = 0x40; // bit 7 turned on

    while (dx)
    {
        if (di & dx)
        {
            // we have this weapon
            if (!WeaponInfo[nWeapon].d || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                break;
        }

        nWeapon--;
        dx >>= 1;
    }

    if (nWeapon < 0)
        nWeapon = kWeaponSword;

    PlayerList[nPlayer].bIsFiring = false;

    SetNewWeapon(nPlayer, nWeapon);
}

void StopFiringWeapon(short nPlayer)
{
    PlayerList[nPlayer].bIsFiring = false;
}

void FireWeapon(short nPlayer)
{
    if (!PlayerList[nPlayer].bIsFiring) {
        PlayerList[nPlayer].bIsFiring = true;
    }
}

void SetWeaponStatus(short nPlayer)
{
}

uint8_t WeaponCanFire(short nPlayer)
{
    short nWeapon = PlayerList[nPlayer].nCurrentWeapon;
    short nSector = nPlayerViewSect[nPlayer];

    if (!(SectFlag[nSector] & kSectUnderwater) || WeaponInfo[nWeapon].bFireUnderwater)
    {
        short nAmmoType = WeaponInfo[nWeapon].nAmmoType;

        if (WeaponInfo[nWeapon].d <= PlayerList[nPlayer].nAmmo[nAmmoType]) {
            return true;
        }
    }

    return false;
}

// UNUSED
void ResetSwordSeqs()
{
    WeaponInfo[kWeaponSword].b[2] = 3;
    WeaponInfo[kWeaponSword].b[3] = 7;
}

int CheckCloseRange(short nPlayer, int *x, int *y, int *z, short *nSector)
{
    short hitSect, hitWall, hitSprite;
    int hitX, hitY, hitZ;

    short nSprite = PlayerList[nPlayer].nSprite;

    int xVect = bcos(sprite[nSprite].ang);
    int yVect = bsin(sprite[nSprite].ang);

    vec3_t startPos = { *x, *y, *z };
    hitdata_t hitData;
    hitscan(&startPos, *nSector, xVect, yVect, 0, &hitData, CLIPMASK1);
    hitX = hitData.pos.x;
    hitY = hitData.pos.y;
    hitZ = hitData.pos.z;
    hitSprite = hitData.sprite;
    hitSect = hitData.sect;
    hitWall = hitData.wall;

    int ecx = bsin(150, -3);

    uint32_t xDiff = abs(hitX - *x);
    uint32_t yDiff = abs(hitY - *y);

    uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

    if (sqrtNum > INT_MAX)
    {
        DPrintf(DMSG_WARNING, "%s %d: overflow\n", __func__, __LINE__);
        sqrtNum = INT_MAX;
    }

    if (ksqrt(sqrtNum) >= ecx)
        return 0;

    *x = hitX;
    *y = hitY;
    *z = hitZ;
    *nSector = hitSect;

    if (hitSprite > -1) {
        return hitSprite | 0xC000;
    }
    if (hitWall > -1) {
        return hitWall | 0x8000;
    }

    return 0;
}

void CheckClip(short nPlayer)
{
    if (nPlayerClip[nPlayer] <= 0)
    {
        nPlayerClip[nPlayer] = PlayerList[nPlayer].nAmmo[kWeaponM60];

        if (nPlayerClip[nPlayer] > 100) {
            nPlayerClip[nPlayer] = 100;
        }
    }

    // Reset pistol's clip amount.
    nPistolClip[nPlayer] = PlayerList[nPlayer].nAmmo[kWeaponPistol] % 6;
}

void MoveWeapons(short nPlayer)
{
    static int dword_96E22 = 0;

    short nSectFlag = SectFlag[nPlayerViewSect[nPlayer]];

    if ((nSectFlag & kSectUnderwater) && (totalmoves & 1)) {
        return;
    }

    nPilotLightFrame++;

    if (nPilotLightFrame >= nPilotLightCount)
        nPilotLightFrame = 0;

    if (!PlayerList[nPlayer].bIsFiring || (nSectFlag & kSectUnderwater))
        nTemperature[nPlayer] = 0;

    short nPlayerSprite = PlayerList[nPlayer].nSprite;
	auto pPlayerSprite = &sprite[nPlayerSprite];
    short nWeapon = PlayerList[nPlayer].nCurrentWeapon;

    if (nWeapon < -1)
    {
        if (PlayerList[nPlayer].field_38 != -1)
        {
            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].field_38;
            PlayerList[nPlayer].field_3A = 0;
            PlayerList[nPlayer].field_3FOUR = 0;
            PlayerList[nPlayer].field_38 = -1;
        }

        return;
    }

    // loc_26ACC
    short eax = PlayerList[nPlayer].field_3A;
    short nSeq = WeaponInfo[nWeapon].nSeq;

    short var_3C = WeaponInfo[nWeapon].b[eax] + SeqOffsets[nSeq];

    int var_1C = (PlayerList[nPlayer].nDouble > 0) + 1;

    frames = var_1C - 1;

    for (frames = var_1C; frames > 0; frames--)
    {
        seq_MoveSequence(nPlayerSprite, var_3C, PlayerList[nPlayer].field_3FOUR);

        PlayerList[nPlayer].field_3FOUR++;

        dword_96E22++;
        if (dword_96E22 >= 15) {
            dword_96E22 = 0;
        }

        if (PlayerList[nPlayer].field_3FOUR >= SeqSize[var_3C])
        {
            if (PlayerList[nPlayer].field_38 == -1)
            {
                switch (PlayerList[nPlayer].field_3A)
                {
                    default:
                        break;

                    case 0:
                    {
                        PlayerList[nPlayer].field_3A = 1;
                        SetWeaponStatus(nPlayer);
                        break;
                    }
                    case 1:
                    {
                        if (PlayerList[nPlayer].bIsFiring)
                        {
                            if (!WeaponCanFire(nPlayer))
                            {
                                if (!dword_96E22) {
                                    D3PlayFX(StaticSound[4], PlayerList[nPlayer].nSprite);
                                }
                            }
                            else
                            {
                                if (nWeapon == kWeaponRing)
                                {
                                    if (Ra[nPlayer].nTarget == -1)
                                        break;

                                    Ra[nPlayer].nAction = 0;
                                    Ra[nPlayer].nFrame  = 0;
                                    Ra[nPlayer].field_C = 1;
                                }

                                PlayerList[nPlayer].field_3A = 2;

                                if (nWeapon == 0)
                                    break;

                                if (nWeapon == kWeaponGrenade)
                                {
                                    BuildGrenade(nPlayer);
                                    AddAmmo(nPlayer, 4, -1);
                                }
                                else if (nWeapon == kWeaponMummified)
                                {
                                    ShootStaff(nPlayer);
                                }
                            }
                        }
                        break;
                    }

                    case 2:
                    case 6:
                    case 7:
                    case 8:
                    {
                        if (nWeapon == kWeaponPistol && nPistolClip[nPlayer] <= 0)
                        {
                            PlayerList[nPlayer].field_3A = 3;
                            PlayerList[nPlayer].field_3FOUR = 0;

                            nPistolClip[nPlayer] = std::min<int>(6, PlayerList[nPlayer].nAmmo[kWeaponPistol]);
                            break;
                        }
                        else if (nWeapon == kWeaponGrenade)
                        {
                            if (!PlayerList[nPlayer].bIsFiring)
                            {
                                PlayerList[nPlayer].field_3A = 3;
                                break;
                            }
                            else
                            {
                                PlayerList[nPlayer].field_3FOUR = SeqSize[var_3C] - 1;
                                continue;
                            }
                        }
                        else if (nWeapon == kWeaponMummified)
                        {
                            PlayerList[nPlayer].field_3A = 0;
                            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].field_3C;

                            nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                            SetPlayerMummified(nPlayer, false);
                            break;
                        }
                        else
                        {
                            // loc_26D88:
                            if (PlayerList[nPlayer].bIsFiring && WeaponCanFire(nPlayer))
                            {
                                if (nWeapon != kWeaponM60 && nWeapon != kWeaponPistol) {
                                    PlayerList[nPlayer].field_3A = 3;
                                }
                            }
                            else
                            {
                                if (WeaponInfo[nWeapon].b[4] == -1)
                                {
                                    PlayerList[nPlayer].field_3A = 1;
                                }
                                else
                                {
                                    if (nWeapon == kWeaponFlamer && (nSectFlag & kSectUnderwater))
                                    {
                                        PlayerList[nPlayer].field_3A = 1;
                                    }
                                    else
                                    {
                                        PlayerList[nPlayer].field_3A = 4;
                                    }
                                }
                            }

                            break;
                        }
                    }

                    case 3:
                    case 9:
                    case 10:
                    case 11:
                    {
                        if (nWeapon == kWeaponMummified)
                        {
                            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].field_3C;

                            nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                            PlayerList[nPlayer].field_3A = 0;
                            break;
                        }
                        else if (nWeapon == kWeaponRing)
                        {
                            if (!WeaponInfo[nWeapon].d || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                            {
                                if (!PlayerList[nPlayer].bIsFiring) {
                                    PlayerList[nPlayer].field_3A = 1;
                                }
                                else {
                                    break;
                                }
                            }
                            else
                            {
                                SelectNewWeapon(nPlayer);
                            }

                            Ra[nPlayer].field_C = 0;
                            break;
                        }
                        else if (nWeapon == kWeaponM60)
                        {
                            CheckClip(nPlayer);
                            PlayerList[nPlayer].field_3A = 1;
                            break;
                        }
                        else if (nWeapon == kWeaponGrenade)
                        {
                            if (!WeaponInfo[nWeapon].d || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                            {
                                PlayerList[nPlayer].field_3A = 0;
                                break;
                            }
                            else
                            {
                                SelectNewWeapon(nPlayer);
                                PlayerList[nPlayer].field_3A = 5;

                                PlayerList[nPlayer].field_3FOUR = SeqSize[WeaponInfo[kWeaponGrenade].b[0] + SeqOffsets[nSeq]] - 1; // CHECKME
                                goto loc_flag; // FIXME
                            }
                        }
                        else
                        {
                            if (PlayerList[nPlayer].bIsFiring && WeaponCanFire(nPlayer)) {
                                PlayerList[nPlayer].field_3A = 2;
                                break;
                            }

                            if (WeaponInfo[nWeapon].b[4] == -1)
                            {
                                PlayerList[nPlayer].field_3A = 1;
                                break;
                            }

                            if (nWeapon == kWeaponFlamer && (nSectFlag & kSectUnderwater))
                            {
                                PlayerList[nPlayer].field_3A = 1;
                            }
                            else
                            {
                                PlayerList[nPlayer].field_3A = 4;
                            }
                        }
                        break;
                    }

                    case 4:
                    {
                        PlayerList[nPlayer].field_3A = 1;
                        break;
                    }

                    case 5:
                    {
                        PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].field_38;

                        nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                        PlayerList[nPlayer].field_3A = 0;
                        PlayerList[nPlayer].field_38 = -1;

                        SetWeaponStatus(nPlayer);
                        break;
                    }
                }

                // loc_26FC5
                var_3C = SeqOffsets[WeaponInfo[nWeapon].nSeq] + WeaponInfo[nWeapon].b[PlayerList[nPlayer].field_3A];
                PlayerList[nPlayer].field_3FOUR = 0;
            }
            else
            {
                if (PlayerList[nPlayer].field_3A == 5)
                {
                    PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].field_38;

                    nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                    PlayerList[nPlayer].field_38 = -1;
                    PlayerList[nPlayer].field_3A = 0;
                }
                else
                {
                    PlayerList[nPlayer].field_3A = 5;
                }

                PlayerList[nPlayer].field_3FOUR = 0;
                continue;
            }
        } // end of if (PlayerList[nPlayer].field_34 >= SeqSize[var_3C])

loc_flag:

        // loc_27001
        short nFrameFlag = seq_GetFrameFlag(var_3C, PlayerList[nPlayer].field_3FOUR);

        if (((!(nSectFlag & kSectUnderwater)) || nWeapon == kWeaponRing) && (nFrameFlag & 4))
        {
            BuildFlash(nPlayer, pPlayerSprite->sectnum, 512);
            AddFlash(
                pPlayerSprite->sectnum,
                pPlayerSprite->x,
                pPlayerSprite->y,
                pPlayerSprite->z,
                0);
        }

        if (nFrameFlag & 0x80)
        {
            int nAction = PlayerList[nPlayer].nAction;

            int var_38 = 1;

            if (nAction < 10 || nAction > 12) {
                var_38 = 0;
            }

            if (nPlayer == nLocalPlayer) {
                obobangle = bobangle = 512;
            }

            if (nWeapon == kWeaponFlamer && (!(nSectFlag & kSectUnderwater)))
            {
                nTemperature[nPlayer]++;

                if (nTemperature[nPlayer] > 50)
                {
                    nTemperature[nPlayer] = 0;
                    PlayerList[nPlayer].field_3A = 4;
                    PlayerList[nPlayer].field_3FOUR = 0;
                }
            }

            short nAmmoType = WeaponInfo[nWeapon].nAmmoType;
            short nAngle = pPlayerSprite->ang;
            int theX = pPlayerSprite->x;
            int theY = pPlayerSprite->y;
            int theZ = pPlayerSprite->z;

            int ebp = bcos(nAngle) * (pPlayerSprite->clipdist << 3);
            int ebx = bsin(nAngle) * (pPlayerSprite->clipdist << 3);

            if (WeaponInfo[nWeapon].c)
            {
                int ecx;

                int theVal = (totalmoves + 101) & (WeaponInfo[nWeapon].c - 1);
                if (theVal & 1)
                    ecx = -theVal;
                else
                    ecx = theVal;

                int var_44 = (nAngle + 512) & kAngleMask;
                ebp += bcos(var_44, -11) * ecx;
                ebx += bsin(var_44, -11) * ecx;
            }

            int nHeight = (-GetSpriteHeight(nPlayerSprite)) >> 1;

            if (nAction < 6)
            {
                nHeight -= 1792;
            }
            else
            {
                if (!var_38)
                {
                    nHeight += 1024;
                }
                else {
                    nHeight -= 2560;
                }
            }

            short nSectorB = pPlayerSprite->sectnum;

            switch (nWeapon)
            {
                // loc_27266:
                case kWeaponSword:
                {
                    nHeight += -PlayerList[nLocalPlayer].horizon.horiz.asq16() >> 10;

                    theZ += nHeight;

                    int var_28;

                    if (PlayerList[nPlayer].field_3A == 2) {
                        var_28 = 6;
                    }
                    else {
                        var_28 = 9;
                    }

                    int cRange = CheckCloseRange(nPlayer, &theX, &theY, &theZ, &nSectorB);

                    if (cRange)
                    {
                        short nDamage = BulletInfo[kWeaponSword].nDamage;

                        if (PlayerList[nPlayer].nDouble) {
                            nDamage *= 2;
                        }

                        if ((cRange & 0xC000) >= 0x8000)
                        {
                            if ((cRange & 0xC000) == 0x8000) // hit wall
                            {
                                // loc_2730E:
                                var_28 += 2;
                            }
                            else if ((cRange & 0xC000) == 0xC000) // hit sprite
                            {
                                short nSprite2 = cRange & 0x3FFF;

                                if (sprite[nSprite2].cstat & 0x50)
                                {
                                    var_28 += 2;
                                }
                                else if (sprite[nSprite2].statnum > 90 && sprite[nSprite2].statnum <= 199)
                                {
                                    runlist_DamageEnemy(nSprite2, nPlayerSprite, nDamage);

                                    if (sprite[nSprite2].statnum < 102) {
                                        var_28++;
                                    }
                                    else if (sprite[nSprite2].statnum == 102)
                                    {
                                        // loc_27370:
                                        BuildAnim(nullptr, 12, 0, theX, theY, theZ, nSectorB, 30, 0);
                                    }
                                    else if (sprite[nSprite2].statnum == kStatExplodeTrigger) {
                                        var_28 += 2;
                                    }
                                    else {
                                        var_28++;
                                    }
                                }
                                else
                                {
                                    // loc_27370:
                                    BuildAnim(nullptr, 12, 0, theX, theY, theZ, nSectorB, 30, 0);
                                }
                            }
                        }
                    }

                    // loc_27399:
                    PlayerList[nPlayer].field_3A = var_28;
                    PlayerList[nPlayer].field_3FOUR = 0;
                    break;
                }
                case kWeaponFlamer:
                {
                    if (nSectFlag & kSectUnderwater)
                    {
                        DoBubbles(nPlayer);
                        PlayerList[nPlayer].field_3A = 1;
                        PlayerList[nPlayer].field_3FOUR = 0;
                        StopSpriteSound(nPlayerSprite);
                        break;
                    }
                    else
                    {
                        if (var_38) {
                            nHeight += 768;
                        }
                        else {
                            nHeight -= 2560;
                        }

                        // fall through to case 1 (kWeaponPistol)
                        fallthrough__;
                    }
                }

                case kWeaponM60:
                {
                    if (nWeapon == kWeaponM60) { // hack(?) to do fallthrough from kWeapon3 into kWeaponPistol without doing the nQuake[] change
                        nQuake[nPlayer] = 128;
                    }
                    // fall through
                    fallthrough__;
                }
                case kWeaponPistol:
                {
                    int h = PlayerList[nLocalPlayer].horizon.horiz.asq16() >> 14;
                    nHeight -= h;

                    int target = 0;
                    if (sPlayerInput[nPlayer].nTarget >= 0 && Autoaim(nPlayer))
                    {
                        assert(sprite[sPlayerInput[nPlayer].nTarget].sectnum < kMaxSectors);
                        int t = sPlayerInput[nPlayer].nTarget;
                        if (t >= 0 && t < MAXSPRITES)
                        {
                            // only autoaim if target is in front of the player.
                            auto pTargetSprite = &sprite[t];
                            int angletotarget = bvectangbam(pTargetSprite->x - pPlayerSprite->x, pTargetSprite->y - pPlayerSprite->y).asbuild();
                            int anglediff = (pPlayerSprite->ang - angletotarget) & 2047;
                            if (anglediff < 512 || anglediff > 1536) target = t + 10000;
                        }
                    }

                    BuildBullet(nPlayerSprite, nAmmoType, ebp, ebx, nHeight, nAngle, target, var_1C);
                    break;
                }

                case kWeaponGrenade:
                {
                    ThrowGrenade(nPlayer, ebp, ebx, nHeight - 2560, FixedToInt(PlayerList[nLocalPlayer].horizon.horiz.asq16()));
                    break;
                }
                case kWeaponStaff:
                {
                    BuildSnake(nPlayer, nHeight);
                    nQuake[nPlayer] = 512;

                    nXDamage[nPlayer] -= bcos(pPlayerSprite->ang, 9);
                    nYDamage[nPlayer] -= bsin(pPlayerSprite->ang, 9);
                    break;
                }
                case kWeaponRing:
                    break;

                case kWeaponMummified:
                {
                    short nDamage = BulletInfo[kWeaponMummified].nDamage;
                    if (PlayerList[nPlayer].nDouble) {
                        nDamage *= 2;
                    }

                    runlist_RadialDamageEnemy(nPlayerSprite, nDamage, BulletInfo[kWeaponMummified].nRadius);
                    break;
                }
            }

            // end of switch, loc_2753E:
            if (nWeapon < kWeaponMummified)
            {
                if (nWeapon != kWeaponGrenade)
                {   
                    if (WeaponInfo[nWeapon].d) {
                        AddAmmo(nPlayer, nAmmoType, -1);
                    }

                    if (nWeapon == kWeaponM60) {
                        nPlayerClip[nPlayer]--;
                    }
                    else if (nWeapon == kWeaponPistol) {
                        nPistolClip[nPlayer]--;
                    }
                }

                if (!WeaponInfo[nWeapon].d || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                {
                    if (nWeapon == kWeaponM60 && nPlayerClip[nPlayer] <= 0)
                    {
                        PlayerList[nPlayer].field_3A = 3;
                        PlayerList[nPlayer].field_3FOUR = 0;
                        // goto loc_27609:
                    }
                }
                else if (nWeapon != kWeaponGrenade)
                {
                    SelectNewWeapon(nPlayer);
                    // go to loc_27609:
                }
            }
        }
    }
}

void DrawWeapons(double smooth)
{
    if (bCamera) {
        return;
    }

    short nWeapon = PlayerList[nLocalPlayer].nCurrentWeapon;
    if (nWeapon < -1) {
        return;
    }
    short var_34 = PlayerList[nLocalPlayer].field_3A;

    short var_30 = SeqOffsets[WeaponInfo[nWeapon].nSeq];

    short var_28 = var_30 + WeaponInfo[nWeapon].b[var_34];

    int8_t nShade = sector[initsect].ceilingshade;

    int nDouble = PlayerList[nLocalPlayer].nDouble;
    int nPal = kPalNormal;

    if (nDouble)
    {
        if (isRed) {
            nPal = kPalRedBrite;
        }

        isRed = isRed == 0;
    }

    nPal = RemapPLU(nPal);

    double xOffset = 0, yOffset = 0;

    if (cl_weaponsway)
    {
        double nBobAngle, nVal;

        if (cl_hudinterpolation)
        {
            nBobAngle = interpolatedangle(buildang(obobangle), buildang(bobangle), smooth).asbuildf();
            nVal = interpolatedvaluef(ototalvel[nLocalPlayer], totalvel[nLocalPlayer], smooth, 16) * 0.5;
        }
        else
        {
            nBobAngle = bobangle;
            nVal = totalvel[nLocalPlayer];
        }

        yOffset = MulScaleF(nVal, bsinf(fmod(nBobAngle, 1024.), -8), 9);

        if (var_34 == 1)
        {
            xOffset = MulScaleF(bcosf(nBobAngle, -8), nVal, 8);
        }
    }
    else
    {
        obobangle = bobangle = 512;
    }

    if (nWeapon == 3 && var_34 == 1) {
        seq_DrawPilotLightSeq(xOffset, yOffset);
    }

    if (nWeapon < 0) {
        nShade = sprite[PlayerList[nLocalPlayer].nSprite].shade;
    }

    double const look_anghalf = PlayerList[nLocalPlayer].angle.look_anghalf(smooth);
    double const looking_arc = PlayerList[nLocalPlayer].angle.looking_arc(smooth);

    xOffset -= look_anghalf;
    yOffset += looking_arc;

    seq_DrawGunSequence(var_28, PlayerList[nLocalPlayer].field_3FOUR, xOffset, yOffset, nShade, nPal);

    if (nWeapon != kWeaponM60)
        return;

    switch (var_34)
    {
        default:
            return;

        case 0:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            if (nClip <= 0)
                return;

            int nSeqOffset;

            if (nClip <= 3)
            {
                nSeqOffset = var_30 + 1;
            }
            else if (nClip <= 6)
            {
                nSeqOffset = var_30 + 2;
            }
            else if (nClip <= 25)
            {
                nSeqOffset = var_30 + 3;
            }
            else //if (nClip > 25)
            {
                nSeqOffset = var_30 + 4;
            }

            seq_DrawGunSequence(nSeqOffset, PlayerList[nLocalPlayer].field_3FOUR, xOffset, yOffset, nShade, nPal);
            return;
        }
        case 1:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short edx = (nClip % 3) * 4;

            if (nClip <= 0) {
                return;
            }

            seq_DrawGunSequence(var_30 + 8, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 3) {
                return;
            }

            seq_DrawGunSequence(var_30 + 9, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 6) {
                return;
            }

            seq_DrawGunSequence(var_30 + 10, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 25) {
                return;
            }

            seq_DrawGunSequence(var_30 + 11, edx, xOffset, yOffset, nShade, nPal);
            return;
        }
        case 2:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short dx = PlayerList[nLocalPlayer].field_3FOUR;

            if (nClip <= 0) {
                return;
            }

            seq_DrawGunSequence(var_30 + 8, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 3) {
                return;
            }

            seq_DrawGunSequence(var_30 + 9, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 6) {
                return;
            }

            seq_DrawGunSequence(var_30 + 10, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 25) {
                return;
            }

            seq_DrawGunSequence(var_30 + 11, dx, xOffset, yOffset, nShade, nPal);
            return;
        }

        case 3:
        case 4:
            return;

        case 5:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short ax = PlayerList[nLocalPlayer].field_3FOUR;

            if (nClip <= 0) {
                return;
            }

            int nSeqOffset;

            if (nClip <= 3)
            {
                nSeqOffset = var_30 + 20;
            }
            else if (nClip <= 6)
            {
                nSeqOffset = var_30 + 19;
            }
            else if (nClip <= 25)
            {
                nSeqOffset = var_30 + 18;
            }
            else
            {
                nSeqOffset = var_30 + 17;
            }

            seq_DrawGunSequence(nSeqOffset, ax, xOffset, yOffset, nShade, nPal);
            return;
        }
    }
}
END_PS_NS
