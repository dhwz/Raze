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

#ifndef GAME_H

#define GAME_H

#ifndef DEBUG
#define DEBUG 0
#endif

#ifdef _MSC_VER
#pragma warning(disable:4101) // there's too many of these... :(
#endif

#include "build.h"
#include "d_net.h"
#include "gamefuncs.h"
#include "coreactor.h"

#include "mytypes.h"
#include "sounds.h"
#include "gamecvars.h"
#include "raze_sound.h"
#include "c_cvars.h"
#include "mapinfo.h"
#include "gamecontrol.h"
#include "gamestruct.h"
#include "packet.h"
#include "gameinput.h"
#include "serialize_obj.h"

EXTERN_CVAR(Bool, sw_ninjahack)
EXTERN_CVAR(Bool, sw_darts)
EXTERN_CVAR(Bool, sw_bunnyrockets)

BEGIN_SW_NS

class DSWActor;
using HitInfo = THitInfo<DSWActor>;
using Collision = TCollision<DSWActor>;


typedef struct
{
    // Net Options from Menus
    uint8_t NetGameType;   // 0=DeathMatch [spawn], 1=Cooperative 2=DeathMatch [no spawn]
    uint8_t NetMonsters;   // Cycle skill levels
    bool NetHurtTeammate;  // Allow friendly kills
    bool NetSpawnMarkers;    // Respawn markers on/off
    bool NetTeamPlay;   // Team play
    uint8_t NetKillLimit;  // Number of frags at which game ends
    uint8_t NetTimeLimit;  // Limit time of game
    uint8_t NetColor;      // Chosen color for player
    bool NetNuke;
} GAME_SET, * GAME_SETp;

extern const GAME_SET gs_defaults;
extern GAME_SET gs;

enum
{
    DREALMSPAL = 1,

    MAXMIRRORS          = 8,
    // This is just some, high, blank tile number not used
    // by real graphics to put the MAXMIRRORS mirrors in
    MIRRORLABEL         = 6000,

};

//#define SW_SHAREWARE 1     // This determines whether game is shareware compile or not!
#define SW_SHAREWARE (!!(g_gameType & GAMEFLAG_SHAREWARE))

// Turn warning off for unreferenced variables.
// I really should fix them at some point
//#pragma off(unreferenced)


#define PRODUCTION_ASSERT(f) \
    do { \
        if (!(f)) \
            I_FatalError("Assertion failed: %s %s, line %u", #f, __FILE__, __LINE__); \
    } while (0)

#define ASSERT assert

#define MONO_PRINT(str)


#define HEAP_CHECK()
#define RANDOM_DEBUG 0


int RandomRange(int);
inline int RANDOM(void)
{
    randomseed = ((randomseed * 21 + 1) & 65535);
    return randomseed;
}

#define RANDOM_P2(pwr_of_2) (MOD_P2(RANDOM(),(pwr_of_2)))

//
// Map directions/degrees
//

#if 0
y--
^ 1536
|
|
|
|
|
|            2047
<---------------------------->
1024         |              0
x--          |             x++
|
|
|
|
V 512
y++

#endif

//////////////////////////////////////////////////////
//
// KEYBOARD
//
//////////////////////////////////////////////////////

extern bool MenuInputMode;

//
// Defines
//

#define CIRCLE_CAMERA_DIST_MIN 12000

// dist at which actors will not move (unless shot?? to do)
#define MAX_ACTIVE_RANGE 42000
// dist at which actors roam about on their own
#define MIN_ACTIVE_RANGE 20000

inline int32_t FIXED(int32_t msw, int32_t lsw)
{
    return IntToFixed(msw) | lsw;
}

// Ouch...
#ifndef WORDS_BIGENDIAN
# define MSB_VAR(fixed) (*(((uint8_t*)&(fixed)) + 1))
# define LSB_VAR(fixed) (*((uint8_t*)&(fixed)))
#else

# define LSB_VAR(fixed) (*(((uint8_t*)&(fixed)) + 1))
# define MSB_VAR(fixed) (*((uint8_t*)&(fixed)))
#endif

#define MSW(fixed) ((fixed)>>16)
#define LSW(fixed) (((int16_t)(fixed)))

// Defines for reading in ST1 sprite tagging
#define SP_TAG1(sp) ((sp)->hitag)
#define SP_TAG2(sp) ((sp)->lotag)
#define SP_TAG3(sp) ((sp)->clipdist)
#define SP_TAG4(sp) ((sp)->ang)
#define SP_TAG5(sp) ((sp)->xvel)
#define SP_TAG6(sp) ((sp)->yvel)
#define SP_TAG7(sp) (MSB_VAR((sp)->zvel))
#define SP_TAG8(sp) (LSB_VAR((sp)->zvel))
#define SP_TAG9(sp) (MSB_VAR((sp)->owner))
#define SP_TAG10(sp) (LSB_VAR((sp)->owner))
#define SP_TAG11(sp) ((sp)->shade)
#define SP_TAG12(sp) ((sp)->pal)
#define SP_TAG13(sp) LittleShort(*((int16_t*)&(sp)->xoffset))
#define SP_TAG14(sp) LittleShort(*((int16_t*)&(sp)->xrepeat))
#define SP_TAG15(sp) ((sp)->z)
#define SET_SP_TAG13(sp,val) (*((int16_t*)&(sp)->xoffset)) = LittleShort((int16_t)val)
#define SET_SP_TAG14(sp,val) (*((int16_t*)&(sp)->xrepeat)) = LittleShort((int16_t)val)

#define TRAVERSE_CONNECT(i)   for (i = connecthead; i != -1; i = connectpoint2[i])


#define NORM_ANGLE(ang) ((ang) & 2047)
#define ANGLE_2_PLAYER(pp,x,y) (NORM_ANGLE(getangle(pp->posx-(x), pp->posy-(y))))
#define NORM_Q16ANGLE(ang) ((ang) & 0x7FFFFFF)

int StdRandomRange(int range);
#define STD_RANDOM_P2(pwr_of_2) (MOD_P2(rand(),(pwr_of_2)))
#define STD_RANDOM_RANGE(range) (StdRandomRange(range))
#define STD_RANDOM() (rand())

#define RANDOM_NEG(x,y) ((RANDOM_P2(((x)<<(y))<<1) - (x))<<(y))

#define MOVEx(vel,ang) (MulScale(vel, bcos(ang), 14))
#define MOVEy(vel,ang) (MulScale(vel, bsin(ang), 14))

#define DIST(x1, y1, x2, y2) ksqrt( SQ((x1) - (x2)) + SQ((y1) - (y2)) )

inline int PIC_SIZY(spritetype* sp)
{
    return tileHeight(sp->picnum);
}

// Distance macro - tx, ty, tmin are holding vars that must be declared in the routine
// that uses this macro
inline void DISTANCE(int x1, int y1, int x2, int y2, int& dist, int& tx, int& ty, int& tmin)
{
	tx = abs(x2 - x1);
	ty = abs(y2 - y1);
	tmin = min(tx, ty);
	dist = tx + ty - (tmin >> 1);
}

inline int SPRITEp_SIZE_X(const spritetypebase* sp)
{
	return MulScale(tileWidth(sp->picnum), sp->xrepeat, 6);
}

inline int SPRITEp_SIZE_Y(const spritetypebase* sp)
{
	return MulScale(tileHeight(sp->picnum), sp->yrepeat, 6);
}

inline int SPRITEp_SIZE_Z(const spritetypebase* sp)
{
	return (tileHeight(sp->picnum) * sp->yrepeat) << 2;
}



// Given a z height and sprite return the correct y repeat value
inline int SPRITEp_SIZE_Z_2_YREPEAT(const spritetype* sp, int zh)
{
	return zh / (4 * tileHeight(sp->picnum));
}


// Z size of top (TOS) and bottom (BOS) part of sprite
inline int SPRITEp_SIZE_TOS(const spritetype* sp)
{
    return (DIV2(SPRITEp_SIZE_Z(sp)) + (tileTopOffset(sp->picnum) << 8));
}

inline int SPRITEp_SIZE_TOS(const tspritetype* sp)
{
    return (DIV2(SPRITEp_SIZE_Z(sp)) + (tileTopOffset(sp->picnum) << 8));
}

inline int SPRITEp_SIZE_BOS(const spritetype* sp)
{
    return (DIV2(SPRITEp_SIZE_Z(sp)) - (tileTopOffset(sp->picnum) << 8));
}

inline int SPRITEp_SIZE_BOS(const tspritetype* sp)
{
    return (DIV2(SPRITEp_SIZE_Z(sp)) - (tileTopOffset(sp->picnum) << 8));
}

// actual Z for TOS and BOS - handles both WYSIWYG and old style
#define SPRITEp_TOS(sp) (TEST((sp)->cstat, CSTAT_SPRITE_YCENTER) ? \
                         ((sp)->z - SPRITEp_SIZE_TOS(sp)) :         \
                         ((sp)->z - SPRITEp_SIZE_Z(sp)))

#define SPRITEp_BOS(sp) (TEST((sp)->cstat, CSTAT_SPRITE_YCENTER) ? \
                         ((sp)->z + SPRITEp_SIZE_BOS(sp)) :         \
                         (sp)->z)

// mid and upper/lower sprite calculations
#define SPRITEp_MID(sp) (DIV2(SPRITEp_TOS(sp) + SPRITEp_BOS(sp)))
#define SPRITEp_UPPER(sp) (SPRITEp_TOS(sp) + (SPRITEp_SIZE_Z(sp) >> 2))
#define SPRITEp_LOWER(sp) (SPRITEp_BOS(sp) - (SPRITEp_SIZE_Z(sp) >> 2))

#define Z(value) ((int)(value) << 8)
#define PIXZ(value) ((int)(value) >> 8)

#define SQ(val) ((val) * (val))

#define KENFACING_PLAYER(pp,sp) (bcos(sp->ang)*(pp->posy-sp->y) >= bsin(sp-ang)*(pp->posx-sp->x))
#define FACING_PLAYER(pp,sp) (abs(getincangle(getangle((pp)->posx - (sp)->x, (pp)->posy - (sp)->y), (sp)->ang)) < 512)
#define PLAYER_FACING(pp,sp) (abs(getincangle(getangle((sp)->x - (pp)->posx, (sp)->y - (pp)->posy), (pp)->angle.ang.asbuild())) < 320)

