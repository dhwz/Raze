
class RedneckEmptyBike : DukeActor
{
	default
	{
		pic "EMPTYBIKE";
		scaleX 0.28125;
		scaleY 0.28125;
		lotag 1;
		statnum STAT_ACTOR;
	}
	
	override void Initialize()
	{
		if (ud.multimode < 2 && self.pal == 1)
		{
			self.scale = (0, 0);
			self.ChangeStat(STAT_MISC);
			return;
		}
		self.pal = 0;
		self.setClipDistFromTile();
		self.saved_ammo = 100;
		self.cstat = CSTAT_SPRITE_BLOCK_ALL;
	}
	
	override void Tick()
	{
		self.makeitfall();
		self.getglobalz();
		if (self.sector.lotag == ST_1_ABOVE_WATER)
		{
			self.setPosition((self.pos.XY, self.floorz + 16));
		}
	}
	
	override bool OnUse(DukePlayer p)
	{
		if (!p.OnMotorcycle && p.cursector.lotag != ST_2_UNDERWATER)
		{
			p.actor.pos.XY = self.pos.XY;
			p.SetTargetAngle(self.angle, true);
			p.ammo_amount[RRWpn.MOTORCYCLE_WEAPON] = self.saved_ammo;
			self.Destroy();
			p.StartMotorcycle();
		}
		return true;
	}
}

class RedneckEmptyBoat : DukeActor
{
	default
	{
		pic "EMPTYBOAT";
		scaleX 0.5;
		scaleY 0.5;
		lotag 1;
		statnum STAT_ACTOR;
	}
	
	override void Initialize()
	{
		if (ud.multimode < 2 && self.pal == 1)
		{
			self.scale = (0, 0);
			self.ChangeStat(STAT_MISC);
			return;
		}
		self.pal = 0;
		self.setClipDistFromTile();
		self.saved_ammo = 20;
		self.cstat = CSTAT_SPRITE_BLOCK_ALL;
	}
	
	
	override void Tick()
	{
		self.makeitfall();
		self.getglobalz();
	}
	
	override bool OnUse(DukePlayer p)
	{
		if (!p.OnBoat)
		{
			p.actor.pos.XY = self.pos.XY;
			p.SetTargetAngle(self.angle, true);
			p.ammo_amount[RRWpn.BOAT_WEAPON] = self.saved_ammo;
			self.Destroy();
			p.StartBoat();
		}
		return true;
	}
}
