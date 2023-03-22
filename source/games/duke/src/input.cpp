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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms

EDuke enhancements integrated: 04/13/2003 - Matt Saettler

Note: EDuke source was in transition.  Changes are in-progress in the
source as it is released.

*/
//-------------------------------------------------------------------------


#include "ns.h"
#include "global.h"
#include "gamecontrol.h"
#include "v_video.h"
#include "dukeactor.h"

EXTERN_CVAR(Float, m_sensitivity_x)
EXTERN_CVAR(Float, m_yaw)

BEGIN_DUKE_NS

//---------------------------------------------------------------------------
//
// handles all HUD related input, i.e. inventory item selection and activation plus weapon selection.
//
// Note: This doesn't restrict the events to WW2GI - since the other games do 
// not define any by default there is no harm done keeping the code clean.
//
//---------------------------------------------------------------------------

void hud_input(int plnum)
{
	int i, k;
	uint8_t dainv;
	player_struct* p;

	p = &ps[plnum];
	auto pact = p->GetActor();

	i = p->aim_mode;
	p->aim_mode = !PlayerInput(plnum, SB_AIMMODE);
	if (p->aim_mode < i)
		p->sync.actions |= SB_CENTERVIEW;

	// Backup weapon here as hud_input() is the first function where any one of the weapon variables can change.
	p->backupweapon();

	// Set-up crouch bools.
	const int sectorLotag = p->insector() ? p->cursector->lotag : 0;
	const bool crouchable = sectorLotag != ST_2_UNDERWATER && (sectorLotag != ST_1_ABOVE_WATER || p->spritebridge);
	const bool disableToggle = p->jetpack_on || (!crouchable && p->on_ground) || (isRRRA() && (p->OnMotorcycle || p->OnBoat));

	if (isRR() && (p->sync.actions & SB_CROUCH)) p->sync.actions &= ~SB_JUMP;

	if (crouch_toggle && (!crouchable || disableToggle))
	{
		crouch_toggle = false;
		p->sync.actions &= ~SB_CROUCH;
	}

	if ((isRR() && p->drink_amt > 88))
		p->sync.actions |= SB_LOOK_LEFT;
	if ((isRR() && p->drink_amt > 99))
		p->sync.actions |= SB_LOOK_DOWN;

	if (isRR())
	{
		if (PlayerInput(plnum, SB_QUICK_KICK) && p->last_pissed_time == 0)
		{
			if (!isRRRA() || p->GetActor()->spr.extra > 0)
			{
				p->last_pissed_time = 4000;
				S_PlayActorSound(437, pact);
				if (p->GetActor()->spr.extra <= gs.max_player_health - gs.max_player_health / 10)
				{
					p->GetActor()->spr.extra += 2;
					p->last_extra = p->GetActor()->spr.extra;
				}
				else if (p->GetActor()->spr.extra < gs.max_player_health)
					p->GetActor()->spr.extra = gs.max_player_health;
			}
		}
	}
	else
	{
		if (PlayerInput(plnum, SB_QUICK_KICK) && p->quick_kick == 0 && (p->curr_weapon != KNEE_WEAPON || p->kickback_pic == 0))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_QUICKKICK, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				p->quick_kick = 14;
				if (!p->quick_kick_msg && plnum == screenpeek) FTA(QUOTE_MIGHTY_FOOT, p);
				p->quick_kick_msg = true;
			}
		}
	}
	if (!PlayerInput(plnum, SB_QUICK_KICK)) p->quick_kick_msg = false;

	if (!PlayerInputBits(plnum, SB_INTERFACE_BITS))
		p->interface_toggle_flag = 0;
	else if (p->interface_toggle_flag == 0)
	{
		p->interface_toggle_flag = 1;

		// Don't go on if paused or dead.
		if (paused) return;
		if (p->GetActor()->spr.extra <= 0) return;

		// Activate an inventory item. This just forwards to the other inventory bits. If the inventory selector was taken out of the playsim this could be removed.
		if (PlayerInput(plnum, SB_INVUSE) && p->newOwner == nullptr)
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_INVENTORY, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (p->inven_icon > ICON_NONE && p->inven_icon <= ICON_HEATS) PlayerSetItemUsed(plnum, p->inven_icon);
			}
		}

		if (!isRR() && PlayerUseItem(plnum, ICON_HEATS))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_USENIGHTVISION, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0 && p->heat_amount > 0)
			{
				p->heat_on = !p->heat_on;
				p->inven_icon = 5;
				S_PlayActorSound(NITEVISION_ONOFF, pact);
				FTA(106 + (!p->heat_on), p);
			}
		}

		if (PlayerUseItem(plnum, ICON_STEROIDS))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_USESTEROIDS, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (p->steroids_amount == 400)
				{
					p->steroids_amount--;
					S_PlayActorSound(DUKE_TAKEPILLS, pact);
					p->inven_icon = ICON_STEROIDS;
					FTA(12, p);
				}
			}
			return;
		}

		if (PlayerInput(plnum, SB_INVPREV) || PlayerInput(plnum, SB_INVNEXT))
		{
			p->invdisptime = 26 * 2;

			if (PlayerInput(plnum, SB_INVNEXT)) k = 1;
			else k = 0;

			dainv = p->inven_icon;

			i = 0;
		CHECKINV1:

			if (i < 9)
			{
				i++;

				switch (dainv)
				{
				case 4:
					if (p->jetpack_amount > 0 && i > 1)
						break;
					if (k) dainv = 5;
					else dainv = 3;
					goto CHECKINV1;
				case 6:
					if (p->scuba_amount > 0 && i > 1)
						break;
					if (k) dainv = 7;
					else dainv = 5;
					goto CHECKINV1;
				case 2:
					if (p->steroids_amount > 0 && i > 1)
						break;
					if (k) dainv = 3;
					else dainv = 1;
					goto CHECKINV1;
				case 3:
					if (p->holoduke_amount > 0 && i > 1)
						break;
					if (k) dainv = 4;
					else dainv = 2;
					goto CHECKINV1;
				case 0:
				case 1:
					if (p->firstaid_amount > 0 && i > 1)
						break;
					if (k) dainv = 2;
					else dainv = 7;
					goto CHECKINV1;
				case 5:
					if (p->heat_amount > 0 && i > 1)
						break;
					if (k) dainv = 6;
					else dainv = 4;
					goto CHECKINV1;
				case 7:
					if (p->boot_amount > 0 && i > 1)
						break;
					if (k) dainv = 1;
					else dainv = 6;
					goto CHECKINV1;
				}
			}
			else dainv = 0;

			// These events force us to keep the inventory selector in the playsim as opposed to the UI where it really belongs.
			if (PlayerInput(plnum, SB_INVPREV))
			{
				SetGameVarID(g_iReturnVarID, dainv, nullptr, plnum);
				OnEvent(EVENT_INVENTORYLEFT, plnum, nullptr, -1);
				dainv = GetGameVarID(g_iReturnVarID, nullptr, plnum).safeValue();
			}
			if (PlayerInput(plnum, SB_INVNEXT))
			{
				SetGameVarID(g_iReturnVarID, dainv, nullptr, plnum);
				OnEvent(EVENT_INVENTORYRIGHT, plnum, nullptr, -1);
				dainv = GetGameVarID(g_iReturnVarID, nullptr, plnum).safeValue();
			}
			p->inven_icon = dainv;
			// Someone must have really hated constant data, doing this with a switch/case (and of course also with literal numbers...)
			static const uint8_t invquotes[] = { QUOTE_MEDKIT, QUOTE_STEROIDS, QUOTE_HOLODUKE, QUOTE_JETPACK, QUOTE_NVG, QUOTE_SCUBA, QUOTE_BOOTS };
			if (dainv >= 1 && dainv < 8) FTA(invquotes[dainv - 1], p);
		}

		int weap = PlayerNewWeapon(plnum);
		if (weap > 1 && p->kickback_pic > 0)
			p->wantweaponfire = weap - 1;

		// Here we have to be extra careful that the weapons do not get mixed up, so let's keep the code for Duke and RR completely separate.
		fi.selectweapon(plnum, weap);

		if (PlayerInput(plnum, SB_HOLSTER))
		{
			if (p->curr_weapon > KNEE_WEAPON)
			{
				if (p->holster_weapon == 0 && p->weapon_pos == 0)
				{
					p->holster_weapon = 1;
					p->weapon_pos = -1;
					FTA(QUOTE_WEAPON_LOWERED, p);
				}
				else if (p->holster_weapon == 1 && p->weapon_pos == -9)
				{
					p->holster_weapon = 0;
					p->weapon_pos = 10;
					FTA(QUOTE_WEAPON_RAISED, p);
				}
			}
		}

		if (PlayerUseItem(plnum, ICON_HOLODUKE) && (isRR() || p->newOwner == nullptr))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_HOLODUKEON, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (!isRR())
				{
					if (p->holoduke_on == nullptr)
					{
						if (p->holoduke_amount > 0)
						{
							p->inven_icon = 3;

							auto pactor =
								CreateActor(p->cursector, p->GetActor()->getPosWithOffsetZ().plusZ(30), TILE_APLAYER, -64, DVector2(0, 0), p->GetActor()->spr.Angles.Yaw, 0., 0., nullptr, 10);
							pactor->temp_data[3] = pactor->temp_data[4] = 0;
							p->holoduke_on = pactor;
							pactor->spr.yint = plnum;
							pactor->spr.extra = 0;
							FTA(QUOTE_HOLODUKE_ON, p);
							S_PlayActorSound(TELEPORTER, p->holoduke_on);
						}
						else FTA(QUOTE_HOLODUKE_NOT_FOUND, p);

					}
					else
					{
						S_PlayActorSound(TELEPORTER, p->holoduke_on);
						p->holoduke_on = nullptr;
						FTA(QUOTE_HOLODUKE_OFF, p);
					}
				}
				else // In RR this means drinking whiskey.
				{
					if (p->holoduke_amount > 0 && p->GetActor()->spr.extra < gs.max_player_health)
					{
						p->holoduke_amount -= 400;
						p->GetActor()->spr.extra += 5;
						if (p->GetActor()->spr.extra > gs.max_player_health)
							p->GetActor()->spr.extra = gs.max_player_health;

						p->drink_amt += 5;
						p->inven_icon = 3;
						if (p->holoduke_amount == 0)
							checkavailinven(p);

						if (p->drink_amt < 99 && !S_CheckActorSoundPlaying(pact, 425))
							S_PlayActorSound(425, pact);
					}
				}
			}
		}

		if (isRR() && PlayerUseItem(plnum, ICON_HEATS) && p->newOwner == nullptr)
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_USENIGHTVISION, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (p->yehaa_timer == 0)
				{
					p->yehaa_timer = 126;
					S_PlayActorSound(390, pact);
					p->noise_radius = 1024;
					madenoise(plnum);
					if (p->cursector->lotag == 857)
					{
						if (p->GetActor()->spr.extra <= gs.max_player_health)
						{
							p->GetActor()->spr.extra += 10;
							if (p->GetActor()->spr.extra >= gs.max_player_health)
								p->GetActor()->spr.extra = gs.max_player_health;
						}
					}
					else
					{
						if (p->GetActor()->spr.extra + 1 <= gs.max_player_health)
						{
							p->GetActor()->spr.extra++;
						}
					}
				}
			}
		}

		if (PlayerUseItem(plnum, ICON_FIRSTAID))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_USEMEDKIT, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (p->firstaid_amount > 0 && p->GetActor()->spr.extra < gs.max_player_health)
				{
					if (!isRR())
					{
						int j = gs.max_player_health - p->GetActor()->spr.extra;

						if (p->firstaid_amount > j)
						{
							p->firstaid_amount -= j;
							p->GetActor()->spr.extra = gs.max_player_health;
							p->inven_icon = 1;
						}
						else
						{
							p->GetActor()->spr.extra += p->firstaid_amount;
							p->firstaid_amount = 0;
							checkavailinven(p);
						}
						S_PlayActorSound(DUKE_USEMEDKIT, pact);
					}
					else
					{
						int j = 10;
						if (p->firstaid_amount > j)
						{
							p->firstaid_amount -= j;
							p->GetActor()->spr.extra += j;
							if (p->GetActor()->spr.extra > gs.max_player_health)
								p->GetActor()->spr.extra = gs.max_player_health;
							p->inven_icon = 1;
						}
						else
						{
							p->GetActor()->spr.extra += p->firstaid_amount;
							p->firstaid_amount = 0;
							checkavailinven(p);
						}
						if (p->GetActor()->spr.extra > gs.max_player_health)
							p->GetActor()->spr.extra = gs.max_player_health;
						p->drink_amt += 10;
						if (p->drink_amt <= 100 && !S_CheckActorSoundPlaying(pact, DUKE_USEMEDKIT))
							S_PlayActorSound(DUKE_USEMEDKIT, pact);
					}
				}
			}
		}

		if (PlayerUseItem(plnum, ICON_JETPACK) && (isRR() || p->newOwner == nullptr))
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_USEJETPACK, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() == 0)
			{
				if (!isRR())
				{
					if (p->jetpack_amount > 0)
					{
						p->jetpack_on = !p->jetpack_on;
						if (p->jetpack_on)
						{
							p->inven_icon = 4;

							S_StopSound(DUKE_SCREAM, pact);	// this will stop the falling scream
							S_PlayActorSound(DUKE_JETPACK_ON, pact);
							FTA(QUOTE_JETPACK_ON, p);
						}
						else
						{
							p->hard_landing = 0;
							p->vel.Z = 0;
							S_PlayActorSound(DUKE_JETPACK_OFF, pact);
							S_StopSound(DUKE_JETPACK_IDLE, pact);
							S_StopSound(DUKE_JETPACK_ON, pact);
							FTA(QUOTE_JETPACK_OFF, p);
						}
					}
					else FTA(QUOTE_JETPACK_NOT_FOUND, p);
				}
				else
				{
					// eat cow pie
					if (p->jetpack_amount > 0 && p->GetActor()->spr.extra < gs.max_player_health)
					{
						if (!S_CheckActorSoundPlaying(pact, 429))
							S_PlayActorSound(429, pact);

						p->jetpack_amount -= 100;
						if (p->drink_amt > 0)
						{
							p->drink_amt -= 5;
							if (p->drink_amt < 0)
								p->drink_amt = 0;
						}

						if (p->eat < 100)
						{
							p->eat += 5;
							if (p->eat > 100)
								p->eat = 100;
						}

						p->GetActor()->spr.extra += 5;

						p->inven_icon = 4;

						if (p->GetActor()->spr.extra > gs.max_player_health)
							p->GetActor()->spr.extra = gs.max_player_health;

						if (p->jetpack_amount <= 0)
							checkavailinven(p);
					}
				}
			}
		}

		if (PlayerInput(plnum, SB_TURNAROUND) && p->Angles.YawSpin == nullAngle && p->on_crane == nullptr)
		{
			SetGameVarID(g_iReturnVarID, 0, nullptr, plnum);
			OnEvent(EVENT_TURNAROUND, plnum, nullptr, -1);
			if (GetGameVarID(g_iReturnVarID, nullptr, plnum).value() != 0)
			{
				p->sync.actions &= ~SB_TURNAROUND;
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// split out for readability
//
//---------------------------------------------------------------------------

static void doVehicleTilting(player_struct* const p, const int turndir, const float factor)
{
	if (turndir)
	{
		p->oTiltStatus = p->TiltStatus;
		p->TiltStatus += factor * turndir;
		if (abs(p->TiltStatus) > 10)
			p->TiltStatus = 10.f * turndir;
	}
}

static float getVehicleTurnVel(player_struct* p, HIDInput* const hidInput, const float factor, const float baseVel, const float velScale)
{
	float turnvel = 0;

	// Cancel out micro-movement
	if (fabs(hidInput->mouseturnx) < (m_sensitivity_x * m_yaw * backendinputscale() * 2.f)) 
		hidInput->mouseturnx = 0;

	// Yes, we need all these bools...
	const bool kbdLeft = buttonMap.ButtonDown(gamefunc_Turn_Left) || buttonMap.ButtonDown(gamefunc_Strafe_Left);
	const bool kbdRight = buttonMap.ButtonDown(gamefunc_Turn_Right) || buttonMap.ButtonDown(gamefunc_Strafe_Right);
	const bool hidLeft = hidInput->mouseturnx < 0 || hidInput->joyaxes[JOYAXIS_Yaw] > 0;
	const bool hidRight = hidInput->mouseturnx > 0 || hidInput->joyaxes[JOYAXIS_Yaw] < 0;
	const int turndir = (kbdRight || hidRight) - (kbdLeft || hidLeft);

	if (p->OnMotorcycle && (p->MotoSpeed == 0 || !p->on_ground))
	{
		resetTurnHeldAmt();
		doVehicleTilting(p, turndir, factor);
	}
	else if ((p->OnMotorcycle || p->MotoSpeed) && (turndir || p->moto_drink))
	{
		if (p->OnMotorcycle || !p->NotOnWater)
			doVehicleTilting(p, turndir, factor);

		const bool noattenuate = (isTurboTurnTime() || hidLeft || hidRight) && (!p->OnMotorcycle || p->MotoSpeed > 0);
		const auto vel = (noattenuate) ? (baseVel) : (baseVel * velScale);

		turnvel = vel * -hidInput->joyaxes[JOYAXIS_Yaw];

		if (const auto kbdDir = kbdRight - kbdLeft)
		{
			turnvel += vel * kbdDir;
			updateTurnHeldAmt(factor);
		}

		if (hidInput->mouseturnx)
			turnvel += sqrtf(abs(vel * hidInput->mouseturnx / factor) * (7.f / 20.f)) * Sgn(vel) * Sgn(hidInput->mouseturnx);
	}
	else if (p->OnMotorcycle || !p->NotOnWater)
	{
		resetTurnHeldAmt();
		p->TiltStatus -= factor * Sgn(p->TiltStatus);
	}

	if (fabs(p->TiltStatus) < factor)
		p->TiltStatus = 0;

	return turnvel * factor;
}

//---------------------------------------------------------------------------
//
// much of this was rewritten from scratch to make the logic easier to follow.
//
//---------------------------------------------------------------------------

static void processVehicleInput(player_struct *p, HIDInput* const hidInput, InputPacket* const inputBuffer, InputPacket* const currInput, const double scaleAdjust)
{
	static constexpr float VEHICLETURN = (20.f * 360.f / 2048.f);
	float baseVel, velScale;

	// mask out all actions not compatible with vehicles.
	inputBuffer->actions &= ~(SB_WEAPONMASK_BITS | SB_TURNAROUND | SB_CENTERVIEW | SB_HOLSTER | SB_JUMP | SB_CROUCH | SB_RUN | 
		SB_AIM_UP | SB_AIM_DOWN | SB_AIMMODE | SB_LOOK_UP | SB_LOOK_DOWN | SB_LOOK_LEFT | SB_LOOK_RIGHT);

	if (p->OnBoat || !p->moto_underwater)
	{
		const bool kbdForwards = buttonMap.ButtonDown(gamefunc_Move_Forward) || buttonMap.ButtonDown(gamefunc_Strafe);
		const bool kbdBackward = buttonMap.ButtonDown(gamefunc_Move_Backward);
		inputBuffer->fvel = clamp(kbdForwards - kbdBackward + hidInput->joyaxes[JOYAXIS_Forward], -1.f, 1.f);
		
		if (buttonMap.ButtonDown(gamefunc_Run))
			inputBuffer->actions |= SB_CROUCH;
	}

	if (p->OnMotorcycle)
	{
		velScale = (3.f / 10.f);
		baseVel = VEHICLETURN * Sgn(p->MotoSpeed);
	}
	else
	{
		velScale = !p->NotOnWater? 1.f : (6.f / 19.f);
		baseVel = VEHICLETURN * velScale;
	}

	inputBuffer->avel += (currInput->avel = getVehicleTurnVel(p, hidInput, (float)scaleAdjust, baseVel, velScale));
}


//---------------------------------------------------------------------------
//
// External entry point
//
//---------------------------------------------------------------------------

void GameInterface::GetInput(HIDInput* const hidInput, InputPacket* const inputBuffer, InputPacket* const currInput, const double scaleAdjust)
{
	auto const p = &ps[myconnectindex];

	if (isRRRA() && (p->OnMotorcycle || p->OnBoat))
	{
		processVehicleInput(p, hidInput, inputBuffer, currInput, scaleAdjust);
	}
	else
	{
		processMovement(hidInput, inputBuffer, currInput, scaleAdjust, p->drink_amt);
	}
}


END_DUKE_NS
