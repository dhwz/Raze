class DukeTurret : DukeActor
{
	const TURRETSTRENGTH = 30;

	default
	{
		pic "ORGANTIC";
		Strength TURRETSTRENGTH;
		+INTERNAL_BADGUY;
		+KILLCOUNT;
		+NOVERTICALMOVE;
		+NOHITJIBS;
		+NOSHOTGUNBLOOD;
		aimoffset 32;
	}
	
	override void PlayFTASound(int mode)
	{
		self.PlayActorSound("TURR_RECOG");
	}
	
	override void Initialize(DukeActor spawner)
	{
		self.cstat |= CSTAT_SPRITE_YCENTER;
	}

}