
class DukeLizMan : DukeActor
{
	const LIZSTRENGTH = 100;
	default
	{
		pic "LIZMAN";
		+INTERNAL_BADGUY;
		+KILLCOUNT;
		+GREENSLIMEFOOD;
		+DONTENTERWATER;
		+RANDOMANGLEONWATER;
		precacheclass "DukeLizmanHead", "DukeLizmanArm", "DukeLizmanLeg";
		Strength LIZSTRENGTH;

	}
	
	override void PlayFTASound(int mode)
	{
		self.PlayActorSound("CAPT_RECOG");
	}
	
	
}

	
//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class DukeLizManSpitting : DukeLizMan
{
	default
	{
		pic "LIZMANSPITTING";
	}
	

}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class DukeLizManFeeding : DukeLizMan
{
	// this one has setup code but no implementation.
	default
	{
		pic "LIZMANFEEDING";
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class DukeLizManJump : DukeLizMan
{
	default
	{
		pic "LIZMANJUMP";
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class DukeLizManStayput : DukeLizMan
{
	default
	{
		pic "LIZMANSTAYPUT";
		+BADGUYSTAYPUT;
	}
	
	override void PlayFTASound(int mode)
	{
	}

}

