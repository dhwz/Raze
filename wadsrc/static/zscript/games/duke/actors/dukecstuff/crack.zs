
class DukeCrack : DukeActor
{
	default
	{
		pic "CRACK1";
		+NOFALLER;
	}
	
	override void Initialize(DukeActor spawner)
	{
		self.cstat |= (self.cstat & CSTAT_SPRITE_ALIGNMENT_MASK) ? CSTAT_SPRITE_BLOCK : (CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_ALIGNMENT_WALL);
		self.extra = 1;

		if (ud.multimode < 2 && self.pal != 0)
		{
			self.scale = (0, 0);
			self.ChangeStat(STAT_MISC);
			return;
		}

		self.pal = 0;
		self.ownerActor = self;
		self.vel.X = 0.5;
		self.DoMove(CLIPMASK0);
		self.ChangeStat(STAT_STANDABLE);
	}
	
	override void Tick()
	{
		if (self.hitag > 0)
		{
			self.counter = self.cstat;
			self.temp_angle = self.angle;
			if (self.ifhitbyweapon() == 2) // explosive damage
			{
				DukeStatIterator it;
				for(let a1 = it.First(STAT_STANDABLE); a1; a1 = it.Next())
				{
					if (self.hitag == a1.hitag && a1.bBRIGHTEXPLODE)
						if (a1.shade != -32)
							a1.shade = -32;
				}
				self.detonate('DukeExplosion2');
			}
			else
			{
				self.cstat = self.counter;
				self.angle = self.temp_angle;
				self.extra = 0;
			}
		}
	}
	
	override bool animate(tspritetype tspr)
	{
		tspr.shade = 16;
		return true;
	}
}
