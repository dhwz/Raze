
class DukeCactusBroke : DukeActor
{
	default
	{
		pic "CACTUSBROKE";
	}
	
	override void Initialize()
	{
		self.cstat |= CSTAT_SPRITE_BLOCK_ALL;
		self.clipdist = 8;
		self.ChangeStat(STAT_DEFAULT);
	}
}

class DukeCactus : DukeCactusBroke
{
	default
	{
		spriteset "CACTUS", "CACTUSBROKE";
	}
	

	override void onHit(DukeActor hitter)
	{
		if (self.spritesetindex == 0 && hitter.actorflag1(SFLAG_INFLAME))
		{
			let scrap = Raze.isRR()? DukeScrap.Scrap6 : DukeScrap.Scrap3;
			
			for (int k = 0; k < 64; k++)
			{
				double ang = frandom(0, 360);
				double vel = frandom(4, 8);
				double zvel = -frandom(0, 16) - self.vel.Z * 0.25;

				let spawned = dlevel.SpawnActor(self.sector, self.pos.plusZ(-48), "DukeScrap", -8, (0.75, 0.75), ang, vel, zvel, self, STAT_MISC);
				if (spawned)
				{
					spawned.spriteextra = DukeScrap.Scrap3 + random(0, 3);
					spawned.pal = 6;
				}
			}

			self.setSpritesetImage(1);
			self.cstat &= ~CSTAT_SPRITE_BLOCK_ALL;
		}
	}
	
	override void onHurt(DukePlayer p)
	{
		if (self.spritesetindex == 0 && p.hurt_delay < 8)
		{
			p.actor.extra -= 5;
			p.hurt_delay = 16;
			p.pals = Color(32, 32, 0, 0);
			p.actor.PlayActorSound("PLAYER_LONGTERM_PAIN");
		}
	}
}
