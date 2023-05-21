//-------------------------------------------------------------------------
/*
Copyright (C) 2019 Christoph Oelckers
Copyright (C) 2020 Mitchell Richters

This is free software; you can redistribute it and/or
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

*/
//------------------------------------------------------------------------- 

#include "menu.h"
#include "gamestate.h"
#include "gameinput.h"
#include "d_net.h"

//---------------------------------------------------------------------------
//
// CVARs to control mouse input sensitivity.
//
//---------------------------------------------------------------------------

EXTERN_CVAR(Int, vr_mode)
CVAR(Float, m_pitch, 1.f, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
CVAR(Float, m_yaw, 1.f, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
CVAR(Float, m_forward, 1.f, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
CVAR(Float, m_side, 1.f, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
CVAR(Float, cl_viewtiltscale, 1.f, CVAR_GLOBALCONFIG | CVAR_ARCHIVE);
CUSTOM_CVAR(Int, cl_viewtilting, 0, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
{
	if (self < 0) self = 0;
	else if (self > 3) self = 3;
}


//---------------------------------------------------------------------------
//
// Initialised variables.
//
//---------------------------------------------------------------------------

GameInput gameInput{};


//---------------------------------------------------------------------------
//
// Input scale helper functions.
//
//---------------------------------------------------------------------------

static inline DAngle getscaledangle(const DAngle angle, const double scale, const double push)
{
	return (angle.Normalized180() * getTicrateScale(scale)) + DAngle::fromDeg(push);
}

bool scaletozero(DAngle& angle, const double scale, const double push)
{
	const auto sgn = angle.Sgn();

	if (!sgn || sgn != (angle -= getscaledangle(angle, scale, push * sgn)).Sgn())
	{
		angle = nullAngle;
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------
//
// Handle all the game-side crouch requirements.
//
//---------------------------------------------------------------------------

void processCrouchToggle(bool& toggle, ESyncBits& actions, const bool crouchable, const bool disabletoggle)
{
	if (actions & SB_CROUCH_LOCK)
	{
		toggle = !toggle && crouchable;
		actions &= ~SB_CROUCH_LOCK;
	}

	if ((actions & (SB_CROUCH|SB_JUMP)) || disabletoggle)
	{
		toggle = 0;
	}

	if (toggle)
	{
		actions |= SB_CROUCH;
	}
}


//---------------------------------------------------------------------------
//
// Player's movement function, called from game's ticker or from gi->doPlayerMovement() as required.
//
//---------------------------------------------------------------------------

void GameInput::processMovement(PlayerAngles* const plrAngles, const float scaleAdjust, const int drink_amt, const bool allowstrafe, const float turnscale)
{
	// set up variables.
	InputPacket thisInput{};
	const auto keymove = 1 << int(!!(inputBuffer.actions & SB_RUN));
	const auto hidspeed = float(getTicrateScale(YAW_TURNSPEEDS[2]) * turnscale);

	// get all input amounts.
	const auto turning = buttonMap.ButtonDown(gamefunc_Turn_Right) -
		buttonMap.ButtonDown(gamefunc_Turn_Left);

	const auto moving = buttonMap.ButtonDown(gamefunc_Move_Forward) -
		buttonMap.ButtonDown(gamefunc_Move_Backward) +
		joyAxes[JOYAXIS_Forward] * scaleAdjust;

	const auto strafing = buttonMap.ButtonDown(gamefunc_Strafe_Right) -
		buttonMap.ButtonDown(gamefunc_Strafe_Left) -
		joyAxes[JOYAXIS_Side] * scaleAdjust;

	const auto soaring = buttonMap.ButtonDown(gamefunc_Move_Up) -
		buttonMap.ButtonDown(gamefunc_Move_Down) +
		joyAxes[JOYAXIS_Up] * scaleAdjust;

	// process player yaw input.
	if (!(buttonMap.ButtonDown(gamefunc_Strafe) && allowstrafe))
	{
		const float turndir = clamp(turning + strafing * !allowstrafe, -1.f, 1.f);
		const float turnspeed = float(getTicrateScale(YAW_TURNSPEEDS[keymove]) / (!isTurboTurnTime() * 2.8 + 1));
		thisInput.avel += mouseInput.X * MOUSE_SCALE * m_yaw;
		thisInput.avel -= joyAxes[JOYAXIS_Yaw] * hidspeed * scaleAdjust;
		thisInput.avel += turndir * turnscale * turnspeed * scaleAdjust;
		if (turndir) updateTurnHeldAmt(scaleAdjust); else turnheldtime = 0;
	}
	else
	{
		thisInput.svel += mouseInput.X * MOUSE_SCALE * m_side;
		thisInput.svel -= joyAxes[JOYAXIS_Yaw] * keymove * scaleAdjust;
		thisInput.svel += turning * turnscale * keymove * scaleAdjust;
	}

	// process player pitch input.
	if (!(inputBuffer.actions & SB_AIMMODE))
	{
		thisInput.horz -= mouseInput.Y * MOUSE_SCALE * m_pitch;
		thisInput.horz -= joyAxes[JOYAXIS_Pitch] * hidspeed * scaleAdjust;
	}
	else
	{
		thisInput.fvel += mouseInput.Y * MOUSE_SCALE * m_forward;
		thisInput.fvel += joyAxes[JOYAXIS_Pitch] * keymove * scaleAdjust;
	}

	// process movement input.
	thisInput.fvel += moving * keymove;
	thisInput.svel += strafing * keymove * allowstrafe;
	thisInput.uvel += soaring; // this isn't scaled by running.

	// process RR's drunk state.
	if (isRR() && drink_amt >= 66 && drink_amt <= 87)
	{
		thisInput.svel += drink_amt & 1 ? -thisInput.fvel : thisInput.fvel;
	}

	// add collected input to game's local input accumulation packet.
	inputBuffer.fvel = clamp(inputBuffer.fvel + thisInput.fvel, -(float)keymove, (float)keymove);
	inputBuffer.svel = clamp(inputBuffer.svel + thisInput.svel, -(float)keymove, (float)keymove);
	inputBuffer.uvel = clamp(inputBuffer.uvel + thisInput.uvel, -1.00f, 1.00f);
	inputBuffer.avel = clamp(inputBuffer.avel + thisInput.avel, -179.f, 179.f);
	inputBuffer.horz = clamp(inputBuffer.horz + thisInput.horz, -179.f, 179.f);

	// directly update player angles if we can.
	if (scaleAdjust < 1)
	{
		plrAngles->CameraAngles.Yaw += DAngle::fromDeg(thisInput.avel);
		plrAngles->CameraAngles.Roll += DAngle::fromDeg(thisInput.roll);
		plrAngles->CameraAngles.Pitch += DAngle::fromDeg(thisInput.horz);
	}
}


//---------------------------------------------------------------------------
//
// Player's vehicle movement function.
//
//---------------------------------------------------------------------------

void GameInput::processVehicle(PlayerAngles* const plrAngles, const float scaleAdjust, const float baseVel, const float velScale, const bool canMove, const bool canTurn, const bool attenuate)
{
	// open up input packet for this session.
	InputPacket thisInput{};

	// mask out all actions not compatible with vehicles.
	inputBuffer.actions &= ~(SB_WEAPONMASK_BITS | SB_TURNAROUND | SB_CENTERVIEW | SB_HOLSTER | SB_JUMP | SB_CROUCH | SB_RUN | 
		SB_AIM_UP | SB_AIM_DOWN | SB_AIMMODE | SB_LOOK_UP | SB_LOOK_DOWN | SB_LOOK_LEFT | SB_LOOK_RIGHT);

	if (canMove)
	{
		const auto kbdForwards = buttonMap.ButtonDown(gamefunc_Move_Forward) || buttonMap.ButtonDown(gamefunc_Strafe);
		const auto kbdBackward = buttonMap.ButtonDown(gamefunc_Move_Backward);
		thisInput.fvel = kbdForwards - kbdBackward + joyAxes[JOYAXIS_Forward];
		inputBuffer.fvel = clamp(inputBuffer.fvel + thisInput.fvel, -1.f, 1.f);

		// This sync bit is the brake key.
		if (buttonMap.ButtonDown(gamefunc_Run)) inputBuffer.actions |= SB_CROUCH;
	}

	if (canTurn)
	{
		// Keyboard turning.
		const auto kbdLeft = buttonMap.ButtonDown(gamefunc_Turn_Left) || buttonMap.ButtonDown(gamefunc_Strafe_Left);
		const auto kbdRight = buttonMap.ButtonDown(gamefunc_Turn_Right) || buttonMap.ButtonDown(gamefunc_Strafe_Right);
		const auto kbdDir = kbdRight - kbdLeft;

		// Input device turning.
		const auto hidLeft = mouseInput.X < 0 || joyAxes[JOYAXIS_Yaw] > 0;
		const auto hidRight = mouseInput.X > 0 || joyAxes[JOYAXIS_Yaw] < 0;

		// Velocity setup.
		const auto turnVel = (!attenuate && (isTurboTurnTime() || hidLeft || hidRight)) ? (baseVel) : (baseVel * velScale);
		const auto mouseVel = abs(turnVel * mouseInput.X * m_yaw) * (45.f / 2048.f) / scaleAdjust;
		const auto maxVel = abs(turnVel * 1.5f);

		// Apply inputs.
		thisInput.avel += ((mouseVel > 1) ? sqrtf(mouseVel) : mouseVel) * Sgn(turnVel) * Sgn(mouseInput.X) * Sgn(m_yaw);
		thisInput.avel -= turnVel * joyAxes[JOYAXIS_Yaw];
		thisInput.avel += turnVel * kbdDir;
		thisInput.avel *= scaleAdjust;
		inputBuffer.avel = clamp(inputBuffer.avel + thisInput.avel, -maxVel, maxVel);
		if (kbdDir) updateTurnHeldAmt(scaleAdjust); else turnheldtime = 0;
	}
	else
	{
		turnheldtime = 0;
	}

	// directly update player angles if we can.
	if (scaleAdjust < 1)
	{
		plrAngles->CameraAngles.Yaw += DAngle::fromDeg(thisInput.avel);
		plrAngles->CameraAngles.Roll += DAngle::fromDeg(thisInput.roll);
		plrAngles->CameraAngles.Pitch += DAngle::fromDeg(thisInput.horz);
	}
}


//---------------------------------------------------------------------------
//
// Processes all the input bits.
//
//---------------------------------------------------------------------------

void GameInput::processInputBits()
{
	if (WeaponToSend != 0) inputBuffer.setNewWeapon(WeaponToSend);
	WeaponToSend = 0;
	if (buttonMap.ButtonDown(gamefunc_Dpad_Select))
	{
		// These buttons should not autorepeat. The game handlers are not really equipped for that.
		if (joyAxes[JOYAXIS_Forward] > 0 && !(dpad_lock & 1)) { dpad_lock |= 1;  inputBuffer.setNewWeapon(WeaponSel_Prev); }
		else dpad_lock &= ~1;
		if (joyAxes[JOYAXIS_Forward] < 0 && !(dpad_lock & 2)) { dpad_lock |= 2;  inputBuffer.setNewWeapon(WeaponSel_Next); }
		else dpad_lock &= ~2;
		if ((joyAxes[JOYAXIS_Side] < 0 || joyAxes[JOYAXIS_Yaw] > 0) && !(dpad_lock & 4)) { dpad_lock |= 4;  inputBuffer.actions |= SB_INVPREV; }
		else dpad_lock &= ~4;
		if ((joyAxes[JOYAXIS_Side] > 0 || joyAxes[JOYAXIS_Yaw] < 0) && !(dpad_lock & 8)) { dpad_lock |= 8;  inputBuffer.actions |= SB_INVNEXT; }
		else dpad_lock &= ~8;

		// This eats the controller input for regular use
		joyAxes[JOYAXIS_Side] = 0;
		joyAxes[JOYAXIS_Forward] = 0;
		joyAxes[JOYAXIS_Yaw] = 0;
	}
	else dpad_lock = 0;

	gi->reapplyInputBits(&inputBuffer);

	inputBuffer.actions |= ActionsToSend;
	ActionsToSend = 0;

	if (buttonMap.ButtonDown(gamefunc_Aim_Up) || (buttonMap.ButtonDown(gamefunc_Dpad_Aiming) && joyAxes[JOYAXIS_Forward] > 0)) 
	{
		inputBuffer.actions |= SB_AIM_UP;
		inputBuffer.actions &= ~SB_CENTERVIEW;
	}

	if ((buttonMap.ButtonDown(gamefunc_Aim_Down) || (buttonMap.ButtonDown(gamefunc_Dpad_Aiming) && joyAxes[JOYAXIS_Forward] < 0))) 
	{
		inputBuffer.actions |= SB_AIM_DOWN;
		inputBuffer.actions &= ~SB_CENTERVIEW;
	}

	if (buttonMap.ButtonDown(gamefunc_Dpad_Aiming))
		joyAxes[JOYAXIS_Forward] = 0;

	if (buttonMap.ButtonDown(gamefunc_Jump))
		inputBuffer.actions |= SB_JUMP;

	if (buttonMap.ButtonDown(gamefunc_Crouch))
		inputBuffer.actions |= SB_CROUCH;

	if (buttonMap.ButtonDown(gamefunc_Toggle_Crouch))
	{
		inputBuffer.actions |= SB_CROUCH_LOCK;
		buttonMap.ClearButton(gamefunc_Toggle_Crouch);
	}

	if (buttonMap.ButtonDown(gamefunc_Fire))
		inputBuffer.actions |= SB_FIRE;

	if (buttonMap.ButtonDown(gamefunc_Alt_Fire))
		inputBuffer.actions |= SB_ALTFIRE;

	if (buttonMap.ButtonDown(gamefunc_Open))
	{
		if (isBlood()) buttonMap.ClearButton(gamefunc_Open);
		inputBuffer.actions |= SB_OPEN;
	}
	if (G_CheckAutorun(buttonMap.ButtonDown(gamefunc_Run)))
		inputBuffer.actions |= SB_RUN;

	if (!in_mousemode && !buttonMap.ButtonDown(gamefunc_Mouse_Aiming)) 
		inputBuffer.actions |= SB_AIMMODE;

	if (buttonMap.ButtonDown(gamefunc_Look_Up)) 
		inputBuffer.actions |= SB_LOOK_UP;

	if (buttonMap.ButtonDown(gamefunc_Look_Down)) 
		inputBuffer.actions |= SB_LOOK_DOWN;

	if (buttonMap.ButtonDown(gamefunc_Look_Left)) 
		inputBuffer.actions |= SB_LOOK_LEFT;

	if (buttonMap.ButtonDown(gamefunc_Look_Right)) 
		inputBuffer.actions |= SB_LOOK_RIGHT;

	if (buttonMap.ButtonDown(gamefunc_Quick_Kick))
		inputBuffer.actions |= SB_QUICK_KICK;
}


//---------------------------------------------------------------------------
//
// Processes input and returns a packet if provided.
//
//---------------------------------------------------------------------------

void GameInput::getInput(const double scaleAdjust, InputPacket* packet)
{
	if (M_Active() || gamestate != GS_LEVEL)
	{
		inputBuffer = {};
		return;
	}

	I_GetAxes(joyAxes);
	processInputBits();
	gi->doPlayerMovement(!SyncInput() ? (float)scaleAdjust : 1.f);
	mouseInput.Zero();

	if (packet)
	{
		*packet = inputBuffer;
		inputBuffer = {};
	}
}


//---------------------------------------------------------------------------
//
// Adjust player's pitch by way of keyboard input.
//
//---------------------------------------------------------------------------

void PlayerAngles::doPitchInput(InputPacket* const input)
{
	// Add player's mouse/device input.
	if (input->horz)
	{
		pActor->spr.Angles.Pitch += DAngle::fromDeg(input->horz * SyncInput());
		input->actions &= ~SB_CENTERVIEW;
	}

	// Set up a myriad of bools.
	const auto aimingUp = (input->actions & SB_LOOK_UP) == SB_AIM_UP;
	const auto aimingDown = (input->actions & SB_LOOK_DOWN) == SB_AIM_DOWN;
	const auto lookingUp = (input->actions & SB_LOOK_UP) == SB_LOOK_UP;
	const auto lookingDown = (input->actions & SB_LOOK_DOWN) == SB_LOOK_DOWN;

	// Process keyboard input.
	if (const auto aiming = aimingDown - aimingUp)
	{
		pActor->spr.Angles.Pitch += DAngle::fromDeg(getTicrateScale(PITCH_AIMSPEED) * aiming);
		input->actions &= ~SB_CENTERVIEW;
	}
	if (const auto looking = lookingDown - lookingUp)
	{
		pActor->spr.Angles.Pitch += DAngle::fromDeg(getTicrateScale(PITCH_LOOKSPEED) * looking);
		input->actions |= SB_CENTERVIEW;
	}

	// Do return to centre.
	if ((input->actions & SB_CENTERVIEW) && !(lookingUp || lookingDown))
	{
		const auto pitch = abs(pActor->spr.Angles.Pitch);
		const auto scale = pitch > PITCH_CNTRSINEOFFSET ? (pitch - PITCH_CNTRSINEOFFSET).Cos() : 1.;
		if (scaletozero(pActor->spr.Angles.Pitch, PITCH_CENTERSPEED * scale))
			input->actions &= ~SB_CENTERVIEW;
	}

	// clamp before we finish, factoring in the player's view pitch offset.
	const auto maximum = GetMaxPitch() - ViewAngles.Pitch * (ViewAngles.Pitch < nullAngle);
	const auto minimum = GetMinPitch() - ViewAngles.Pitch * (ViewAngles.Pitch > nullAngle);
	pActor->spr.Angles.Pitch = clamp(pActor->spr.Angles.Pitch, maximum, minimum);
}


//---------------------------------------------------------------------------
//
// Adjust player's yaw by way of keyboard input.
//
//---------------------------------------------------------------------------

void PlayerAngles::doYawInput(InputPacket* const input)
{
	// Add player's mouse/device input.
	pActor->spr.Angles.Yaw += DAngle::fromDeg(input->avel * SyncInput());

	if (input->actions & SB_TURNAROUND)
	{
		if (YawSpin == nullAngle)
		{
			// currently not spinning, so start a spin
			YawSpin = -DAngle180;
		}
		input->actions &= ~SB_TURNAROUND;
	}

	if (YawSpin < nullAngle)
	{
		// return spin to 0
		DAngle add = DAngle::fromDeg(getTicrateScale(!(input->actions & SB_CROUCH) ? YAW_SPINSTAND : YAW_SPINCROUCH));
		YawSpin += add;
		if (YawSpin > nullAngle)
		{
			// Don't overshoot our target. With variable factor this is possible.
			add -= YawSpin;
			YawSpin = nullAngle;
		}
		pActor->spr.Angles.Yaw += add;
	}
}


//---------------------------------------------------------------------------
//
// Player's slope tilt when playing without a mouse and on a slope.
//
//---------------------------------------------------------------------------

void PlayerAngles::doViewPitch(const bool canslopetilt, const bool climbing)
{
	if (cl_slopetilting && canslopetilt)
	{
		const auto actorsect = pActor->sector();
		if (actorsect && (actorsect->floorstat & CSTAT_SECTOR_SLOPE)) // If the floor is sloped
		{
			// Get a point, 512 (64 for Blood) units ahead of player's position
			const auto rotpt = pActor->spr.pos.XY() + pActor->spr.Angles.Yaw.ToVector() * (!isBlood() ? 32 : 4);
			auto tempsect = actorsect;
			updatesector(rotpt, &tempsect);

			if (tempsect != nullptr) // If the new point is inside a valid sector...
			{
				// Get the floorz as if the new (x,y) point was still in
				// your sector, unless it's Blood.
				const double j = getflorzofslopeptr(actorsect, pActor->spr.pos.XY());
				const double k = getflorzofslopeptr(!isBlood() ? actorsect : tempsect, rotpt);

				// If extended point is in same sector as you or the slopes
				// of the sector of the extended point and your sector match
				// closely (to avoid accidently looking straight out when
				// you're at the edge of a sector line) then adjust horizon
				// accordingly
				if (actorsect == tempsect || (!isBlood() && abs(getflorzofslopeptr(tempsect, rotpt) - k) <= 4))
				{
					ViewAngles.Pitch -= maphoriz((j - k) * (!isBlood() ? 0.625 : 5.5));
				}
			}
		}
	}

	if (cl_slopetilting && climbing)
	{
		// tilt when climbing but you can't even really tell it.
		if (ViewAngles.Pitch > PITCH_HORIZOFFCLIMB)
			ViewAngles.Pitch += getscaledangle(deltaangle(ViewAngles.Pitch, PITCH_HORIZOFFCLIMB), PITCH_HORIZOFFSPEED, PITCH_HORIZOFFPUSH);
	}
	else
	{
		// Make horizoff grow towards 0 since horizoff is not modified when you're not on a slope.
		scaletozero(ViewAngles.Pitch, PITCH_HORIZOFFSPEED, PITCH_HORIZOFFPUSH);
	}

	// Clamp off against the maximum allowed pitch.
	ViewAngles.Pitch = ClampViewPitch(ViewAngles.Pitch);
}


//---------------------------------------------------------------------------
//
// Player's look left/right key angle handler.
//
//---------------------------------------------------------------------------

void PlayerAngles::doViewYaw(InputPacket* const input)
{
	// Process angle return to zeros.
	scaletozero(ViewAngles.Yaw, YAW_LOOKRETURN);
	scaletozero(ViewAngles.Roll, YAW_LOOKRETURN);

	// Process keyboard input.
	if (const auto looking = !!(input->actions & SB_LOOK_RIGHT) - !!(input->actions & SB_LOOK_LEFT))
	{
		ViewAngles.Yaw += DAngle::fromDeg(getTicrateScale(YAW_LOOKINGSPEED) * looking);
		ViewAngles.Roll += DAngle::fromDeg(getTicrateScale(YAW_ROTATESPEED) * looking);
	}
}


//---------------------------------------------------------------------------
//
// View tilting effects, mostly for Exhumed to enhance its gameplay feel.
//
//---------------------------------------------------------------------------

void PlayerAngles::doRollInput(InputPacket* const input, const DVector2& nVelVect, const double nMaxVel, const bool bUnderwater)
{
	// Allow viewtilting if we're not in a VR mode.
	if (!vr_mode)
	{
		// Scale/attenuate tilting based on player actions.
		const auto rollAmp = cl_viewtiltscale / (bUnderwater + 1);
		const auto runScale = 1. / (!(input->actions & SB_RUN) + 1);
		const auto strafeScale = !!input->svel + 1;

		if (cl_viewtilting == 1)
		{
			// Console-like yaw rolling. Adjustment == ~(90/32) for keyboard turning. Clamp is 1.5x this value.
			const auto rollAdj = DAngle::fromDeg(input->avel * ROLL_TILTAVELSCALE * rollAmp);
			const auto rollMax = DAngle::fromDeg((90. / 32. * 1.5) * cl_viewtiltscale);
			scaletozero(pActor->spr.Angles.Roll, ROLL_TILTRETURN);
			pActor->spr.Angles.Roll = clamp(pActor->spr.Angles.Roll + rollAdj, -rollMax, rollMax);
		}
		else if (cl_viewtilting == 2)
		{
			// Quake-like strafe rolling. Adjustment == (90/48) for running keyboard strafe.
			const auto rollAdj = StrafeVel * strafeScale * rollAmp;
			const auto rollMax = nMaxVel * runScale * cl_viewtiltscale;
			pActor->spr.Angles.Roll = DAngle::fromDeg(clamp(rollAdj, -rollMax, rollMax) * (1.875 / nMaxVel));
		}
		else if (cl_viewtilting == 3)
		{
			// Movement rolling from player's velocity. Adjustment == (90/48) for running keyboard strafe.
			const auto rollAdj = nVelVect.Rotated(-pActor->spr.Angles.Yaw).Y * strafeScale * rollAmp;
			const auto rollMax = nMaxVel * runScale * cl_viewtiltscale;
			pActor->spr.Angles.Roll = DAngle::fromDeg(clamp(rollAdj, -rollMax, rollMax) * (1.875 / nMaxVel));
		}
		else
		{
			// Always reset roll if we're not tilting at all.
			pActor->spr.Angles.Roll = nullAngle;
		}
	}
	else
	{
		// Add player's device input.
		pActor->spr.Angles.Roll += DAngle::fromDeg(input->roll * SyncInput());
	}
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

FSerializer& Serialize(FSerializer& arc, const char* keyname, PlayerAngles& w, PlayerAngles* def)
{
	if (arc.BeginObject(keyname))
	{
		arc("viewangles", w.ViewAngles)
			("spin", w.YawSpin)
			("actor", w.pActor)
			.EndObject();

		if (arc.isReading())
		{
			w.resetCameraAngles();
		}
	}
	return arc;
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

CCMD(slot)
{
	// The max differs between games so we have to handle this here.
	const bool isDukeShareware = (g_gameType & (GAMEFLAG_DUKE | GAMEFLAG_SHAREWARE)) == (GAMEFLAG_DUKE | GAMEFLAG_SHAREWARE);
	const int max = isExhumed() || isDukeShareware ? WeaponSel_MaxExhumed : isBlood() ? WeaponSel_MaxBlood : WeaponSel_Max;

	if (argv.argc() != 2)
	{
		Printf("slot <weaponslot>: select a weapon from the given slot (1-%d)", max);
		return;
	}

	const auto slot = atoi(argv[1]);

	if (slot >= 1 && slot <= max)
	{
		gameInput.SendWeapon(slot);
	}
}

CCMD(weapprev)
{
	gameInput.SendWeapon(WeaponSel_Prev);
}

CCMD(weapnext)
{
	gameInput.SendWeapon(WeaponSel_Next);
}

CCMD(weapalt)
{
	gameInput.SendWeapon(WeaponSel_Alt);	// Only used by SW - should also be made usable by Blood ans Duke which put multiple weapons in the same slot.
}

CCMD(useitem)
{
	const int max = isExhumed() ? 6 : isSWALL() ? 7 : isBlood() ? 4 : 5;

	if (argv.argc() != 2)
	{
		Printf("useitem <itemnum>: activates an inventory item (1-%d)", max);
		return;
	}

	const auto slot = atoi(argv[1]);

	if (slot >= 1 && slot <= max)
	{
		gameInput.SendAction(ESyncBits::FromInt(SB_ITEM_BIT_1 << (slot - 1)));
	}
}

CCMD(invprev)
{
	gameInput.SendAction(SB_INVPREV);
}

CCMD(invnext)
{
	gameInput.SendAction(SB_INVNEXT);
}

CCMD(invuse)
{
	gameInput.SendAction(SB_INVUSE);
}

CCMD(centerview)
{
	gameInput.SendAction(SB_CENTERVIEW);
}

CCMD(turnaround)
{
	gameInput.SendAction(SB_TURNAROUND);
}

CCMD(holsterweapon)
{
	gameInput.SendAction(SB_HOLSTER);
}

CCMD(warptocoords)
{
	if (netgame)
	{
		Printf("warptocoords cannot be used in multiplayer.\n");
		return;
	}
	if (argv.argc() < 4)
	{
		Printf("warptocoords [x] [y] [z] [yaw] (optional) [pitch] (optional): warps the player to the specified coordinates\n");
		return;
	}
	if (gamestate != GS_LEVEL)
	{
		Printf("warptocoords: must be in a level\n");
		return;
	}

	if (const auto pActor = gi->getConsoleActor())
	{
		pActor->spr.pos = DVector3(atof(argv[1]), atof(argv[2]), atof(argv[3]));
		if (argv.argc() > 4) pActor->spr.Angles.Yaw = DAngle::fromDeg(atof(argv[4]));
		if (argv.argc() > 5) pActor->spr.Angles.Pitch = DAngle::fromDeg(atof(argv[5]));
		pActor->backuploc();
	}
}

CCMD(third_person_view)
{
	gi->ToggleThirdPerson();
}

CCMD(coop_view)
{
	gi->SwitchCoopView();
}

CCMD(show_weapon)
{
	gi->ToggleShowWeapon();
}
