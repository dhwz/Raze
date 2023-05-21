
class DukeGlassPieces : DukeActor
{
	default
	{
		spriteset "GLASSPIECES";
	}
	override void Tick()
	{
		let sectp = self.sector;

		self.makeitfall();

		if (self.vel.Z > 16) self.vel.Z = 16;
		if (sectp == null)
		{
			self.Destroy();
			return;
		}

		if (self.pos.Z == self.floorz - 1 && self.counter < 3)
		{
			self.vel.Z = -(3 - self.counter) - frandom(0, 2);
			if (sectp.lotag == 2)
				self.vel.Z *= 0.5;

			self.scale *= 0.5;
			if (Duke.rnd(96))
				self.SetPosition(self.pos);
			self.counter++;//Number of bounces
		}
		else if (self.counter == 3)
		{
			self.Destroy();
			return;
		}

		if(self.vel.X > 0)
		{
			self.vel.X -= 1/8.;
			self.cstat = randomFlip();
		}
		else self.vel.X = 0;

		self.DoMove(CLIPMASK0);
	}
}

class DukeGlassPieces1 : DukeGlassPieces
{
	default
	{
		spriteset "GLASSPIECES1";
	}
}

class DukeGlassPieces2 : DukeGlassPieces
{
	default
	{
		spriteset "GLASSPIECES2";
	}
}

class RedneckPopcorn : DukeGlassPieces
{
	default
	{
		pic "POPCORN";
	}
}

