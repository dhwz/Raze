//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "ns.h"
#include "build.h"

#include "names2.h"
#include "game.h"
#include "tags.h"
#include "ai.h"
#include "misc.h"

BEGIN_SW_NS

DECISION SkelBattle[] =
{
    {600,   &AF(InitActorMoveCloser)         },
    {602,   &AF(InitActorSetDecide)         },
    {700,   &AF(InitActorRunAway   )         },
    {1024,  &AF(InitActorAttack    )         }
};

DECISION SkelOffense[] =
{
    {700,   &AF(InitActorMoveCloser)         },
    {702,   &AF(InitActorSetDecide)         },
    {1024,  &AF(InitActorAttack    )         }
};

DECISIONB SkelBroadcast[] =
{
    {3,    attr_alert      },
    {6,    attr_ambient      },
    {1024, 0  }
};

DECISION SkelSurprised[] =
{
    {701,   &AF(InitActorMoveCloser)         },
    {1024,  &AF(InitActorDecide    )        }
};

DECISION SkelEvasive[] =
{
    {22,     &AF(InitActorDuck )            },
    {30,     &AF(InitActorEvade)            },
    {1024,   nullptr                      },
};

DECISION SkelLostTarget[] =
{
    {900,   &AF(InitActorFindPlayer  )       },
    {1024,  &AF(InitActorWanderAround)       }
};

DECISION SkelCloseRange[] =
{
    {800,  &AF(InitActorAttack    )         },
    {1024, &AF(InitActorReposition)            }
};

PERSONALITY SkelPersonality =
{
    SkelBattle,
    SkelOffense,
    SkelBroadcast,
    SkelSurprised,
    SkelEvasive,
    SkelLostTarget,
    SkelCloseRange,
    SkelCloseRange
};

ATTRIBUTE SkelAttrib =
{
    {60, 80, 100, 130},                 // Speeds
    {3, 0, -2, -3},                     // Tic Adjusts
    3,                                 // MaxWeapons;
    {
        DIGI_SPAMBIENT, DIGI_SPALERT, 0,
        DIGI_SPPAIN, DIGI_SPSCREAM, DIGI_SPBLADE,
        DIGI_SPELEC,DIGI_SPTELEPORT,0,0
    }
};

//////////////////////
//
// SKEL RUN
//
//////////////////////

#define SKEL_RUN_RATE 12

// +4 on frame #3 to add character

STATE s_SkelRun[5][6] =
{
    {
        {SKEL_RUN_R0 + 0, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[0][1]},
        {SKEL_RUN_R0 + 1, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[0][2]},
        {SKEL_RUN_R0 + 2, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[0][3]},
        {SKEL_RUN_R0 + 3, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[0][4]},
        {SKEL_RUN_R0 + 4, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[0][5]},
        {SKEL_RUN_R0 + 5, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[0][0]},
    },
    {
        {SKEL_RUN_R1 + 0, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[1][1]},
        {SKEL_RUN_R1 + 1, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[1][2]},
        {SKEL_RUN_R1 + 2, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[1][3]},
        {SKEL_RUN_R1 + 3, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[1][4]},
        {SKEL_RUN_R1 + 4, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[1][5]},
        {SKEL_RUN_R1 + 5, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[1][0]},
    },
    {
        {SKEL_RUN_R2 + 0, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[2][1]},
        {SKEL_RUN_R2 + 1, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[2][2]},
        {SKEL_RUN_R2 + 2, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[2][3]},
        {SKEL_RUN_R2 + 3, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[2][4]},
        {SKEL_RUN_R2 + 4, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[2][5]},
        {SKEL_RUN_R2 + 5, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[2][0]},
    },
    {
        {SKEL_RUN_R3 + 0, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[3][1]},
        {SKEL_RUN_R3 + 1, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[3][2]},
        {SKEL_RUN_R3 + 2, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[3][3]},
        {SKEL_RUN_R3 + 3, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[3][4]},
        {SKEL_RUN_R3 + 4, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[3][5]},
        {SKEL_RUN_R3 + 5, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[3][0]},
    },
    {
        {SKEL_RUN_R4 + 0, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[4][1]},
        {SKEL_RUN_R4 + 1, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[4][2]},
        {SKEL_RUN_R4 + 2, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[4][3]},
        {SKEL_RUN_R4 + 3, SKEL_RUN_RATE+4, &AF(DoSkelMove), &s_SkelRun[4][4]},
        {SKEL_RUN_R4 + 4, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[4][5]},
        {SKEL_RUN_R4 + 5, SKEL_RUN_RATE, &AF(DoSkelMove), &s_SkelRun[4][0]},
    }
};


