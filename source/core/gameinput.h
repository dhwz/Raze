#pragma once

#include "m_fixed.h"
#include "gamecvars.h"
#include "gamestruct.h"
#include "gamefuncs.h"
#include "packet.h"

struct PlayerAngles
{
	// Temporary wrappers.
	DAngle& ZzHORIZON() { return pActor->spr.Angles.Pitch; }
	DAngle& ZzOLDHORIZON() { return pActor->PrevAngles.Pitch; }
	DAngle& ZzANGLE() { return pActor->spr.Angles.Yaw; }
	DAngle& ZzOLDANGLE() { return pActor->PrevAngles.Yaw; }

	friend FSerializer& Serialize(FSerializer& arc, const char* keyname, PlayerAngles& w, PlayerAngles* def);

	// Prototypes for applying input.
	void applyPitch(float const horz, ESyncBits* actions, double const scaleAdjust = 1);
	void applyYaw(float const avel, ESyncBits* actions, double const scaleAdjust = 1);

	// Prototypes for applying view.
	void doViewPitch(const DVector2& pos, DAngle const ang, bool const aimmode, bool const canslopetilt, sectortype* const cursectnum, double const scaleAdjust = 1, bool const climbing = false);

	// General methods.
	void setActor(DCoreActor* const actor) { pActor = actor; }

	// Pitch methods.
	void lockPitch() { legacyDisabledPitch = true; }
	void unlockPitch() { legacyDisabledPitch = false; }
	bool targetedPitch() { return legacyTargetPitch.Sgn(); }
	bool lockedPitch() { return targetedPitch() || legacyDisabledPitch; }
	void addPitch(DAngle const value)
	{
		if (!SyncInput())
		{
			legacyAdjustmentPitch += value;
		}
		else
		{
			ZzHORIZON() += value;
		}
	}
	void setPitch(DAngle value, bool const backup = false)
	{
		// Clamp incoming variable because sometimes the caller can exceed bounds.
		value = ClampViewPitch(value);

		if (!SyncInput() && !backup)
		{
			legacyTargetPitch = value.Sgn() ? value : minAngle;
		}
		else
		{
			ZzHORIZON() = value;
			if (backup) ZzOLDHORIZON() = ZzHORIZON();
		}
	}

	// Yaw methods.
	void lockYaw() { legacyDisabledYaw = true; }
	void unlockYaw() { legacyDisabledYaw = false; }
	bool targetedYaw() { return legacyTargetYaw.Sgn(); }
	bool lockedYaw() { return targetedYaw() || legacyDisabledYaw; }
	void addYaw(const DAngle value)
	{
		if (!SyncInput())
		{
			legacyAdjustmentYaw += value.Normalized180();
		}
		else
		{
			ZzANGLE() += value;
		}
	}

	void setYaw(const DAngle value, bool const backup = false)
	{
		if (!SyncInput() && !backup)
		{
			legacyTargetYaw = value.Sgn() ? value : minAngle;
		}
		else
		{
			ZzANGLE() = value;
			if (backup) ZzOLDANGLE() = ZzANGLE();
		}
	}

	// Miscellaneous helpers.
	double angLOOKANGHALF(double const interpfrac) { return angRENDERLOOKANG(interpfrac).Normalized180().Degrees() * (128. / 45.); }
	double angLOOKINGARC(double const interpfrac) { return fabs(angRENDERLOOKANG(interpfrac).Normalized180().Degrees() * (1024. / 1620.)); }

	// Crosshair x/y offsets based on look_ang's tangent.
	DVector2 angCROSSHAIROFFSETS(const double interpfrac)
	{
		return DVector2(159.72, 145.5 * angRENDERROTSCRN(interpfrac).Sin()) * -angRENDERLOOKANG(interpfrac).Tan() * (1. / tan(r_fov * pi::pi() / 360.));
	}

	// Weapon x/y offsets based on the above.
	DVector2 angWEAPONOFFSETS(const double interpfrac)
	{
		auto offsets = angCROSSHAIROFFSETS(interpfrac); offsets.Y = abs(offsets.Y) * 4.;
		return offsets;
	}


