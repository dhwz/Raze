
class DukeCanWithSomething : DukeActor
{
	Default
	{
		extra 0;
		clipdist 18;
		statnum STAT_ZOMBIEACTOR;
		pic "CANWITHSOMETHING";
	}
		
	override void Initialize()
	{
		if (!self.mapSpawned)
			self.scale = (0.5, 0.5);
		self.makeitfall();
		self.cstat = CSTAT_SPRITE_BLOCK_ALL | randomXFlip();
	}


	override void Tick()
	{
		self.makeitfall();
		int j = self.ifhitbyweapon();
		if (j >= 0)
		{
			self.PlayActorSound("VENT_BUST");
			for (j = 0; j < 10; j++)
				self.RANDOMSCRAP();

			if (self.lotag) self.spawnsprite(self.lotag);
			self.Destroy();
		}
	}
}