#define FACING_PLAYER_RANGE(pp,sp,range) (abs(getincangle(getangle((pp)->posx - (sp)->x, (pp)->posy - (sp)->y), (sp)->ang)) < (range))
#define PLAYER_FACING_RANGE(pp,sp,range) (abs(getincangle(getangle((sp)->x - (pp)->posx, (sp)->y - (pp)->posy), (pp)->angle.ang.asbuild())) < (range))
#define FACING_RANGE(sp1,sp2,range) (abs(getincangle(getangle((sp1)->x - (sp2)->x, (sp1)->y - (sp2)->y), (sp2)->ang)) < (range))

// two vectors
// can determin direction
#define DOT_PRODUCT_2D(x1,y1,x2,y2) (MulScale((x1), (x2), 16) + MulScale((y1), (y2), 16))


#define SEC(value) ((value)*120)

#define CEILING_DIST (Z(4))
#define FLOOR_DIST (Z(4))

#define NORM_SPRITE(val) ((val) & (kHitIndexMask))

#define NORM_SECTOR(val) ((val) & (kHitIndexMask))

// overwritesprite flags
#define OVER_SPRITE_MIDDLE      (BIT(0))
#define OVER_SPRITE_VIEW_CLIP   (BIT(1))
#define OVER_SPRITE_TRANSLUCENT (BIT(2))
#define OVER_SPRITE_XFLIP       (BIT(3))
#define OVER_SPRITE_YFLIP       (BIT(4))

// system defines for status bits
#define CEILING_STAT_PLAX           BIT(0)
#define CEILING_STAT_SLOPE          BIT(1)
#define CEILING_STAT_SWAPXY         BIT(2)
#define CEILING_STAT_SMOOSH         BIT(3)
#define CEILING_STAT_XFLIP          BIT(4)
#define CEILING_STAT_YFLIP          BIT(5)
#define CEILING_STAT_RELATIVE       BIT(6)
#define CEILING_STAT_TYPE_MASK     (BIT(7)|BIT(8))
#define CEILING_STAT_MASKED         BIT(7)
#define CEILING_STAT_TRANS          BIT(8)
#define CEILING_STAT_TRANS_FLIP     (BIT(7)|BIT(8))
#define CEILING_STAT_FAF_BLOCK_HITSCAN      BIT(15)

#define FLOOR_STAT_PLAX           BIT(0)
#define FLOOR_STAT_SLOPE          BIT(1)
#define FLOOR_STAT_SWAPXY         BIT(2)
#define FLOOR_STAT_SMOOSH         BIT(3)
#define FLOOR_STAT_XFLIP          BIT(4)
#define FLOOR_STAT_YFLIP          BIT(5)
#define FLOOR_STAT_RELATIVE       BIT(6)
#define FLOOR_STAT_TYPE_MASK     (BIT(7)|BIT(8))
#define FLOOR_STAT_MASKED         BIT(7)
#define FLOOR_STAT_TRANS          BIT(8)
#define FLOOR_STAT_TRANS_FLIP     (BIT(7)|BIT(8))
#define FLOOR_STAT_FAF_BLOCK_HITSCAN      BIT(15)

#undef CLIPMASK0 // defined in build.h
#undef CLIPMASK1

// new define more readable defines

// Clip Sprite adjustment
#define CS(sprite_bit) IntToFixed(sprite_bit)

// for players to clip against walls
#define CLIPMASK_PLAYER (CS(CSTAT_SPRITE_BLOCK) | CSTAT_WALL_BLOCK)

// for actors to clip against walls
#define CLIPMASK_ACTOR                   \
    (                                    \
        CS(CSTAT_SPRITE_BLOCK) |          \
        CSTAT_WALL_BLOCK |                   \
        CSTAT_WALL_BLOCK_ACTOR               \
    )

// for missiles to clip against actors
#define CLIPMASK_MISSILE                                            \
    (                                                               \
        CS(CSTAT_SPRITE_BLOCK_HITSCAN|CSTAT_SPRITE_BLOCK_MISSILE) |     \
        CSTAT_WALL_BLOCK_HITSCAN                                        \
    )

#define CLIPMASK_WARP_HITSCAN            \
    (                                    \
        CS(CSTAT_SPRITE_BLOCK_HITSCAN) |     \
        CSTAT_WALL_BLOCK_HITSCAN |           \
        CSTAT_WALL_WARP_HITSCAN              \
    )


#define SIZ countof


//
// Directions
//

enum
{
    DEGREE_45 = 256,
    DEGREE_90 = 512
};

////
//
// Directional enumerations
//
////

enum DirOrd
{
    ORD_NORTH, ORD_NE, ORD_EAST, ORD_SE, ORD_SOUTH, ORD_SW, ORD_WEST, ORD_NW
};

enum Dir8
{
    NORTH   = ORD_NORTH * DEGREE_45,
    NE      = ORD_NE    * DEGREE_45,
    EAST    = ORD_EAST  * DEGREE_45,
    SE      = ORD_SE    * DEGREE_45,
    SOUTH   = ORD_SOUTH * DEGREE_45,
    SW      = ORD_SW    * DEGREE_45,
    WEST    = ORD_WEST  * DEGREE_45,
    NW      = ORD_NW    * DEGREE_45,
};

typedef enum Dir8 DIR8;

// Auto building enumerations

#define DIGI_ENUM
enum digi
{
#include "digi.h"
    DIGI_MAX
};
#undef DIGI_ENUM

#define DAMAGE_ENUM
enum dam
{
#include "damage.h"
};
#undef DAMAGE_ENUM

////
//
// State declarations
//
////


// Forward declarations
struct STATEstruct;
typedef struct STATEstruct STATE, *STATEp, * *STATEpp;

//struct PIC_STATEstruct;
//typedef struct PIC_STATEstruct PIC_STATE, *PIC_STATEp;

struct PANEL_STATEstruct;
typedef struct PANEL_STATEstruct PANEL_STATE, *PANEL_STATEp;

struct PLAYERstruct;
typedef struct PLAYERstruct PLAYER, *PLAYERp;

struct PERSONALITYstruct;
typedef struct PERSONALITYstruct PERSONALITY, *PERSONALITYp;

struct ATTRIBUTEstruct;
typedef struct ATTRIBUTEstruct ATTRIBUTE, *ATTRIBUTEp;

struct SECTOR_OBJECTstruct;
typedef struct SECTOR_OBJECTstruct SECTOR_OBJECT, *SECTOR_OBJECTp;

struct PANEL_SPRITEstruct;
typedef struct PANEL_SPRITEstruct PANEL_SPRITE, *PANEL_SPRITEp;

struct ANIMstruct;
typedef struct ANIMstruct ANIM, *ANIMp;

class DSWActor;
typedef int ANIMATOR (DSWActor* actor);
typedef ANIMATOR *ANIMATORp;

typedef void pANIMATOR (PANEL_SPRITEp);
typedef pANIMATOR *pANIMATORp;

typedef void soANIMATOR (SECTOR_OBJECTp);
typedef soANIMATOR *soANIMATORp;

typedef spritetype SPRITE, *SPRITEp;
typedef sectortype SECTOR, *SECTORp;
typedef walltype WALL, *WALLp;

struct STATEstruct
{
    short     Pic;
    int       Tics;
    ANIMATORp Animator;

    STATEp   NextState;
};

//
// State Flags
//

#define SF_TICS_MASK 0xFFFF
#define SF_QUICK_CALL BIT(16)
#define SF_PLAYER_FUNC BIT(17) // only for players to execute
#define SF_TIC_ADJUST BIT(18) // use tic adjustment for these frames
#define SF_WALL_STATE BIT(19) // use for walls instead of sprite

///////////////////////////////////////////////////////////////////////////////
// Jim's MISC declarations from other files
///////////////////////////////////////////////////////////////////////////////

typedef enum {WATER_FOOT, BLOOD_FOOT} FOOT_TYPE;

extern FOOT_TYPE FootMode;
ANIMATOR QueueFloorBlood;                // Weapon.c
int QueueFootPrint(DSWActor*);                 // Weapon.c
void QueueLoWangs(DSWActor*);                   // Weapon.c
int SpawnShell(DSWActor* actor, int ShellNum);     // JWeapon.c
void UnlockKeyLock(short key_num, DSWActor* actor);  // JSector.c

#define MAX_PAIN 5
extern int PlayerPainVocs[MAX_PAIN];
extern int PlayerLowHealthPainVocs[MAX_PAIN];

#define MAX_TAUNTAI 33
extern int TauntAIVocs[MAX_TAUNTAI];

#define MAX_GETSOUNDS 5
extern int PlayerGetItemVocs[MAX_GETSOUNDS];

#define MAX_YELLSOUNDS 3
extern int PlayerYellVocs[MAX_YELLSOUNDS];

void BossHealthMeter(void);

// Global variables used for modifying variouse things from the Console

///////////////////////////////////////////////////////////////////////////////////////////
//
// JPlayer
//
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//
// Weapon
//
///////////////////////////////////////////////////////////////////////////////////////////

#define MAX_WEAPONS_KEYS 10
#define MAX_WEAPONS_EXTRA 4 // extra weapons like the two extra head attacks
#define MAX_WEAPONS (MAX_WEAPONS_KEYS + MAX_WEAPONS_EXTRA)

// weapons that not missile type sprites
#define WPN_NM_LAVA (-8)
#define WPN_NM_SECTOR_SQUISH (-9)

//#define WEAP_ENTRY(id, init_func, damage_lo, damage_hi, radius)

typedef struct
{
    void (*Init)(PLAYERp);
    int16_t damage_lo;
    int16_t damage_hi;
    unsigned int radius;
    int16_t max_ammo;
    int16_t min_ammo;
    int16_t with_weapon;
    int16_t weapon_pickup;
    int16_t ammo_pickup;
} DAMAGE_DATA, *DAMAGE_DATAp;

extern DAMAGE_DATA DamageData[];

// bit arrays that determine if a) Weapon has no ammo b) Weapon is the ammo (no weapon exists)
extern int WeaponHasNoAmmo, WeaponIsAmmo;


void InitWeaponFist(PLAYERp);
void InitWeaponStar(PLAYERp);
void InitWeaponShotgun(PLAYERp);
void InitWeaponRocket(PLAYERp);
void InitWeaponRail(PLAYERp);
void InitWeaponMicro(PLAYERp);
void InitWeaponUzi(PLAYERp);
void InitWeaponSword(PLAYERp);
void InitWeaponHothead(PLAYERp);
void InitWeaponElectro(PLAYERp);
void InitWeaponHeart(PLAYERp);
void InitWeaponGrenade(PLAYERp);
void InitWeaponMine(PLAYERp);