	// Legacy, to be removed.
	DAngle ZzHORIZOFF, ZzOHORIZOFF;
	void processLegacyHelperPitch(double const scaleAdjust)
	{
		if (targetedPitch())
		{
			auto delta = deltaangle(ZzHORIZON(), legacyTargetPitch);

			if (abs(delta).Degrees() > 0.45)
			{
				ZzHORIZON() += delta * scaleAdjust;
			}
			else
			{
				ZzHORIZON() = legacyTargetPitch;
				legacyTargetPitch = nullAngle;
			}
		}
		else if (legacyAdjustmentPitch.Sgn())
		{
			ZzHORIZON() += legacyAdjustmentPitch * scaleAdjust;
		}
	}
	void backupPitch()
	{
		ZzOLDHORIZON() = ZzHORIZON();
		ZzOHORIZOFF = ZzHORIZOFF;
	}
	void restorePitch()
	{
		ZzHORIZON() = ZzOLDHORIZON();
		ZzHORIZOFF = ZzOHORIZOFF;
	}
	DAngle horizOLDSUM() { return ZzOLDHORIZON() + ZzOHORIZOFF; }
	DAngle horizSUM() { return ZzHORIZON() + ZzHORIZOFF; }
	DAngle horizLERPSUM(double const interpfrac) { return interpolatedvalue(horizOLDSUM(), horizSUM(), interpfrac); }
	void resetAdjustmentPitch() { legacyAdjustmentPitch = nullAngle; }

	DAngle ZzLOOKANG, ZzOLDLOOKANG, ZzROTSCRNANG, ZzOLDROTSCRNANG, YawSpin;
	void processLegacyHelperYaw(double const scaleAdjust)
	{
		if (targetedYaw())
		{
			auto delta = deltaangle(ZzANGLE(), legacyTargetYaw);

			if (abs(delta) > DAngleBuildToDeg)
			{
				ZzANGLE() += delta * scaleAdjust;
			}
			else
			{
				ZzANGLE() = legacyTargetYaw;
				legacyTargetYaw = nullAngle;
			}
		}
		else if (legacyAdjustmentYaw.Sgn())
		{
			ZzANGLE() += legacyAdjustmentYaw * scaleAdjust;
		}
	}
	void backupYaw()
	{
		ZzOLDANGLE() = ZzANGLE();
		ZzOLDLOOKANG = ZzLOOKANG;
		ZzOLDROTSCRNANG = ZzROTSCRNANG;
	}
	void restoreYaw()
	{
		ZzANGLE() = ZzOLDANGLE();
		ZzLOOKANG = ZzOLDLOOKANG;
		ZzROTSCRNANG = ZzOLDROTSCRNANG;
	}
	DAngle angOLDSUM() { return ZzOLDANGLE() + ZzOLDLOOKANG; }
	DAngle angSUM() { return ZzANGLE() + ZzLOOKANG; }
	DAngle angLERPSUM(double const interpfrac) { return interpolatedvalue(angOLDSUM(), angSUM(), interpfrac); }
	DAngle angLERPANG(double const interpfrac) { return interpolatedvalue(ZzOLDANGLE(), ZzANGLE(), interpfrac); }
	DAngle angLERPLOOKANG(double const interpfrac) { return interpolatedvalue(ZzOLDLOOKANG, ZzLOOKANG, interpfrac); }
	DAngle angLERPROTSCRN(double const interpfrac) { return interpolatedvalue(ZzOLDROTSCRNANG, ZzROTSCRNANG, interpfrac); }
	DAngle angRENDERLOOKANG(double const interpfrac) { return !SyncInput() ? ZzLOOKANG : angLERPLOOKANG(interpfrac); }
	DAngle angRENDERROTSCRN(double const interpfrac) { return !SyncInput() ? ZzROTSCRNANG : angLERPROTSCRN(interpfrac); }
	void resetAdjustmentYaw() { legacyAdjustmentYaw = nullAngle; }

private:
	// Private data which should never be accessed publically.
	DCoreActor* pActor;


	// Legacy, to be removed.
	DAngle legacyTargetPitch, legacyAdjustmentPitch;
	bool legacyDisabledPitch;

	DAngle legacyTargetYaw, legacyAdjustmentYaw;
	bool legacyDisabledYaw;
};

class FSerializer;
FSerializer& Serialize(FSerializer& arc, const char* keyname, PlayerAngles& w, PlayerAngles* def);


void updateTurnHeldAmt(double const scaleAdjust);
bool isTurboTurnTime();
void resetTurnHeldAmt();
void processMovement(InputPacket* const currInput, InputPacket* const inputBuffer, ControlInfo* const hidInput, double const scaleAdjust, int const drink_amt = 0, bool const allowstrafe = true, double const turnscale = 1);
