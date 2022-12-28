#pragma once
#include "build.h"
#include "gamefuncs.h"

class FSerializer;
struct IntRect;

void render_drawrooms(DCoreActor* playersprite, const DVector3& position, sectortype* sectnum, const DRotator& angles, double interpfrac, float fov = -1);
void render_camtex(DCoreActor* playersprite, const DVector3& position, sectortype* sect, const DRotator& angles, FGameTexture* camtex, IntRect& rect, double interpfrac);

struct PortalDesc
{
	int type;
	DVector3 delta;
	TArray<int> targets;
};

FSerializer& Serialize(FSerializer& arc, const char* key, PortalDesc& obj, PortalDesc* defval);


extern TArray<PortalDesc> allPortals;

inline void portalClear()
{
	allPortals.Clear();
}

inline int portalAdd(int type, int target, const DVector3& offset)
{
	auto& pt = allPortals[allPortals.Reserve(1)];
	pt.type = type;
	if (target >= 0) pt.targets.Push(target);
	pt.delta = offset;
	return allPortals.Size() - 1;
}

// merges portals in adjoining sectors.
inline void mergePortals()
{
	//Printf("Have %d portals\n", allPortals.Size());
	bool didsomething = true;
	while (didsomething)
	{
		didsomething = false;
		for (unsigned i = 0; i < allPortals.Size(); i++)
		{
			auto& pt1 = allPortals[i];
			if (pt1.type == PORTAL_SECTOR_CEILING || pt1.type == PORTAL_SECTOR_FLOOR)
			{
				for (unsigned j = i + 1; j < allPortals.Size(); j++)
				{
					auto& pt2 = allPortals[j];
					if (pt1.type != pt2.type || pt1.delta != pt2.delta) continue;
					for (unsigned s = 0; s < pt1.targets.Size() && pt2.targets.Size(); s++)
					{
						for (unsigned t = 0; t < pt2.targets.Size(); t++)
						{
							if (sectorsConnected(pt1.targets[s], pt2.targets[t]))
							{
								pt1.targets.Append(pt2.targets);
								pt2.targets.Reset();
								pt2.type = -1;
								for (auto& sec : sector)
								{
									//Printf("Merged %d and %d\n", i, j);
									if (sec.portalnum == (int)j) sec.portalnum = i;
								}
								didsomething = true;
								break;
							}
						}
					}
				}
			}
		}
	}
}