void InitWeaponNapalm(PLAYERp);
void InitWeaponRing(PLAYERp);

extern void (*InitWeapon[MAX_WEAPONS]) (PLAYERp);

///////////////////////////////////////////////////////////////////////////////////////////
//
// Player
//
///////////////////////////////////////////////////////////////////////////////////////////

#define MAX_SW_PLAYERS_SW  (4)
#define MAX_SW_PLAYERS_REG (8)
#define MAX_SW_PLAYERS (SW_SHAREWARE ? MAX_SW_PLAYERS_SW : MAX_SW_PLAYERS_REG)

extern int   ThemeTrack[6];                                          // w
extern FString ThemeSongs[6];                                          //

#define MAX_EPISODE_NAME_LEN 24
extern char EpisodeNames[3][MAX_EPISODE_NAME_LEN+2];

enum
{
    MAX_KEYS = 8,
    MAX_FORTUNES = 16,
    MAX_INVENTORY_Q = 11,//InvDecl_TOTAL

    QUOTE_KEYMSG = 1,
    QUOTE_DOORMSG = QUOTE_KEYMSG + MAX_KEYS,
    // 23+24 are reserved.
    QUOTE_COOKIE = 25,  
    QUOTE_INVENTORY = QUOTE_COOKIE + MAX_FORTUNES,
    QUOTE_WPNFIST = QUOTE_INVENTORY + MAX_INVENTORY_Q,
    QUOTE_WPNSWORD,
    QUOTE_WPNSHURIKEN,
    QUOTE_WPNSTICKY,
    QUOTE_WPNUZI,
    QUOTE_WPNLAUNCH,
    QUOTE_WPNNUKE,
    QUOTE_WPNGRENADE,
    QUOTE_WPNRAILGUN,
    QUOTE_WPNRIOT,
    QUOTE_WPNHEAD,
    QUOTE_WPNRIPPER,
    // Here a gap of two needs to be inserted because the weapon array contains two bogus entries the parser can access.
    // Not all ammo types here are used, but the entries must be reserved for the parser.
    QUOTE_AMMOFIST = QUOTE_WPNRIPPER + 2,
    QUOTE_AMMOSWORD,
    QUOTE_AMMOSHURIKEN,
    QUOTE_AMMOSTICKY,
    QUOTE_AMMOUZI,
    QUOTE_AMMOLAUNCH,
    QUOTE_AMMONUKE,
    QUOTE_AMMOGRENADE,
    QUOTE_AMMORAILGUN,
    QUOTE_AMMORIOT,
    QUOTE_AMMOHEAD,
    QUOTE_AMMORIPPER,
    // Again, here a gap of two needs to be inserted because the weapon array contains two bogus entries the parser can access.

};

#define PACK 1

extern bool CameraTestMode;

enum PlayerDeathTypes
{
    PLAYER_DEATH_FLIP, PLAYER_DEATH_CRUMBLE, PLAYER_DEATH_EXPLODE, PLAYER_DEATH_RIPPER, PLAYER_DEATH_SQUISH, PLAYER_DEATH_DROWN, MAX_PLAYER_DEATHS
};

typedef void (*PLAYER_ACTION_FUNCp)(PLAYERp);

#include "inv.h"

typedef struct
{
    sectortype* cursectp, * lastcursectp;
    int pang;
    int xvect,yvect,oxvect,oyvect,slide_xvect,slide_yvect;
    int posx,posy,posz;
    SECTOR_OBJECTp sop_control;
} REMOTE_CONTROL, *REMOTE_CONTROLp;

struct PLAYERstruct
{
    // variable that fit in the sprite or user structure
    union
    {
        struct { int32_t posx, posy, posz; };
        vec3_t pos;
    };
    
    TObjPtr<DSWActor*> actor;
    TObjPtr<DSWActor*> lowActor, highActor;
    TObjPtr<DSWActor*> remoteActor;
    TObjPtr<DSWActor*> PlayerUnderActor;
    TObjPtr<DSWActor*> KillerActor;  //who killed me
    TObjPtr<DSWActor*> HitBy;                    // Sprite num of whatever player was last hit by
    TObjPtr<DSWActor*> last_camera_act;

    DSWActor* Actor()
    {
        return actor;
    }

    // interpolation
    int oposx, oposy, oposz;

    // holds last valid move position
    int lv_x,lv_y,lv_z;

    REMOTE_CONTROL remote;
    SECTOR_OBJECTp sop_remote;
    SECTOR_OBJECTp sop;  // will either be sop_remote or sop_control

    int jump_count, jump_speed;     // jumping
    int16_t down_speed, up_speed; // diving
    int z_speed,oz_speed; // used for diving and flying instead of down_speed, up_speed
    int climb_ndx;
    int hiz,loz;
    int ceiling_dist,floor_dist;
    SECTORp hi_sectp, lo_sectp;

    int circle_camera_dist;
    int six,siy,siz; // save player interp position for PlayerSprite
    int16_t siang;

    int xvect, yvect;
    int oxvect, oyvect;
    int friction;
    int slide_xvect, slide_yvect;
    int16_t slide_ang;
    int slide_dec;
    float drive_avel;

    int16_t view_outside_dang;  // outside view delta ang
    int16_t circle_camera_ang;
    int16_t camera_check_time_delay;

    
    sectortype
        * cursector,
        * lastcursector,
        * lv_sector;

    void setcursector(sectortype* s) { cursector = s; }
    bool insector() const { return cursector != nullptr; }
    fixed_t turn180_target; // 180 degree turn

    // variables that do not fit into sprite structure
    int hvel,tilt,tilt_dest;
    PlayerHorizon horizon;
    PlayerAngle angle;
    int16_t recoil_amt;
    int16_t recoil_speed;
    int16_t recoil_ndx;
    fixed_t recoil_ohorizoff, recoil_horizoff;

    int oldposx,oldposy,oldposz;
    int RevolveX, RevolveY;
    int16_t RevolveDeltaAng;
    binangle RevolveAng;

    int16_t pnum; // carry along the player number

    sectortype* LadderSector;
    int lx,ly; // ladder x and y
    int16_t JumpDuration;
    int16_t WadeDepth;
    int16_t bob_amt;
    int16_t bob_ndx;
    int16_t bcnt; // bob count
    int bob_z, obob_z;

    //Multiplayer variables
    InputPacket input;
    InputPacket lastinput;

    // must start out as 0
    int playerreadyflag;

    PLAYER_ACTION_FUNCp DoPlayerAction;
    int Flags, Flags2;
    ESyncBits KeyPressBits;

    SECTOR_OBJECTp sop_control; // sector object pointer
    SECTOR_OBJECTp sop_riding; // sector object pointer

    struct
    {
        PANEL_SPRITEp Next, Prev;
    } PanelSpriteList;

    // hack stuff to get a working pointer to this list element without running into type punning warnings with GCC.
    // The list uses itself as sentinel element despite the type mismatch.
    PANEL_SPRITEp GetPanelSpriteList()
    {
        void* p = &PanelSpriteList;
        return reinterpret_cast<PANEL_SPRITEp>(p);
    }

    // Key stuff
    uint8_t HasKey[8];

    // Weapon stuff
    int16_t SwordAng;
    int WpnGotOnceFlags; // for no respawn mode where weapons are allowed grabbed only once
    int WpnFlags;
    int16_t WpnAmmo[MAX_WEAPONS];
    int16_t WpnNum;
    PANEL_SPRITEp CurWpn;
    PANEL_SPRITEp Wpn[MAX_WEAPONS];
    PANEL_SPRITEp Chops;
    uint8_t WpnRocketType; // rocket type
    uint8_t WpnRocketHeat; // 5 to 0 range
    uint8_t WpnRocketNuke; // 1, you have it, or you don't
    uint8_t WpnFlameType; // Guardian weapons fire
    uint8_t WpnFirstType; // First weapon type - Sword/Shuriken
    uint8_t WeaponType; // for weapons with secondary functions
    int16_t FirePause; // for sector objects - limits rapid firing
    //
    // Inventory Vars
    //
    int16_t InventoryNum;
    int16_t InventoryBarTics;
    int16_t InventoryTics[MAX_INVENTORY];
    int16_t InventoryPercent[MAX_INVENTORY];
    int8_t InventoryAmount[MAX_INVENTORY];
    bool InventoryActive[MAX_INVENTORY];

    int16_t DiveTics;
    int16_t DiveDamageTics;

    // Death stuff
    uint16_t DeathType;
    int16_t Kills;
    int16_t KilledPlayer[MAX_SW_PLAYERS_REG];
    int16_t SecretsFound;

    // Health
    int16_t Armor;
    int16_t MaxHealth;

    char PlayerName[32];

    uint8_t UziShellLeftAlt;
    uint8_t UziShellRightAlt;
    uint8_t TeamColor;  // used in team play and also used in regular mulit-play for show

    // palette fading up and down for player hit and get items
    int16_t FadeTics;                 // Tics between each fade cycle
    int16_t FadeAmt;                  // Current intensity of fade
    bool NightVision;               // Is player's night vision active?
    uint8_t StartColor;       // Darkest color in color range being used
    //short electro[64];
    bool IsAI;                      // Is this and AI character?
    int16_t fta,ftq;                  // First time active and first time quote, for talking in multiplayer games
    int16_t NumFootPrints;            // Number of foot prints left to lay down
    uint8_t WpnUziType;                // Toggle between single or double uzi's if you own 2.
    uint8_t WpnShotgunType;            // Shotgun has normal or fully automatic fire
    uint8_t WpnShotgunAuto;            // 50-0 automatic shotgun rounds
    uint8_t WpnShotgunLastShell;       // Number of last shell fired
    uint8_t WpnRailType;               // Normal Rail Gun or EMP Burst Mode
    bool Bloody;                    // Is player gooey from the slaughter?
    bool InitingNuke;
    bool TestNukeInit;
    bool NukeInitialized;           // Nuke already has counted down
    int16_t FistAng;                  // KungFu attack angle
    uint8_t WpnKungFuMove;             // KungFu special moves
    int16_t Reverb;                   // Player's current reverb setting
    int16_t Heads;                    // Number of Accursed Heads orbiting player
    int PlayerVersion;

    char cookieQuote[256];          // Should be an FString but must be POD for now so that PLAYER remains POD.
    int cookieTime;

    uint8_t WpnReloadState;
};

extern PLAYER Player[MAX_SW_PLAYERS_REG+1];


//
// Player Flags
//

