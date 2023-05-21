class DukeCar : DukeActor
{
	default
	{
		pic "DUKECAR";
	}
	
	override void Initialize(DukeActor spawner)
	{
		self.extra = 1;
		self.clipdist = 32;
		self.vel.X = 292 / 16.;
		self.vel.Z = 360 / 256.;
		self.cstat = CSTAT_SPRITE_BLOCK_ALL;
		self.ChangeStat(STAT_ACTOR);
	}
	
	override void Tick()
	{
		self.pos.Z += self.vel.Z;
		self.counter++;

		if (self.counter == 4) self.PlayActorSound("WAR_AMBIENCE2");

		if (self.counter > (26 * 8))
		{
			Duke.PlaySound("RPG_EXPLODE");
			for (int j = 0; j < 32; j++) 
					self.RANDOMSCRAP();
			ud.earthquaketime = 16;
			self.Destroy();
			return;
		} 
		else if ((self.counter & 3) == 0)
			self.spawn("DukeExplosion2");
		self.DoMove(CLIPMASK0);
	}
}

class DukeHelicopter : DukeCar
{
	default
	{
		pic "HELECOPT";
	}
}