STATE* sg_SkelRun[] =
{
    &s_SkelRun[0][0],
    &s_SkelRun[1][0],
    &s_SkelRun[2][0],
    &s_SkelRun[3][0],
    &s_SkelRun[4][0]
};

//////////////////////
//
// SKEL SLASH
//
//////////////////////

#define SKEL_SLASH_RATE 20

STATE s_SkelSlash[5][7] =
{
    {
        {SKEL_SLASH_R0 + 0, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[0][1]},
        {SKEL_SLASH_R0 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[0][2]},
        {SKEL_SLASH_R0 + 2, 0|SF_QUICK_CALL, &AF(InitSkelSlash), &s_SkelSlash[0][3]},
        {SKEL_SLASH_R0 + 2, SKEL_SLASH_RATE*2, &AF(NullSkel), &s_SkelSlash[0][4]},
        {SKEL_SLASH_R0 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[0][5]},
        {SKEL_SLASH_R0 + 1, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSlash[0][6]},
        {SKEL_SLASH_R0 + 1, SKEL_SLASH_RATE, &AF(DoSkelMove), &s_SkelSlash[0][6]},
    },
    {
        {SKEL_SLASH_R1 + 0, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[1][1]},
        {SKEL_SLASH_R1 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[1][2]},
        {SKEL_SLASH_R1 + 2, 0|SF_QUICK_CALL, &AF(InitSkelSlash), &s_SkelSlash[1][3]},
        {SKEL_SLASH_R1 + 2, SKEL_SLASH_RATE*2, &AF(NullSkel), &s_SkelSlash[1][4]},
        {SKEL_SLASH_R1 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[1][5]},
        {SKEL_SLASH_R1 + 1, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSlash[1][6]},
        {SKEL_SLASH_R1 + 1, SKEL_SLASH_RATE, &AF(DoSkelMove), &s_SkelSlash[1][6]},
    },
    {
        {SKEL_SLASH_R2 + 0, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[2][1]},
        {SKEL_SLASH_R2 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[2][2]},
        {SKEL_SLASH_R2 + 2, 0|SF_QUICK_CALL, &AF(InitSkelSlash), &s_SkelSlash[2][3]},
        {SKEL_SLASH_R2 + 2, SKEL_SLASH_RATE*2, &AF(NullSkel), &s_SkelSlash[2][4]},
        {SKEL_SLASH_R2 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[2][5]},
        {SKEL_SLASH_R2 + 1, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSlash[2][6]},
        {SKEL_SLASH_R2 + 1, SKEL_SLASH_RATE, &AF(DoSkelMove), &s_SkelSlash[2][6]},
    },
    {
        {SKEL_SLASH_R3 + 0, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[3][1]},
        {SKEL_SLASH_R3 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[3][2]},
        {SKEL_SLASH_R3 + 2, 0|SF_QUICK_CALL, &AF(InitSkelSlash), &s_SkelSlash[3][3]},
        {SKEL_SLASH_R3 + 2, SKEL_SLASH_RATE*2, &AF(NullSkel), &s_SkelSlash[3][4]},
        {SKEL_SLASH_R3 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[3][5]},
        {SKEL_SLASH_R3 + 1, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSlash[3][6]},
        {SKEL_SLASH_R3 + 1, SKEL_SLASH_RATE, &AF(DoSkelMove), &s_SkelSlash[3][6]},
    },
    {
        {SKEL_SLASH_R4 + 0, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[4][1]},
        {SKEL_SLASH_R4 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[4][2]},
        {SKEL_SLASH_R4 + 2, 0|SF_QUICK_CALL, &AF(InitSkelSlash), &s_SkelSlash[4][3]},
        {SKEL_SLASH_R4 + 2, SKEL_SLASH_RATE*2, &AF(NullSkel), &s_SkelSlash[4][4]},
        {SKEL_SLASH_R4 + 1, SKEL_SLASH_RATE, &AF(NullSkel), &s_SkelSlash[4][5]},
        {SKEL_SLASH_R4 + 1, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSlash[4][6]},
        {SKEL_SLASH_R4 + 1, SKEL_SLASH_RATE, &AF(DoSkelMove), &s_SkelSlash[4][6]},
    }
};