enum
{
    PF_DEAD                     = (BIT(1)),
    PF_JUMPING                  = (BIT(2)),
    PF_FALLING                  = (BIT(3)),
    PF_LOCK_CRAWL               = (BIT(4)),
    PF_PLAYER_MOVED             = (BIT(7)),
    PF_PLAYER_RIDING            = (BIT(8)),
    PF_RECOIL                   = (BIT(10)),
    PF_FLYING                   = (BIT(11)),
    PF_WEAPON_RETRACT           = (BIT(12)),
    PF_PICKED_UP_AN_UZI         = (BIT(13)),
    PF_CRAWLING                 = (BIT(14)),
    PF_CLIMBING                 = (BIT(15)),
    PF_SWIMMING                 = (BIT(16)),
    PF_DIVING                   = (BIT(17)),
    PF_DIVING_IN_LAVA           = (BIT(18)),
    PF_TWO_UZI                  = (BIT(19)),
    PF_DEAD_HEAD                = (BIT(22)), // are your a dead head
    PF_HEAD_CONTROL             = (BIT(23)), // have control of turning when a head?
    PF_CLIP_CHEAT               = (BIT(24)), // cheat for wall clipping
    PF_SLIDING                  = (BIT(25)), // cheat for wall clipping
    PF_VIEW_FROM_OUTSIDE        = (BIT(26)),
    PF_VIEW_OUTSIDE_WEAPON      = (BIT(27)),
    PF_VIEW_FROM_CAMERA         = (BIT(28)),
    PF_TANK                     = (BIT(29)), // Doin the tank thang
    PF_WEAPON_DOWN              = (BIT(31)),
    PF2_TELEPORTED              = (BIT(0)),
    PF2_INPUT_CAN_AIM           = (BIT(1)), // Allow calling DoPlayerHorizon() from processMovement()
    PF2_INPUT_CAN_TURN_GENERAL  = (BIT(2)), // Allow calling DoPlayerTurn() from processMovement()
    PF2_INPUT_CAN_TURN_VEHICLE  = (BIT(3)), // Allow calling DoPlayerTurnVehicle() from processMovement()
    PF2_INPUT_CAN_TURN_TURRET   = (BIT(4)), // Allow calling DoPlayerTurnTurret() from processMovement()
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// Actor
//
///////////////////////////////////////////////////////////////////////////////////////////

//
// Hit Points
//

#define HEALTH_RIPPER            70
#define HEALTH_RIPPER2           200
#define HEALTH_MOMMA_RIPPER      500
#define HEALTH_NINJA             40
#define HEALTH_RED_NINJA         160
#define HEALTH_COOLIE            120
#define HEALTH_COOLIE_GHOST      65
#define HEALTH_SKEL_PRIEST       90
#define HEALTH_GORO              200
#define HEALTH_HORNET            4
#define HEALTH_SKULL             4
#define HEALTH_EEL               100

#define HEALTH_SERP_GOD          3800

//
// Action Set Structure
//

typedef struct
{
#define MAX_ACTOR_CLOSE_ATTACK 2
#define MAX_ACTOR_ATTACK 6
    STATEp *Stand;
    STATEp *Run;
    STATEp *Jump;
    STATEp *Fall;
    STATEp *Crawl;
    STATEp *Swim;
    STATEp *Fly;
    STATEp *Rise;
    STATEp *Sit;
    STATEp *Look;
    STATEp *Climb;
    STATEp *Pain;
    STATEp *Death1;
    STATEp *Death2;
    STATEp *Dead;
    STATEp *DeathJump;
    STATEp *DeathFall;

    STATEp *CloseAttack[MAX_ACTOR_CLOSE_ATTACK];
    int16_t  CloseAttackPercent[MAX_ACTOR_CLOSE_ATTACK];

    STATEp *Attack[MAX_ACTOR_ATTACK];
    int16_t  AttackPercent[MAX_ACTOR_ATTACK];

    STATEp *Special[2];
    STATEp *Duck;
    STATEp *Dive;
} ACTOR_ACTION_SET,*ACTOR_ACTION_SETp;

struct ROTATOR
{
    int pos;           // current position - always moves toward tgt
    int open_dest;     // destination of open position
    int tgt;           // current target
    int speed;         // speed of movement
    int orig_speed;    // original speed - vel jacks with speed
    int vel;           // velocity adjuments

    TArray<int> origX;
    TArray<int> origY;

    void SetNumWalls(int num)
    {
        origX.Resize(num);
        origY.Resize(num);
        memset(origX.Data(), 0, num * sizeof(int));
        memset(origY.Data(), 0, num * sizeof(int));
    }

    void ClearWalls()
    {
        origX.Reset();
        origY.Reset();
    }

};

using ROTATORp = ROTATOR*;

//
// User Extension record
//

struct USER
{
    // C++'s default init rules suck, so we have to help it out a bit to do what we need (i.e. setting all POD members to 0.
    USER()
    {
        memset(&WallP, 0, sizeof(USER) - myoffsetof(USER, WallP));
    }

    void Clear()
    {
        rotator.Clear();
        WallShade.Clear();
        memset(&WallP, 0, sizeof(USER) - myoffsetof(USER, WallP));
    }

    //
    // Variables that can be used by actors and Player
    //
    TPointer<ROTATOR> rotator;

    // wall vars for lighting
    TArray<int8_t> WallShade;

    WALLp WallP; // operate on wall instead of sprite
    STATEp State;
    STATEp *Rot;
    STATEp StateStart;
    STATEp StateEnd;
    STATEp *StateFallOverride; // a bit kludgy - override std fall state

    ANIMATORp ActorActionFunc;
    ACTOR_ACTION_SETp ActorActionSet;
    PERSONALITYp Personality;
    ATTRIBUTEp Attrib;
    SECTOR_OBJECTp sop_parent;  // denotes that this sprite is a part of the
    // sector object - contains info for the SO

    // referenced actors
    TObjPtr<DSWActor*> lowActor, highActor;
    TObjPtr<DSWActor*> targetActor; // target player for the enemy - can only handle one player at at time
    TObjPtr<DSWActor*> flameActor;
    TObjPtr<DSWActor*> attachActor;  // attach to sprite if needed - electro snake
    TObjPtr<DSWActor*> flagOwnerActor;
    TObjPtr<DSWActor*> WpnGoalActor;

    int Flags;
    int Flags2;
    int Tics;
    int oz; // serialized copy of sprite.oz

    int16_t RotNum;
    int16_t ID;

    // Health/Pain related
    int16_t Health;
    int16_t MaxHealth;

    int16_t LastDamage;           // last damage amount taken
    int16_t PainThreshold;       // amount of damage that can be taken before
    // going into pain frames.

    // jump & fall
    int16_t jump_speed;
    int16_t jump_grav;

    // clipmove
    int16_t ceiling_dist;
    int16_t floor_dist;
    int16_t lo_step;
    int hiz,loz;
    int zclip; // z height to move up for clipmove
    int active_range;
    SECTORp hi_sectp, lo_sectp;


    // if a player's sprite points to player structure
    PLAYERp PlayerP;
    int16_t Sibling;


    //
    // Possibly used by both.
    //

    // precalculated vectors
    int xchange,ychange,zchange;

    int  z_tgt;

    // velocity
    int  vel_tgt;
    int16_t vel_rate;
    uint8_t speed; // Ordinal Speed Range 0-3 from slow to fast

    int16_t Counter;
    int16_t Counter2;
    int16_t Counter3;
    int16_t DamageTics;
    int16_t BladeDamageTics;

    unsigned int Radius;    // for distance checking
    int  OverlapZ;  // for z overlap variable

    //
    // Only have a place for actors
    //

    

    // scaling
    int16_t scale_speed;
    unsigned short scale_value;
    int16_t scale_tgt;

    // zig zagging
    int16_t DistCheck;

    int16_t Dist;
    int16_t TargetDist;
    int16_t WaitTics;

    // track
    int16_t track;
    int16_t point;
    int16_t track_dir;
    int  track_vel;

    // sliding variables - slide backwards etc
    int16_t slide_ang;
    int  slide_vel;
    int16_t slide_dec;

    int16_t motion_blur_dist;
    int16_t motion_blur_num;

    int16_t wait_active_check;  // for enemy checking of player
    int16_t inactive_time; // length of time actor has been unaware of his tgt
    int  sx,sy,sz;
    int16_t sang;
    uint8_t spal;  // save off default palette number

    Collision coll; // same thing broken up into useful components.

    // Need to get rid of these flags
    int  Flag1;

    int8_t  LastWeaponNum;
    int8_t  WeaponNum;

    int16_t bounce;           // count bounces off wall for killing shrap stuff
    // !JIM! my extensions
    int ShellNum;          // This is shell no. 0 to whatever
    // Shell gets deleted when ShellNum < (ShellCount - MAXSHELLS)
    int16_t FlagOwner;        // Not the spritenum of the original flag (abused to hell by other things)
    int16_t Vis;              // Shading upgrade, for shooting, etc...
    bool DidAlert;          // Has actor done his alert noise before?

    int16_t oangdiff;      // Used for interpolating sprite angles

    uint8_t filler;
};

using USERp = USER*;

struct USERSAVE
{
    int16_t Health;
    int8_t WeaponNum;
    int8_t LastWeaponNum;

    void CopyFromUser(USER* u)
    {
        Health = u->Health;
        WeaponNum = u->WeaponNum;
        LastWeaponNum = u->LastWeaponNum;
    }

    void CopyToUser(USER* u)
    {
        u->Health = Health;
        u->WeaponNum = WeaponNum;
        u->LastWeaponNum = LastWeaponNum;
    }

};

enum
{
    // sprite->extra flags
    // BUILD AND GAME - DO NOT MOVE THESE
    SPRX_SKILL = (BIT(0) | BIT(1) | BIT(2)),

    // BIT(4) ST1 BUILD AND GAME
    SPRX_STAY_PUT_VATOR = (BIT(5)),    // BUILD AND GAME - will not move with vators etc
    // DO NOT MOVE THIS

    SPRX_STAG = (BIT(6)),    // BUILD AND GAME - NON-ST1 sprite with ST1 type tagging
    // DO NOT MOVE

    SPRX_QUEUE_SPRITE = (BIT(7)),    // Queue sprite -check queue when deleting
    SPRX_MULTI_ITEM = (BIT(9)),    // BUILD AND GAME - multi player item

