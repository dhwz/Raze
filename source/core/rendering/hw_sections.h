#pragma once

#include "build.h"

enum ESectionFlag
{
	Unclosed = 1,	// at least one unclosed loop
	Dumped = 2,		// builder was unable to properly construct, so content may not be usable for triangulator.
	BadWinding = 4,
};


struct SectionLine
{
	int section;
	int partnersection;
	int startpoint;
	int endpoint;
	int wall;
	int partner;

	DVector2 v1() const { return ::wall[startpoint].pos; }
	DVector2 v2() const { return ::wall[endpoint].pos; }
	walltype* wallp() const { return &::wall[wall]; }
	SectionLine* partnerLine() const;

};
extern TArray<SectionLine> sectionLines;

inline SectionLine* SectionLine::partnerLine() const
{
	return partner == -1 ? nullptr : &sectionLines[partner];
}

struct Section2Loop
{
	TArrayView<int> walls;
};

struct Section
{
	uint8_t flags;
	uint8_t dirty;
	uint8_t geomflags;
	unsigned index;
	int sector;
	// this uses a memory arena for storage, so use TArrayView instead of TArray
	TArrayView<int> lines;
	TArrayView<Section2Loop> loops;
};

extern TArray<Section> sections;
extern TArrayView<TArrayView<int>> sectionsPerSector;

void hw_CreateSections();
using Outline = TArray<TArray<DVector2>>;
using Point = std::pair<float, float>;
using FOutline = std::vector<std::vector<Point>>; // Data type was chosen so it can be passed directly into Earcut.
Outline BuildOutline(Section* section);

void hw_SetSplitSector(int sector, int startpos, int endpos);
void hw_ClearSplitSector();