STATE* sg_SkelSlash[] =
{
    &s_SkelSlash[0][0],
    &s_SkelSlash[1][0],
    &s_SkelSlash[2][0],
    &s_SkelSlash[3][0],
    &s_SkelSlash[4][0]
};


//////////////////////
//
// SKEL SPELL
//
//////////////////////

#define SKEL_SPELL_RATE 20

STATE s_SkelSpell[5][7] =
{
    {
        {SKEL_SPELL_R0 + 0, SKEL_SPELL_RATE+9, &AF(NullSkel), &s_SkelSpell[0][1]},
        {SKEL_SPELL_R0 + 1, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[0][2]},
        {SKEL_SPELL_R0 + 2, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[0][3]},
        {SKEL_SPELL_R0 + 3, SKEL_SPELL_RATE*2, &AF(NullSkel), &s_SkelSpell[0][4]},
        {SKEL_SPELL_R0 + 3, 0|SF_QUICK_CALL, &AF(InitSkelSpell), &s_SkelSpell[0][5]},
        {SKEL_SPELL_R0 + 3, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSpell[0][6]},
        {SKEL_SPELL_R0 + 3, SKEL_SPELL_RATE, &AF(DoSkelMove), &s_SkelSpell[0][6]},
    },
    {
        {SKEL_SPELL_R1 + 0, SKEL_SPELL_RATE+9, &AF(NullSkel), &s_SkelSpell[1][1]},
        {SKEL_SPELL_R1 + 1, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[1][2]},
        {SKEL_SPELL_R1 + 2, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[1][3]},
        {SKEL_SPELL_R1 + 3, SKEL_SPELL_RATE*2, &AF(NullSkel), &s_SkelSpell[1][4]},
        {SKEL_SPELL_R1 + 3, 0|SF_QUICK_CALL, &AF(InitSkelSpell), &s_SkelSpell[1][5]},
        {SKEL_SPELL_R1 + 3, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSpell[1][6]},
        {SKEL_SPELL_R1 + 3, SKEL_SPELL_RATE, &AF(DoSkelMove), &s_SkelSpell[1][6]},
    },
    {
        {SKEL_SPELL_R2 + 0, SKEL_SPELL_RATE+9, &AF(NullSkel), &s_SkelSpell[2][1]},
        {SKEL_SPELL_R2 + 1, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[2][2]},
        {SKEL_SPELL_R2 + 2, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[2][3]},
        {SKEL_SPELL_R2 + 3, SKEL_SPELL_RATE*2, &AF(NullSkel), &s_SkelSpell[2][4]},
        {SKEL_SPELL_R2 + 3, 0|SF_QUICK_CALL, &AF(InitSkelSpell), &s_SkelSpell[2][5]},
        {SKEL_SPELL_R2 + 3, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSpell[2][6]},
        {SKEL_SPELL_R2 + 3, SKEL_SPELL_RATE, &AF(DoSkelMove), &s_SkelSpell[2][6]},
    },
    {
        {SKEL_SPELL_R3 + 0, SKEL_SPELL_RATE+9, &AF(NullSkel), &s_SkelSpell[3][1]},
        {SKEL_SPELL_R3 + 1, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[3][2]},
        {SKEL_SPELL_R3 + 2, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[3][3]},
        {SKEL_SPELL_R3 + 3, SKEL_SPELL_RATE*2, &AF(NullSkel), &s_SkelSpell[3][4]},
        {SKEL_SPELL_R3 + 3, 0|SF_QUICK_CALL, &AF(InitSkelSpell), &s_SkelSpell[3][5]},
        {SKEL_SPELL_R3 + 3, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSpell[3][6]},
        {SKEL_SPELL_R3 + 3, SKEL_SPELL_RATE, &AF(DoSkelMove), &s_SkelSpell[3][6]},
    },
    {
        {SKEL_SPELL_R4 + 0, SKEL_SPELL_RATE+9, &AF(NullSkel), &s_SkelSpell[4][1]},
        {SKEL_SPELL_R4 + 1, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[4][2]},
        {SKEL_SPELL_R4 + 2, SKEL_SPELL_RATE, &AF(NullSkel), &s_SkelSpell[4][3]},
        {SKEL_SPELL_R4 + 3, SKEL_SPELL_RATE*2, &AF(NullSkel), &s_SkelSpell[4][4]},
        {SKEL_SPELL_R4 + 3, 0|SF_QUICK_CALL, &AF(InitSkelSpell), &s_SkelSpell[4][5]},
        {SKEL_SPELL_R4 + 3, 0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelSpell[4][6]},
        {SKEL_SPELL_R4 + 3, SKEL_SPELL_RATE, &AF(DoSkelMove), &s_SkelSpell[4][6]},
    }
};