    // have users - could be moved
    SPRX_PLAYER_OR_ENEMY = (BIT(11)),   // for checking quickly if sprite is a
    // player or actor
    // do not need Users
    SPRX_FOUND = (BIT(12)),   // BUILD ONLY INTERNAL - used for finding sprites
    SPRX_BLADE = (BIT(12)),   // blade sprite
    SPRX_BREAKABLE = (BIT(13)),   // breakable items
    SPRX_BURNABLE = (BIT(14)),   // used for burnable sprites in the game

    // temp use
    SPRX_BLOCK = (BIT(15)),   // BUILD AND GAME
};

// BUILD - tell which actors should not spawn
// GAME - used for internal game code
// ALT-M debug mode

// !LIGHT
// all three bits set - should never happen with skill
// #define SPRX_USER_NON_STANDARD  (BIT(0)|BIT(1)|BIT(2))   // used for lighting

// boolean flags carried over from build
#define SPRX_BOOL11 (BIT(5))
#define SPRX_BOOL1 (BIT(6))
#define SPRX_BOOL2 (BIT(7))
#define SPRX_BOOL3 (BIT(8))
#define SPRX_BOOL4 (BIT(9))
#define SPRX_BOOL5 (BIT(10))
#define SPRX_BOOL6 (BIT(11))
#define SPRX_BOOL7 (BIT(4))  // bit 12 was used build
#define SPRX_BOOL8 (BIT(13))
#define SPRX_BOOL9 (BIT(14))
#define SPRX_BOOL10 (BIT(15))

#define SET_BOOL1(sp) SET((sp)->extra, SPRX_BOOL1)
#define SET_BOOL2(sp) SET((sp)->extra, SPRX_BOOL2)
#define SET_BOOL3(sp) SET((sp)->extra, SPRX_BOOL3)
#define SET_BOOL4(sp) SET((sp)->extra, SPRX_BOOL4)
#define SET_BOOL5(sp) SET((sp)->extra, SPRX_BOOL5)
#define SET_BOOL6(sp) SET((sp)->extra, SPRX_BOOL6)
#define SET_BOOL7(sp) SET((sp)->extra, SPRX_BOOL7)
#define SET_BOOL8(sp) SET((sp)->extra, SPRX_BOOL8)
#define SET_BOOL9(sp) SET((sp)->extra, SPRX_BOOL9)
#define SET_BOOL10(sp) SET((sp)->extra, SPRX_BOOL10)
#define SET_BOOL11(sp) SET((sp)->extra, SPRX_BOOL11)

#define RESET_BOOL1(sp) RESET((sp)->extra, SPRX_BOOL1)
#define RESET_BOOL2(sp) RESET((sp)->extra, SPRX_BOOL2)
#define RESET_BOOL3(sp) RESET((sp)->extra, SPRX_BOOL3)
#define RESET_BOOL4(sp) RESET((sp)->extra, SPRX_BOOL4)
#define RESET_BOOL5(sp) RESET((sp)->extra, SPRX_BOOL5)
#define RESET_BOOL6(sp) RESET((sp)->extra, SPRX_BOOL6)
#define RESET_BOOL7(sp) RESET((sp)->extra, SPRX_BOOL7)
#define RESET_BOOL8(sp) RESET((sp)->extra, SPRX_BOOL8)
#define RESET_BOOL9(sp) RESET((sp)->extra, SPRX_BOOL9)
#define RESET_BOOL10(sp) RESET((sp)->extra, SPRX_BOOL10)
#define RESET_BOOL11(sp) RESET((sp)->extra, SPRX_BOOL11)

#define TEST_BOOL1(sp) TEST((sp)->extra, SPRX_BOOL1)
#define TEST_BOOL2(sp) TEST((sp)->extra, SPRX_BOOL2)
#define TEST_BOOL3(sp) TEST((sp)->extra, SPRX_BOOL3)
#define TEST_BOOL4(sp) TEST((sp)->extra, SPRX_BOOL4)
#define TEST_BOOL5(sp) TEST((sp)->extra, SPRX_BOOL5)
#define TEST_BOOL6(sp) TEST((sp)->extra, SPRX_BOOL6)
#define TEST_BOOL7(sp) TEST((sp)->extra, SPRX_BOOL7)
#define TEST_BOOL8(sp) TEST((sp)->extra, SPRX_BOOL8)
#define TEST_BOOL9(sp) TEST((sp)->extra, SPRX_BOOL9)
#define TEST_BOOL10(sp) TEST((sp)->extra, SPRX_BOOL10)
#define TEST_BOOL11(sp) TEST((sp)->extra, SPRX_BOOL11)

// User->Flags flags
enum
{
    SPR_MOVED               = BIT(0), // Did actor move
    SPR_ATTACKED            = BIT(1), // Is sprite being attacked?
    SPR_TARGETED            = BIT(2), // Is sprite a target of a weapon?
    SPR_ACTIVE              = BIT(3), // Is sprite aware of the player?
    SPR_ELECTRO_TOLERANT    = BIT(4), // Electro spell does not slow actor
    SPR_JUMPING             = BIT(5), // Actor is jumping
    SPR_FALLING             = BIT(6), // Actor is falling
    SPR_CLIMBING            = BIT(7), // Actor is falling
    SPR_DEAD               = BIT(8), // Actor is dying
    
    SPR_ZDIFF_MODE          = BIT(10), // For following tracks at different z heights
    SPR_SPEED_UP            = BIT(11), // For following tracks at different speeds
    SPR_SLOW_DOWN           = BIT(12), // For following tracks at different speeds
    SPR_DONT_UPDATE_ANG     = BIT(13), // For tracks - don't update the angle for a while
    
    SPR_SO_ATTACHED         = BIT(14), // sprite is part of a sector object
    SPR_SUICIDE             = BIT(15), // sprite is set to kill itself
    
    SPR_RUN_AWAY            = BIT(16), // sprite is in "Run Away" track mode.
    SPR_FIND_PLAYER         = BIT(17), // sprite is in "Find Player" track mode.
    
    SPR_SWIMMING            = BIT(18), // Actor is swimming
    SPR_WAIT_FOR_PLAYER     = BIT(19), // Track Mode - Actor is waiting for player to come close
    SPR_WAIT_FOR_TRIGGER    = BIT(20), // Track Mode - Actor is waiting for player to trigger
    SPR_SLIDING             = BIT(21), // Actor is sliding
    SPR_ON_SO_SECTOR        = BIT(22), // sprite is on a sector object sector
    
    SPR_SHADE_DIR           = BIT(23), // sprite is on a sector object sector
    SPR_XFLIP_TOGGLE        = BIT(24), // sprite rotation xflip bit
    SPR_NO_SCAREDZ          = BIT(25), // not afraid of falling
    
    SPR_SET_POS_DONT_KILL   = BIT(26), // Don't kill sprites in MissileSetPos
    SPR_SKIP2               = BIT(27), // 20 moves ps
    SPR_SKIP4               = BIT(28), // 10 moves ps
    
    SPR_BOUNCE              = BIT(29), // For shrapnel types that can bounce once
    SPR_UNDERWATER          = BIT(30), // For missiles etc
    
    SPR_SHADOW              = BIT(31), // Sprites that have shadows

