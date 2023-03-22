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

#pragma once 

#include "gamecontrol.h"
#include "gameinput.h"

BEGIN_PS_NS

void SetSavePoint(int nPlayer, const DVector3& pos, sectortype* pSector, DAngle nAngle);
void InitPlayer();
void InitPlayerKeys(int nPlayer);
int GrabPlayer();
void InitPlayerInventory(int nPlayer);
void RestartPlayer(int nPlayer);

enum
{
	kMaxPlayers			= 8,
	kDefaultLives		= 3,
	kMaxPlayerLives		= 5,
	kMaxHealth			= 800
};

extern int nLocalPlayer;

struct PlayerSave
{
    sectortype* pSector;
    DVector3 pos;
    DAngle nAngle;
};

struct Player
{
    DExhumedActor* pActor;
    int16_t nHealth;
    int16_t nLives;
    int16_t nDouble;
    int16_t nInvisible;
    int16_t nTorch;
    int16_t nSeqSize;
    int16_t nAction;
    int16_t bIsMummified;
    int16_t invincibility;
    int16_t nAir;
    int16_t nSeq;
    int16_t nMaskAmount;
    uint16_t keys;
    int16_t nMagic;
    int16_t nItem;
    int8_t nCurrentItem;
    int8_t items[8];
    int16_t nAmmo[7]; // TODO - kMaxWeapons?

    int16_t nCurrentWeapon;
    int16_t nSeqSize2;
    int16_t bIsFiring;
    int16_t nNextWeapon;
    int16_t nState;
    int16_t nLastWeapon;
    int16_t nRun;
    bool bPlayerPan, bLockPan;
    DAngle nDestVertPan;

    InputPacket input;
    PlayerAngles Angles;
    DVector2 vel;
    sectortype* pPlayerPushSect;
    sectortype* pPlayerViewSect;

    int16_t nBreathTimer;
    int16_t nPlayerSwear;
    int16_t nDeathType;
    int16_t nPlayerScore;
    int16_t nPlayerColor;
    int16_t nPistolClip;
    DVector2 nPlayerD;
    DVector2 nThrust;
    int16_t nPlayerOldWeapon;
    int16_t nPlayerClip;
    int16_t nPlayerPushSound;
    int16_t nTauntTimer;
    uint16_t nPlayerWeapons; // each set bit represents a weapon the player has
    PlayerSave sPlayerSave;
    int ototalvel;
    int totalvel;
    TObjPtr<DExhumedActor*> pPlayerGrenade;
    TObjPtr<DExhumedActor*> pPlayerFloorSprite;
    TObjPtr<DExhumedActor*> pDoppleSprite;
    TObjPtr<DExhumedActor*> pTarget;

};

extern int PlayerCount;

extern Player PlayerList[kMaxPlayers];

extern int obobangle, bobangle;

extern TObjPtr<DExhumedActor*> nNetStartSprite[kMaxPlayers];
extern int nNetStartSprites;
extern int nCurStartSprite;

int GetPlayerFromActor(DExhumedActor* actor);
void SetPlayerMummified(int nPlayer, int bIsMummified);
int AddAmmo(int nPlayer, int nWeapon, int nAmmoAmount);
void ShootStaff(int nPlayer);

END_PS_NS

