class RedneckSheriff : DukeActor
{
	const LTHSNDAMB = 16;
	const LTHSTRENGTH = 500;
	default
	{
		pic "LTH";
		+INTERNAL_BADGUY;
		+KILLCOUNT;
		Strength LTHSTRENGTH;
	}
	override void Initialize(DukeActor spawner)
	{
		self.scale =  (0.375, 0.34375);
		self.setClipDistFromTile();
	}

}

class RedneckRASheriff : RedneckSheriff // LTH (4352) // less health than in RR!
{
	default
	{
		Strength 300;
	}
}