STATE* sg_SkelSpell[] =
{
    &s_SkelSpell[0][0],
    &s_SkelSpell[1][0],
    &s_SkelSpell[2][0],
    &s_SkelSpell[3][0],
    &s_SkelSpell[4][0]
};

//////////////////////
//
// SKEL PAIN
//
//////////////////////

#define SKEL_PAIN_RATE 38

STATE s_SkelPain[5][1] =
{
    {
        {SKEL_PAIN_R0 + 0, SKEL_PAIN_RATE, &AF(DoSkelPain), &s_SkelPain[0][0]},
    },
    {
        {SKEL_PAIN_R1 + 0, SKEL_PAIN_RATE, &AF(DoSkelPain), &s_SkelPain[1][0]},
    },
    {
        {SKEL_PAIN_R2 + 0, SKEL_PAIN_RATE, &AF(DoSkelPain), &s_SkelPain[2][0]},
    },
    {
        {SKEL_PAIN_R3 + 0, SKEL_PAIN_RATE, &AF(DoSkelPain), &s_SkelPain[3][0]},
    },
    {
        {SKEL_PAIN_R4 + 0, SKEL_PAIN_RATE, &AF(DoSkelPain), &s_SkelPain[4][0]},
    }
};

STATE* sg_SkelPain[] =
{
    &s_SkelPain[0][0],
    &s_SkelPain[1][0],
    &s_SkelPain[2][0],
    &s_SkelPain[3][0],
    &s_SkelPain[4][0]
};



//////////////////////
//
// SKEL TELEPORT
//
//////////////////////

#define SKEL_TELEPORT_RATE 20

STATE s_SkelTeleport[] =
{
    {SKEL_TELEPORT + 0,  1,                  nullptr,  &s_SkelTeleport[1]},
    {SKEL_TELEPORT + 0,  0|SF_QUICK_CALL,    &AF(DoSkelInitTeleport), &s_SkelTeleport[2]},
    {SKEL_TELEPORT + 0,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[3]},
    {SKEL_TELEPORT + 1,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[4]},
    {SKEL_TELEPORT + 2,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[5]},
    {SKEL_TELEPORT + 3,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[6]},
    {SKEL_TELEPORT + 4,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[7]},
    {SKEL_TELEPORT + 5,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[8]},

    {SKEL_TELEPORT + 5,  0|SF_QUICK_CALL,    &AF(DoSkelTeleport), &s_SkelTeleport[9]},

    {SKEL_TELEPORT + 5,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[10]},
    {SKEL_TELEPORT + 4,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[11]},
    {SKEL_TELEPORT + 3,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[12]},
    {SKEL_TELEPORT + 2,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[13]},
    {SKEL_TELEPORT + 1,  SKEL_TELEPORT_RATE, nullptr,  &s_SkelTeleport[14]},
    {SKEL_TELEPORT + 0,  SKEL_TELEPORT_RATE, &AF(DoSkelTermTeleport), &s_SkelTeleport[15]},
    {SKEL_TELEPORT + 0,  0|SF_QUICK_CALL, &AF(InitActorDecide), &s_SkelTeleport[16]},
    {SKEL_TELEPORT + 0,  SKEL_TELEPORT_RATE, &AF(DoSkelMove), &s_SkelTeleport[16]},
};