    // User->Flags2 flags
    SPR2_BLUR_TAPER         = BIT(13)|BIT(14),   // taper type
    SPR2_BLUR_TAPER_FAST    = BIT(13),   // taper fast
    SPR2_BLUR_TAPER_SLOW    = BIT(14),   // taper slow
    SPR2_SPRITE_FAKE_BLOCK  = BIT(15),   // fake blocking bit for damage
    SPR2_NEVER_RESPAWN      = BIT(16),   // for item respawning
    SPR2_ATTACH_WALL        = BIT(17),
    SPR2_ATTACH_FLOOR       = BIT(18),
    SPR2_ATTACH_CEILING     = BIT(19),
    SPR2_CHILDREN           = BIT(20),   // sprite OWNS children
    SPR2_SO_MISSILE         = BIT(21),   // this is a missile from a SO
    SPR2_DYING              = BIT(22),   // Sprite is currently dying
    SPR2_VIS_SHADING        = BIT(23),   // Sprite shading to go along with vis adjustments
    SPR2_DONT_TARGET_OWNER  = BIT(24),
    SPR2_FLAMEDIE           = BIT(25),  // was previously 'flame == -2'
};


typedef struct
{
    int16_t high;
} RANGE,*RANGEp;


///////////////////////////////////////////////////////////////////////////////////////////
//
// Sector Stuff - Sector Objects and Tracks
//
///////////////////////////////////////////////////////////////////////////////////////////

// flags in EXTRA variable
#define SECTFX_SINK                  BIT(0)
#define SECTFX_OPERATIONAL           BIT(1)
#define SECTFX_WARP_SECTOR           BIT(2)
#define SECTFX_CURRENT               BIT(3)
#define SECTFX_Z_ADJUST              BIT(4) // adjust ceiling/floor
#define SECTFX_NO_RIDE               BIT(5) // moving sector - don't ride it
#define SECTFX_DYNAMIC_AREA          BIT(6)
#define SECTFX_DIVE_AREA             BIT(7) // Diving area
#define SECTFX_UNDERWATER            BIT(8) // Underwater area
#define SECTFX_UNDERWATER2           BIT(9) // Underwater area

#define SECTFX_LIQUID_MASK           (BIT(10)|BIT(11)) // only valid for sectors with depth
#define SECTFX_LIQUID_NONE           (0)
#define SECTFX_LIQUID_LAVA           BIT(10)
#define SECTFX_LIQUID_WATER          BIT(11)
#define SECTFX_SECTOR_OBJECT         BIT(12)  // for collision detection
#define SECTFX_VATOR                 BIT(13)  // denotes that this is a vertical moving sector
// vator type
#define SECTFX_TRIGGER               BIT(14)  // trigger type to replace tags.h trigger types

// flags in sector USER structure
#define SECTFU_SO_DONT_BOB          BIT(0)
#define SECTFU_SO_SINK_DEST         BIT(1)
#define SECTFU_SO_DONT_SINK         BIT(2)
#define SECTFU_DONT_COPY_PALETTE    BIT(3)
#define SECTFU_SO_SLOPE_FLOOR_TO_POINT BIT(4)
#define SECTFU_SO_SLOPE_CEILING_TO_POINT BIT(5)
#define SECTFU_DAMAGE_ABOVE_SECTOR  BIT(6)
#define SECTFU_VATOR_BOTH           BIT(7)  // vators set up for both ceiling and floor
#define SECTFU_CANT_SURFACE         BIT(8)  // for diving
#define SECTFU_SLIDE_SECTOR         BIT(9)  // for diving

#define MAKE_STAG_ENUM
enum stag_id
{
#include "stag.h"
};
typedef enum stag_id STAG_ID;
#undef MAKE_STAG_ENUM


#define WALLFX_LOOP_DONT_SPIN            BIT(0)
#define WALLFX_LOOP_REVERSE_SPIN         BIT(1)
#define WALLFX_LOOP_SPIN_2X              BIT(2)
#define WALLFX_LOOP_SPIN_4X              BIT(3)
#define WALLFX_LOOP_OUTER                BIT(4) // for sector object
#define WALLFX_DONT_MOVE                 BIT(5) // for sector object
#define WALLFX_SECTOR_OBJECT             BIT(6) // for collision detection
#define WALLFX_DONT_STICK                BIT(7) // for bullet holes and stars
#define WALLFX_DONT_SCALE                BIT(8) // for sector object
#define WALLFX_LOOP_OUTER_SECONDARY      BIT(9) // for sector object

enum ShrapType
{
    SHRAP_NONE              = 0,
    SHRAP_GLASS             = 1,  //
    SHRAP_TREE_BARK         = 2,  // (NEED) outside tree bark
    SHRAP_SO_SMOKE          = 3,  // only used for damaged SO's
    SHRAP_PAPER             = 4,  //
    SHRAP_BLOOD             = 5,  // std blood from gibs
    SHRAP_EXPLOSION         = 6,  // small explosion
    SHRAP_LARGE_EXPLOSION   = 7,  // large explosion
    SHRAP_METAL             = 8,  //
    SHRAP_STONE             = 9,  // what we have might be ok
    SHRAP_PLANT             = 10, // (NEED)
    SHRAP_GIBS              = 11, // std blood and guts
    SHRAP_WOOD              = 12, //
    SHRAP_GENERIC           = 13, // what we have might be ok - sort of gray brown rock look
    SHRAP_TREE_PULP         = 14, // (NEED) inside tree wood
    SHRAP_COIN              = 15,
    SHRAP_METALMIX          = 16,
    SHRAP_WOODMIX           = 17,
    SHRAP_MARBELS           = 18,
    SHRAP_PAPERMIX          = 19,
    SHRAP_USER_DEFINED      = 99
};

# define CallocMem(size, num) M_Calloc(size, num)
# define FreeMem(ptr) M_Free(ptr)

typedef struct TARGET_SORT
{
    DSWActor* actor;
    int16_t dang;
    int dist;
    int weight;
} *TARGET_SORTp;

enum { MAX_TARGET_SORT = 16 };
extern TARGET_SORT TargetSort[MAX_TARGET_SORT];
extern unsigned TargetSortCount;

enum DoorType
{
    OPERATE_TYPE,
    DOOR_HORIZ_TYPE,
    DOOR_SLIDE_TYPE,
    DOOR_SWING_TYPE,
    DOOR_ROTATE_TYPE
};

typedef enum DoorType DOOR_TYPE;

typedef struct
{
    DOOR_TYPE Type;
    int Sector;
    int16_t Speed;
    int16_t TimeOut;
} DOOR_AUTO_CLOSE, *DOOR_AUTO_CLOSEp;

#define MAX_DOOR_AUTO_CLOSE 16

typedef struct
{
    int origx[17], origy[17];
    int sector;
    int16_t angopen, angclosed, angopendir, sang, anginc;
} SWING;

typedef struct SINE_WAVE_FLOOR
{
    sectortype* sectp;
    int floor_origz, ceiling_origz, range;
    int16_t sintable_ndx, speed_shift;
    uint8_t flags;
} *SINE_WAVE_FLOORp;

#define MAX_SINE_WAVE 6
extern SINE_WAVE_FLOOR SineWaveFloor[MAX_SINE_WAVE][21];

typedef struct SINE_WALL
{
    walltype* wallp;
    int orig_xy, range;
    int16_t sintable_ndx, speed_shift, type;
} *SINE_WALLp;

#define MAX_SINE_WALL 10
#define MAX_SINE_WALL_POINTS 64
extern SINE_WALL SineWall[MAX_SINE_WALL][MAX_SINE_WALL_POINTS];

struct SPRING_BOARD
{
    sectortype* sectp;
    int TimeOut;
};

extern SPRING_BOARD SpringBoard[20];
extern SWING Rotate[17];

extern DOOR_AUTO_CLOSE DoorAutoClose[MAX_DOOR_AUTO_CLOSE];

#define MAXANIM 256
typedef void ANIM_CALLBACK (ANIMp, void *);
typedef ANIM_CALLBACK *ANIM_CALLBACKp;
typedef void *ANIM_DATAp;

enum
{
    ANIM_Floorz,
    ANIM_SopZ,
    ANIM_Spritez,
    ANIM_Userz,
    ANIM_SUdepth,
};


typedef struct TRACK_POINT
{
    int x,y,z;
    int16_t ang, tag_low, tag_high, filler;
} *TRACK_POINTp;

typedef struct TRACK
{
    TRACK_POINTp TrackPoint;
    int ttflags;
    int flags;
    int NumPoints;

    void FreeTrackPoints()
    {
        if (TrackPoint)
        {
            M_Free(TrackPoint);
            // !JIM! I added null assigner
            TrackPoint = nullptr;
        }
    }

    TRACK_POINTp SetTrackSize(unsigned newsize)
    {
        FreeTrackPoints();
        TrackPoint = (TRACK_POINTp)M_Calloc((newsize * sizeof(TRACK_POINT)), 1);
        return TrackPoint;
    }

}*TRACKp;

// Most track type flags are in tags.h

// Regular track flags
#define TF_TRACK_OCCUPIED BIT(0)

typedef struct
{
    uint8_t FromRange,ToRange,FromColor,ToColor;
} COLOR_MAP, *COLOR_MAPp;

#define MAX_TRACKS 100

extern TRACK Track[MAX_TRACKS];

struct SECTOR_OBJECTstruct
{
#define MAX_SO_SECTOR 40
#define MAX_SO_POINTS (MAX_SO_SECTOR*15)
#define MAX_SO_SPRITE 60
#define MAX_CLIPBOX 32

    soANIMATORp PreMoveAnimator;
    soANIMATORp PostMoveAnimator;
    soANIMATORp Animator;
    TObjPtr<DSWActor*> controller;

    TObjPtr<DSWActor*> sp_child;  // child sprite that holds info for the sector object

    union
    {
        struct { int xmid, ymid, zmid; };  // midpoints of the sector object
        vec3_t pmid;
    };
    
	TObjPtr<DSWActor*> so_actors[MAX_SO_SPRITE];    // hold the actors of the object
	TObjPtr<DSWActor*> match_event_actor; // spritenum of the match event sprite

    sectortype
        *sectp[MAX_SO_SECTOR],
        *scratch,           // Just a filler to account for shitty loop tests.
        *op_main_sector, // main sector operational SO moves in - for speed purposes
        *mid_sector;     // middle sector

    walltype
        * morph_wall_point;       // actual wall point to drag


    int    vel,            // velocity
           vel_tgt,        // target velocity
           player_xoff,    // player x offset from the xmid
           player_yoff,    // player y offset from the ymid
           zorig_floor[MAX_SO_SECTOR],      // original z values for all sectors
           zorig_ceiling[MAX_SO_SECTOR],      // original z values for all sectors
           zdelta,         // z delta from original
           z_tgt,          // target z delta
           z_rate,         // rate at which z aproaches target
           update,         // Distance from player at which you continue updating
    // only works for single player.
           bob_diff,       // bobbing difference for the frame
           target_dist,    // distance to next point
           floor_loz,      // floor low z
           floor_hiz,      // floor hi z
           morph_z,        // morphing point z
           morph_z_min,    // morphing point z min
           morph_z_max,
           bob_amt,        // bob amount max in z coord
    // variables set by mappers for drivables
           drive_angspeed,
           drive_angslide,
           drive_speed,
           drive_slide,
           crush_z,
           flags;

    int16_t xorig[MAX_SO_POINTS],   // save the original x & y location of each wall so it can be
            yorig[MAX_SO_POINTS],   // refreshed
            max_damage,     // max damage
            ram_damage,     // damage taken by ramming
            wait_tics,      //
            num_sectors,    // number of sectors
            num_walls,      // number of sectors
            track,          // the track # 0 to 20
            point,          // the point on the track that the sector object is headed toward
            vel_rate,       // rate at which velocity aproaches target
            dir,            // direction traveling on the track
            ang,            // angle facing
            ang_moving,     // angle the SO is facing
            clipdist,       // cliping distance for operational sector objects
            clipbox_dist[MAX_CLIPBOX], // mult-clip box variables
            clipbox_xoff[MAX_CLIPBOX], // mult-clip box variables
            clipbox_yoff[MAX_CLIPBOX], // mult-clip box variables
            clipbox_ang[MAX_CLIPBOX], // mult-clip box variables
            clipbox_vdist[MAX_CLIPBOX], // mult-clip box variables
            clipbox_num,
            ang_tgt,        // target angle
            ang_orig,       // original angle
            last_ang,       // last angle before started spinning
            old_ang,        // holding variable for the old angle
            spin_speed,     // spin_speed
            spin_ang,       // spin angle
            turn_speed,     // shift value determines how fast SO turns to match new angle
            bob_sine_ndx,   // index into sine table
            bob_speed,      // shift value for speed
            save_vel,       // save velocity
            save_spin_speed, // save spin speed
            match_event,    // match number
    // SO Scaling Vector Info
            scale_type,         // type of scaling - enum controled
            scale_active_type,  // activated by a switch or trigger

    // values for whole SO
            scale_dist,         // distance from center
            scale_speed,        // speed of scaling
            scale_dist_min,     // absolute min
            scale_dist_max,     // absolute max
            scale_rand_freq,    // freqency of direction change - based on rand(1024)

    // values for single point scaling
            scale_point_dist[MAX_SO_POINTS],         // distance from center
            scale_point_speed[MAX_SO_POINTS],        // speed of scaling
            scale_point_base_speed,                       // base speed of scaling
            scale_point_dist_min,     // absolute min
            scale_point_dist_max,     // absolute max
            scale_point_rand_freq,    // freqency of direction change - based on rand(1024)

            scale_x_mult,           // x multiplyer for scaling
            scale_y_mult,           // y multiplyer for scaling

    // Used for center point movement
            morph_ang,              // angle moving from CENTER
            morph_speed,            // speed of movement
            morph_dist_max,         // radius boundry
            morph_rand_freq,        // freq of dir change
            morph_dist,             // dist from CENTER
            morph_z_speed,          // z speed for morph point
            morph_xoff,             // save xoff from center
            morph_yoff,             // save yoff from center

