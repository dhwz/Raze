
class RedneckCactusLargeYellow : DukeActor
{
	default
	{
		statnum STAT_ACTOR;
		SpriteSet "CACTUSLARGEYELLOW";
	}
	
	override void onHurt(DukePlayer p)
	{
		if (p.hurt_delay2 < 8)
		{
			p.actor.extra -= 2;
			p.hurt_delay2 = 16;
			p.pals = Color(32, 32, 0, 0);
			p.actor.PlayActorSound("PLAYER_LONGTERM_PAIN");
		}
	}

}

class RedneckCactusLargeBrown : RedneckCactusLargeYellow
{
	default
	{
		SpriteSet "CACTUSLARGEBROWN";
	}
}

class RedneckCactusLargeGreen2 : RedneckCactusLargeYellow
{
	default
	{
		SpriteSet "CACTUSLARGEGREEN2";
	}
}

class RedneckCactusLargeGreen : RedneckCactusLargeYellow
{
	default
	{
		SpriteSet "CACTUSLARGEGREEN", "CACTUSWITHHOLES";
	}
	
	override void onHit(DukeActor hitter)
	{
		if (self.SpriteSetIndex == 0)
		{
			if (self.pal != 4)
			{
				self.setSpriteSetImage(1);
				if (self.lotag != 0)
				{
					DukeSpriteIterator it;
					for(let act = it.First(); act; act = it.Next())
					{
						if (act is 'RedneckCactusLargeGreen' && act.spriteSetIndex == 0 && act.pal == 4)
						{
							if (self.lotag == act.lotag)
								act.setSpriteSetImage(1);
						}
					}
				}
			}
		}
		else
		{
			if (self.pal != 4)
			{
				self.PlayActorSound("SQUISHED");
				if (self.lotag != 0)
				{
					DukeSpriteIterator it;
					for(let act = it.First(); act; act = it.Next())
					{
						if (act is 'RedneckCactusLargeGreen' && act.spriteSetIndex == 1 && act.pal == 4)
						{
							if (self.lotag == act.lotag)
							{
								self.spawnguts('RedneckCactusDebris1', 12);
								self.spawnguts('RedneckCactusDebris2', 3);
								act.scale = (0, 0);
								self.scale = (0, 0);
							}
						}
					}
				}
				else
				{
					self.spawnguts('RedneckCactusDebris1', 12);
					self.spawnguts('RedneckCactusDebris2', 3);
					self.scale = (0, 0);
				}
			}
		}
	}
	
	override void OnMotoSmash(DukePlayer hitter)
	{
		self.PlayActorSound("SQUISHED");
		if (self.lotag != 0)
		{
			DukeSpriteIterator it;
			for(let act = it.First(); act; act = it.Next())
			{
				if (act is 'RedneckCactusLargeGreen' && act.pal == 4)
				{
					if (self.lotag == act.lotag)
					{
						act.scale = (0, 0);
					}
				}
			}
		}

		self.spawnguts('RedneckCactusDebris1', 12);
		self.spawnguts('RedneckCactusDebris2', 3);
		self.scale = (0, 0);
	}
}

class RedneckCactusWithHoles : RedneckCactusLargeGreen
{
	default
	{
		SpriteSetIndex 1;
	}
}

class RedneckCactusDrug : RedneckCactusLargeYellow
{
	default
	{
		SpriteSet "CACTUSDRUG", "CACTUSSMALL";
	}
	
	override void onHit(DukeActor hitter)
	{
		if (self.spritesetIndex == 0 && self.pal != 19)
		{
			self.setSpriteSetImage(1);
		}
		else if (self.spritesetIndex == 1)
		{
			self.PlayActorSound("SQUISHED");
			self.spawnguts('RedneckCactusDebris2', 3);
			self.Destroy();
		}
	}
	
	override void onTouch(DukePlayer toucher)
	{
		if (self.spritesetindex == 0 && self.pal == 19)
		{
			self.pal = 0;
			toucher.DrugMode = 5;
			toucher.actor.extra = gs.max_player_health;
		}
	}
	
	override void OnMotoSmash(DukePlayer hitter)
	{
		self.spawnguts('RedneckCactusDebris2', 3);
		self.scale = (0, 0);
	}
	
}

class RedneckCactusSmall : RedneckCactusDrug
{
	default
	{
		SpriteSetIndex 1;
	}
}

