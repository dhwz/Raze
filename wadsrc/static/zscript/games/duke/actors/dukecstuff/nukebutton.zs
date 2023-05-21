
class DukeNukeButton : DukeActor
{
	default
	{
		spriteset "NUKEBUTTON", "NUKEBUTTON1",  "NUKEBUTTON2",  "NUKEBUTTON3";
	}
	
	override void Initialize(DukeActor spawner)
	{
		self.ChangeStat(STAT_MISC);
	}
	
	override void Tick()
	{
		if (self.counter)
		{
			self.counter++;
			let Owner = self.ownerActor;
			if (self.counter == 8) self.setSpritesetImage(1);
			else if (self.counter == 16 && Owner)
			{
				self.setSpritesetImage(2);
				Owner.GetPlayer().fist_incs = 1;
			}
			if (Owner && Owner.GetPlayer().fist_incs == 26)
				self.setSpritesetImage(3);
		}
	}
	
	override bool OnUse(DukePlayer p)
	{
		if (self.counter == 0 && !p.hitablockingwall())
		{
			self.counter = 1;
			self.ownerActor = p.actor;
			p.buttonpalette = self.pal;
			if (p.buttonpalette)
				ud.secretlevel = self.lotag;
			else ud.secretlevel = 0;
		}
		return true;
	}
}