    //scale_rand_reverse,            // random at random interval
    // limit rotation angle
            limit_ang_center, // for limiting the angle of turning - turrets etc
            limit_ang_delta; //

};

#define MAX_SECTOR_OBJECTS 20

#define SOBJ_SPEED_UP           BIT(0)
#define SOBJ_SLOW_DOWN          BIT(1)
#define SOBJ_ZUP                BIT(2)
#define SOBJ_ZDOWN              BIT(3)
#define SOBJ_ZDIFF_MODE         BIT(4)
#define SOBJ_MOVE_VERTICAL      BIT(5) // for sprite objects - move straight up/down
#define SOBJ_ABSOLUTE_ANGLE     BIT(7)
#define SOBJ_SPRITE_OBJ         BIT(8)
#define SOBJ_DONT_ROTATE        BIT(9)
#define SOBJ_WAIT_FOR_EVENT     BIT(10)
#define SOBJ_HAS_WEAPON         BIT(11)
#define SOBJ_SYNC1              BIT(12) // for syncing up several SO's perfectly
#define SOBJ_SYNC2              BIT(13) // for syncing up several SO's perfectly
#define SOBJ_DYNAMIC            BIT(14) // denotes scaling or morphing object
#define SOBJ_ZMID_FLOOR         BIT(15) // can't remember which sector objects need this
// think its the bobbing and sinking ones
#define SOBJ_SLIDE              BIT(16)

#define SOBJ_OPERATIONAL        BIT(17)
#define SOBJ_KILLABLE           BIT(18)
#define SOBJ_DIE_HARD           BIT(19)
#define SOBJ_UPDATE_ONCE        BIT(20)
#define SOBJ_UPDATE             BIT(21)
#define SOBJ_NO_QUAKE           BIT(22)
#define SOBJ_REMOTE_ONLY        BIT(23)
#define SOBJ_RECT_CLIP          BIT(24)
#define SOBJ_BROKEN               BIT(25)

// track set to these to tell them apart
#define SO_OPERATE_TRACK_START 90
#define SO_TURRET_MGUN 96 // machine gun
#define SO_TURRET 97
#define SO_VEHICLE 98
// #define SO_SPEED_BOAT 99

#define SO_EMPTY(sop) ((sop)->xmid == INT32_MAX)

extern SECTOR_OBJECT SectorObject[MAX_SECTOR_OBJECTS];


///////////////////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//
///////////////////////////////////////////////////////////////////////////////////////////

ANIMATOR NullAnimator;

int Distance(int x1, int y1, int x2, int y2);

int NewStateGroup(DSWActor* actor, STATEp SpriteGroup[]);
void SectorMidPoint(sectortype* sect, int *xmid, int *ymid, int *zmid);
USERp SpawnUser(DSWActor* actor, short id, STATEp state);

short ActorFindTrack(DSWActor* actor, int8_t player_dir, int track_type, int *track_point_num, int *track_dir);

// Some sounds were checked by storing handles in static local variables.
// Problems with this design:
// 1. The variables were unmaintained and could refer to handles that had been reused already.
// 2. No proper sound ownership tracking.
// 3. In some cases items that were supposed to use the same check referred to different handle variables.
// In short: It was very broken. This is a list of all sound items used this way, now each one gets a dedicated channel
// so that proper checks can be performed and sound ownership be tracked.

enum
{
    CHAN_ToiletFart = 1000,
    CHAN_AnimeMad = 1001,
    CHAN_AnimeSing = 1002,
    CHAN_CoyHandle = 1003,
    CHAN_RipHeart = 1004,
};

short SoundDist(int x, int y, int z, int basedist);
short SoundAngle(int x, int  y);
//void PlaySound(int num, short angle, short vol);
int _PlaySound(int num, DSWActor* sprite, PLAYERp player, vec3_t *pos, Voc3D_Flags flags, int channel, EChanFlags sndflags);
void InitAmbient(int num, DSWActor* actor);

inline void PlaySound(int num, PLAYERp player, Voc3D_Flags flags, int channel = 8, EChanFlags sndflags = CHANF_NONE)
{
    _PlaySound(num, nullptr, player, nullptr, flags, channel, sndflags);
}
inline void PlaySound(int num, Voc3D_Flags flags, int channel = 8, EChanFlags sndflags = CHANF_NONE)
{
    _PlaySound(num, nullptr, nullptr, nullptr, flags, channel, sndflags);
}
inline void PlaySound(int num, vec3_t *pos, Voc3D_Flags flags, int channel = 8, EChanFlags sndflags = CHANF_NONE)
{
    _PlaySound(num, nullptr, nullptr, pos, flags, channel, sndflags);
}

int _PlayerSound(int num, PLAYERp pp);
inline int PlayerSound(int num, int flags, PLAYERp pp) { return _PlayerSound(num, pp); }
void StopPlayerSound(PLAYERp pp, int which = -1);
 bool SoundValidAndActive(DSWActor* spr, int channel);


ANIMATOR DoActorBeginJump,DoActorJump,DoActorBeginFall,DoActorFall,DoActorDeathMove;

struct BREAK_INFO;
int SpawnShrap(DSWActor*, DSWActor*, int = -1, BREAK_INFO* breakinfo = nullptr);

void PlayerUpdateHealth(PLAYERp pp, short value);
void PlayerUpdateAmmo(PLAYERp pp, short WeaponNum, short value);
void PlayerUpdateWeapon(PLAYERp pp, short WeaponNum);
void PlayerUpdateKills(PLAYERp pp, short value);
void RefreshInfoLine(PLAYERp pp);

void DoAnim(int numtics);
void AnimDelete(int animtype, int animindex, DSWActor*);
short AnimGetGoal(int animtype, int animindex, DSWActor*);
int AnimSet(int animtype, int animindex, DSWActor* animactor, int thegoal, int thevel);
int AnimSet(int animtype, sectortype* animindex, int thegoal, int thevel)
{
    return AnimSet(animtype, sectnum(animindex), nullptr, thegoal, thevel);
}

short AnimSetCallback(short anim_ndx, ANIM_CALLBACKp call, SECTOR_OBJECTp data);
short AnimSetVelAdj(short anim_ndx, short vel_adj);

void EnemyDefaults(DSWActor* actor, ACTOR_ACTION_SETp action, PERSONALITYp person);

void getzrangepoint(int x, int y, int z, sectortype* sect, int32_t* ceilz, Collision* ceilhit, int32_t* florz, Collision* florhit);
Collision move_sprite(DSWActor* , int xchange, int ychange, int zchange, int ceildist, int flordist, uint32_t cliptype, int numtics);
Collision move_missile(DSWActor*, int xchange, int ychange, int zchange, int ceildist, int flordist, uint32_t cliptype, int numtics);
DSWActor* DoPickTarget(DSWActor*, uint32_t max_delta_ang, int skip_targets);

void change_actor_stat(DSWActor* actor, int stat, bool quick = false);
void SetOwner(DSWActor*, DSWActor*, bool flag = true);
void SetOwner(int a, int b); // we still need this...
void ClearOwner(DSWActor* ownr);
DSWActor* GetOwner(DSWActor* child);
void SetAttach(DSWActor*, DSWActor*);
void analyzesprites(spritetype* tsprite, int& spritesortcnt, int viewx, int viewy, int viewz, int camang);
void CollectPortals();

int SpawnBlood(DSWActor* actor, DSWActor* weapActor, short hit_ang, int hit_x, int hit_y, int hit_z);

#define FAF_PLACE_MIRROR_PIC 341
#define FAF_MIRROR_PIC 2356

inline bool FAF_ConnectCeiling(sectortype* sect)
{
    return (sect && sect->ceilingpicnum == FAF_MIRROR_PIC);
}

inline bool FAF_ConnectFloor(sectortype* sect)
{
    return (sect && sect->floorpicnum == FAF_MIRROR_PIC);
}

inline bool FAF_ConnectArea(sectortype* sect)
{
    return sect && (FAF_ConnectCeiling(sect) || FAF_ConnectFloor(sect));
}

bool PlayerCeilingHit(PLAYERp pp, int zlimit);
bool PlayerFloorHit(PLAYERp pp, int zlimit);

void FAFhitscan(int32_t x, int32_t y, int32_t z, sectortype* sect,
    int32_t xvect, int32_t yvect, int32_t zvect,
    HitInfo& hit, int32_t clipmask);

bool FAFcansee(int32_t xs, int32_t ys, int32_t zs, sectortype* sects, int32_t xe, int32_t ye, int32_t ze, sectortype* secte);

void FAFgetzrange(vec3_t pos, sectortype* sect,
                  int32_t* hiz, Collision* ceilhit,
                  int32_t* loz, Collision* florhit,
                  int32_t clipdist, int32_t clipmask);

void FAFgetzrangepoint(int32_t x, int32_t y, int32_t z, sectortype* sect,
                       int32_t* hiz, Collision* ceilhit,
                       int32_t* loz, Collision* florhit);


enum SoundType
{
    SOUND_OBJECT_TYPE,
    SOUND_EVERYTHING_TYPE
};

void DoSoundSpotMatch(short match, short sound_num, short sound_type);

#define ACTOR_GRAVITY 8

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Externs
//
///////////////////////////////////////////////////////////////////////////////////////////

extern bool NewGame;
extern uint8_t CommPlayers;
extern bool CommEnabled;
extern int LastFrameTics;
extern char ds[];
extern short Skill;
extern int GodMode;

extern bool ReloadPrompt;

extern int lockspeed;

enum
{
    synctics = 3,
    ACTORMOVETICS = (synctics << 1),
    TICSPERMOVEMENT = synctics,
};

// subtract value from clipdist on getzrange calls
#define GETZRANGE_CLIP_ADJ 8

// MULTIPLAYER
// VARIABLES:  (You should extern these in your game.c)
/*
extern short numplayers, myconnectindex;
extern short connecthead, connectpoint2[MAXPLAYERS];
*/
extern int *lastpacket2clock;

// save player info when moving to a new level (shortened to only cover the fields that actually are copied back.(
extern USERSAVE puser[MAX_SW_PLAYERS_REG];

///////////////////////////
//
// TEXT PRINTING
//
///////////////////////////

#define TEXT_TEST_LINE (200/2)
#define TEXT_XCENTER(width) ((320 - width)/2)
#define TEXT_YCENTER(h) ((200 - height)/2)
#define TEXT_TEST_COL(width) TEXT_XCENTER(width)
#define TEXT_TEST_TIME 2

///////////////////////////
//
// RECENT network additions
//
///////////////////////////

extern double smoothratio;
extern int MoveSkip4, MoveSkip2, MoveSkip8;
extern int MinEnemySkill;

#define MASTER_SWITCHING 1

extern short screenpeek;

#define STAT_DAMAGE_LIST_SIZE 20
extern int16_t StatDamageList[STAT_DAMAGE_LIST_SIZE];

///////////////////////////////////////////////////////////////
//
// Stuff for player palette flashes when hurt or getting items
//
///////////////////////////////////////////////////////////////

#define COLOR_PAIN  128  // Light red range
extern void SetFadeAmt(PLAYERp pp, short damage, uint8_t startcolor);
extern void DoPaletteFlash(PLAYERp pp);
extern bool NightVision;



#define MAXSO (INT32_MAX)

///////////////////////////////////////////////////////////////
//
// Stuff added by JonoF. These should get put into their own
// headers and included by that which needs them.
//
///////////////////////////////////////////////////////////////

int PickJumpMaxSpeed(DSWActor*, short max_speed); // ripper.c
int DoRipperRipHeart(DSWActor*);  // ripper.c
int DoRipper2RipHeart(DSWActor*); // ripper2.c
DSWActor* BunnyHatch2(DSWActor*);  // bunny.c

void TerminateLevel(void);  // game.c
void DrawMenuLevelScreen(void); // game.c
void DebugWriteString(char *string);    // game.c

void getsyncstat(void); // sync.c
void SyncStatMessage(void); // sync.c

void drawscreen(PLAYERp pp, double smoothratio);    // draw.c
int COVERsetgamemode(int mode, int xdim, int ydim, int bpp);    // draw.c
void ScreenCaptureKeys(void);   // draw.c

void computergetinput(int snum,InputPacket *syn); // jplayer.c

void SetupMirrorTiles(void);    // rooms.c
bool FAF_Sector(sectortype* sect); // rooms.c
int GetZadjustment(sectortype* sect,short hitag);  // rooms.c

void InitSetup(void);   // setup.c

void LoadKVXFromScript(const char *filename); // scrip2.c
void LoadCustomInfoFromScript(const char *filename);  // scrip2.c

int PlayerInitChemBomb(PLAYERp pp); // jweapon.c
int PlayerInitFlashBomb(PLAYERp pp);    // jweapon.c
int PlayerInitCaltrops(PLAYERp pp); // jweapon.c
int InitPhosphorus(DSWActor*);    // jweapon.c
void SpawnFloorSplash(DSWActor*); // jweapon.c

int SaveGame(short save_num);   // save.c
int LoadGame(short save_num);   // save.c
int LoadGameFullHeader(short save_num, char *descr, short *level, short *skill);    // save,c
void LoadGameDescr(short save_num, char *descr);    // save.c

void SetRotatorActive(DSWActor* actor); // rotator.c

bool VatorSwitch(short match, short setting); // vator.c
void MoveSpritesWithSector(sectortype* sect,int z_amt,bool type);  // vator.c
void SetVatorActive(DSWActor*);   // vator.c

void DoSpikeMatch(short match); // spike.c
void SpikeAlign(DSWActor*);   // spike.c

short DoSectorObjectSetScale(short match);  // morph.c
short DoSOevent(short match,short state);   // morph.c
void SOBJ_AlignCeilingToPoint(SECTOR_OBJECTp sop,int x,int y,int z);    // morph.c
void SOBJ_AlignFloorToPoint(SECTOR_OBJECTp sop,int x,int y,int z);  // morph.c
void ScaleSectorObject(SECTOR_OBJECTp sop); // morph.c
void MorphTornado(SECTOR_OBJECTp sop);  // morph.c
void MorphFloor(SECTOR_OBJECTp sop);    // morph.c
void ScaleRandomPoint(SECTOR_OBJECTp sop,short k,short ang,int x,int y,int *dx,int *dy);    // morph.c

void CopySectorMatch(int match);  // copysect.c

int DoWallMoveMatch(short match);   // wallmove.c
int DoWallMove(DSWActor* sp); // wallmove.c
bool CanSeeWallMove(SPRITEp wp,short match);    // wallmove.c

void DoSpikeOperate(sectortype* sect); // spike.c
void SetSpikeActive(DSWActor*);   // spike.c

#define NTAG_SEARCH_LO 1
#define NTAG_SEARCH_HI 2
#define NTAG_SEARCH_LO_HI 3

DSWActor* insertActor(sectortype* sect, int statnum);

void AudioUpdate(void); // stupid

extern short LastSaveNum;
void LoadSaveMsg(const char *msg);

void UpdateStatusBar();
int32_t registerosdcommands(void);
void SW_InitMultiPsky(void);

extern short LevelSecrets;
extern int TotalKillable;
extern int OrigCommPlayers;

extern uint8_t PlayerGravity;
extern short wait_active_check_offset;
//extern short Zombies;
extern int PlaxCeilGlobZadjust, PlaxFloorGlobZadjust;
extern bool left_foot;
extern bool bosswasseen[3];
extern DSWActor* BossSpriteNum[3];
extern int ChopTics;
extern int Bunny_Count;


#define ANIM_SERP 1
#define ANIM_SUMO 2
#define ANIM_ZILLA 3

struct GameInterface : public ::GameInterface
{
    const char* Name() override { return "ShadowWarrior"; }
    void app_init() override;
    void LoadGameTextures();
    void loadPalette();
    void clearlocalinputstate() override;
    void FreeLevelData() override;
    bool GenerateSavePic() override;
	void MenuSound(EMenuSounds snd) override;
	bool CanSave() override;
	bool StartGame(FNewGameStartup& gs) override;
	FSavegameInfo GetSaveSig() override;
    void SerializeGameState(FSerializer& arc);
    void SetAmbience(bool on) override { if (on) StartAmbientSound(); else StopAmbientSound(); }
    FString GetCoordString() override;
    ReservedSpace GetReservedScreenSpace(int viewsize) override;
    void UpdateSounds() override;
    void ErrorCleanup() override;
    void GetInput(ControlInfo* const hidInput, double const scaleAdjust, InputPacket* input = nullptr) override;
    void DrawBackground(void) override;
    void Ticker(void) override;
    void Render() override;
    void Startup() override;
    const char *CheckCheatMode() override;
    const char* GenericCheat(int player, int cheat) override;
	void LevelCompleted(MapRecord *map, int skill) override;
	void NextLevel(MapRecord *map, int skill) override;
	void NewGame(MapRecord *map, int skill, bool) override;
    bool DrawAutomapPlayer(int mx, int my, int x, int y, int z, int a, double const smoothratio) override;
    int playerKeyMove() override { return 35; }
    void WarpToCoords(int x, int y, int z, int a, int h) override;
    void ToggleThirdPerson() override;
    void SwitchCoopView() override;
    int chaseCamX(binangle ang) override { return -ang.bcos(-3); }
    int chaseCamY(binangle ang) override { return -ang.bsin(-3); }
    int chaseCamZ(fixedhoriz horiz) override { return horiz.asq16() >> 8; }
    void processSprites(tspritetype* tsprite, int& spritesortcnt, int viewx, int viewy, int viewz, binangle viewang, double smoothRatio) override;
    void UpdateCameras(double smoothratio) override;
    void EnterPortal(spritetype* viewer, int type) override;
    void LeavePortal(spritetype* viewer, int type) override;
    int Voxelize(int sprnum);
    void ExitFromMenu() override;
    int GetCurrentSkill() override;