STATE* sg_SkelTeleport[] =
{
    s_SkelTeleport,
    s_SkelTeleport,
    s_SkelTeleport,
    s_SkelTeleport,
    s_SkelTeleport
};

//////////////////////
//
// SKEL STAND
//
//////////////////////

#define SKEL_STAND_RATE 12

STATE s_SkelStand[5][1] =
{
    {
        {SKEL_RUN_R0 + 0, SKEL_STAND_RATE, &AF(DoSkelMove), &s_SkelStand[0][0]},
    },
    {
        {SKEL_RUN_R1 + 0, SKEL_STAND_RATE, &AF(DoSkelMove), &s_SkelStand[1][0]},
    },
    {
        {SKEL_RUN_R2 + 0, SKEL_STAND_RATE, &AF(DoSkelMove), &s_SkelStand[2][0]},
    },
    {
        {SKEL_RUN_R3 + 0, SKEL_STAND_RATE, &AF(DoSkelMove), &s_SkelStand[3][0]},
    },
    {
        {SKEL_RUN_R4 + 0, SKEL_STAND_RATE, &AF(DoSkelMove), &s_SkelStand[4][0]},
    },
};


STATE* sg_SkelStand[] =
{
    s_SkelStand[0],
    s_SkelStand[1],
    s_SkelStand[2],
    s_SkelStand[3],
    s_SkelStand[4]
};

//////////////////////
//
// SKEL DIE
//
//////////////////////

#define SKEL_DIE_RATE 25

STATE s_SkelDie[] =
{
    {SKEL_DIE + 0, SKEL_DIE_RATE, nullptr,  &s_SkelDie[1]},
    {SKEL_DIE + 1, SKEL_DIE_RATE, nullptr,  &s_SkelDie[2]},
    {SKEL_DIE + 2, SKEL_DIE_RATE, nullptr,  &s_SkelDie[3]},
    {SKEL_DIE + 3, SKEL_DIE_RATE, nullptr,  &s_SkelDie[4]},
    {SKEL_DIE + 4, SKEL_DIE_RATE, nullptr,  &s_SkelDie[5]},
    {SKEL_DIE + 5, SKEL_DIE_RATE, &AF(DoSuicide),   &s_SkelDie[5]},
};

STATE* sg_SkelDie[] =
{
    s_SkelDie
};

/*
STATE* *Stand[MAX_WEAPONS];
STATE* *Run;
STATE* *Jump;
STATE* *Fall;
STATE* *Crawl;
STATE* *Swim;
STATE* *Fly;
STATE* *Rise;
STATE* *Sit;
STATE* *Look;
STATE* *Climb;
STATE* *Pain;
STATE* *Death1;
STATE* *Death2;
STATE* *Dead;
STATE* *DeathJump;
STATE* *DeathFall;
STATE* *CloseAttack[2];
STATE* *Attack[6];
STATE* *Special[2];
*/

ACTOR_ACTION_SET SkelActionSet =
{
    sg_SkelStand,
    sg_SkelRun,
    nullptr, //sg_SkelJump,
    nullptr, //sg_SkelFall,
    nullptr, //sg_SkelCrawl,
    nullptr, //sg_SkelSwim,
    nullptr, //sg_SkelFly,
    nullptr, //sg_SkelRise,
    nullptr, //sg_SkelSit,
    nullptr, //sg_SkelLook,
    nullptr, //climb
    sg_SkelPain, //pain
    sg_SkelDie,
    nullptr, //sg_SkelHariKari,
    nullptr, //sg_SkelDead,
    nullptr, //sg_SkelDeathJump,
    nullptr, //sg_SkelDeathFall,
    {sg_SkelSlash},
    {1024},
    {sg_SkelSpell},
    {1024},
    {nullptr},
    sg_SkelTeleport,
    nullptr
};

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int SetupSkel(DSWActor* actor)
{
    if (!(actor->spr.cstat & CSTAT_SPRITE_RESTORE))
    {
        SpawnUser(actor,SKEL_RUN_R0,s_SkelRun[0]);
        actor->user.Health = HEALTH_SKEL_PRIEST;
    }

    ChangeState(actor, s_SkelRun[0]);
    actor->user.__legacyState.Attrib = &SkelAttrib;
    DoActorSetSpeed(actor, NORM_SPEED);
    actor->user.__legacyState.StateEnd = s_SkelDie;
    actor->user.__legacyState.Rot = sg_SkelRun;

    EnemyDefaults(actor, &SkelActionSet, &SkelPersonality);

    // 256 is default
    //actor->clipdist = 16;
    actor->user.Flags |= (SPR_XFLIP_TOGGLE);

    return 0;
}

DEFINE_ACTION_FUNCTION(DSWSkeleton, Initialize)
{
    PARAM_SELF_PROLOGUE(DSWActor);
    SetupSkel(self);
    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int DoSkelInitTeleport(DSWActor* actor)
{
    actor->spr.cstat &= ~(CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
    PlaySpriteSound(actor,attr_extra3,v3df_follow);
    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int DoSkelTeleport(DSWActor* actor)
{
    auto pos = actor->spr.pos;

    while (true)
    {
        pos.XY() = actor->spr.pos.XY();

        if (RANDOM_P2(1024) < 512)
            pos.X += 32 + RANDOM_P2F(64, 4);
        else
            pos.X -= 32 + RANDOM_P2F(64, 4);

        if (RANDOM_P2(1024) < 512)
            pos.Y += 32 + RANDOM_P2F(64, 4);
        else
            pos.Y -= 32 + RANDOM_P2F(64, 4);

        SetActorZ(actor, pos);

        if (actor->insector())
            break;
    }

    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int DoSkelTermTeleport(DSWActor* actor)
{
    actor->spr.cstat |= (CSTAT_SPRITE_BLOCK|CSTAT_SPRITE_BLOCK_HITSCAN);
    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int NullSkel(DSWActor* actor)
{
    if (actor->user.Flags & (SPR_SLIDING))
        DoActorSlide(actor);

    KeepActorOnFloor(actor);
    DoActorSectorDamage(actor);

    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int DoSkelPain(DSWActor* actor)
{
    NullSkel(actor);

    if ((actor->user.WaitTics -= ACTORMOVETICS) <= 0)
        InitActorDecide(actor);

    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

int DoSkelMove(DSWActor* actor)
{
    if (actor->user.Flags & (SPR_SLIDING))
        DoActorSlide(actor);

    if (actor->user.track >= 0)
        ActorFollowTrack(actor, ACTORMOVETICS);
    else
        actor->callAction();

    KeepActorOnFloor(actor);

    DoActorSectorDamage(actor);

    return 0;
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------


#include "saveable.h"

static saveable_data saveable_skel_data[] =
{
    SAVE_DATA(SkelPersonality),

    SAVE_DATA(SkelAttrib),

    SAVE_DATA(s_SkelRun),
    SAVE_DATA(sg_SkelRun),
    SAVE_DATA(s_SkelSlash),
    SAVE_DATA(sg_SkelSlash),
    SAVE_DATA(s_SkelSpell),
    SAVE_DATA(sg_SkelSpell),
    SAVE_DATA(s_SkelPain),
    SAVE_DATA(sg_SkelPain),
    SAVE_DATA(s_SkelTeleport),
    SAVE_DATA(sg_SkelTeleport),
    SAVE_DATA(s_SkelStand),
    SAVE_DATA(sg_SkelStand),
    SAVE_DATA(s_SkelDie),
    SAVE_DATA(sg_SkelDie),

    SAVE_DATA(SkelActionSet),
};

saveable_module saveable_skel =
{
    // code
    nullptr, 0,

    // data
    saveable_skel_data,
    SIZ(saveable_skel_data)
};
END_SW_NS