    GameStats getStats() override;
};


// OVER and UNDER water macros
inline bool SpriteInDiveArea(SPRITEp sp)
{
    return (TEST(sp->sector()->extra, SECTFX_DIVE_AREA) ? true : false);
}

inline bool SpriteInUnderwaterArea(SPRITEp sp)
{
    return (TEST(sp->sector()->extra, SECTFX_UNDERWATER | SECTFX_UNDERWATER2) ? true : false);
}

inline bool SectorIsDiveArea(sectortype* sect)
{
    return (TEST(sect->extra, SECTFX_DIVE_AREA) ? true : false);
}

inline bool SectorIsUnderwaterArea(sectortype* sect)
{
    if (!sect) return false;
    return (TEST(sect->extra, SECTFX_UNDERWATER | SECTFX_UNDERWATER2) ? true : false);
}

END_SW_NS

#include "swactor.h"

BEGIN_SW_NS

inline int ActorUpper(DSWActor* actor)
{
    return SPRITEp_UPPER(&actor->s());
}

inline int ActorLower(DSWActor* actor)
{
    return SPRITEp_LOWER(&actor->s());
}

inline int ActorMid(DSWActor* actor)
{
    return SPRITEp_MID(&actor->s());
}

inline int Facing(DSWActor* actor1, DSWActor* actor2)
{
    auto sp1 = &actor1->s();
    auto sp2 = &actor2->s();
    return (abs(getincangle(getangle((sp1)->x - (sp2)->x, (sp1)->y - (sp2)->y), (sp2)->ang)) < 512);
}

// just determine if the player is moving
inline bool PLAYER_MOVING(PLAYERp pp)
{
	return (pp->xvect | pp->yvect);
}

inline void PlaySound(int num, DSWActor* actor, Voc3D_Flags flags, int channel = 8, EChanFlags sndflags = CHANF_NONE)
{
    _PlaySound(num, actor, nullptr, nullptr, flags, channel, sndflags);
}

struct ANIMstruct
{
	int animtype, animindex;
	int goal;
	int vel;
	short vel_adj;
	TObjPtr<DSWActor*> animactor;
	ANIM_CALLBACKp callback;
	SECTOR_OBJECTp callbackdata;    // only gets used in one place for this so having a proper type makes serialization easier.

	int& Addr()
	{
        static int scratch;
		switch (animtype)
		{
		case ANIM_Floorz:
			return sector[animindex].floorz;
		case ANIM_SopZ:
			return SectorObject[animindex].zmid;
		case ANIM_Spritez:
            if (animactor == nullptr) return scratch;
			return animactor->s().z;
		case ANIM_Userz:
            if (animactor == nullptr) return scratch;
            return animactor->u()->sz;
		case ANIM_SUdepth:
			return sector[animindex].depth_fixed;
		default:
			return animindex;
		}
	}
};

extern ANIM Anim[MAXANIM];
extern short AnimCnt;


END_SW_NS

#endif

