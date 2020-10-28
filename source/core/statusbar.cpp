/*
** shared_sbar.cpp
** Base status bar implementation
**
**---------------------------------------------------------------------------
** Copyright 1998-2006 Randy Heit
** Copyright 2017 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include <assert.h>

#include "build.h"
#include "templates.h"
#include "statusbar.h"
#include "c_cvars.h"
#include "c_dispatch.h"
#include "c_console.h"
#include "v_video.h"
#include "filesystem.h"
#include "s_soundinternal.h"
#include "serializer.h"
#include "serialize_obj.h"
#include "cmdlib.h"
#include "vm.h"
#include "gstrings.h"
#include "utf8.h"
#include "texturemanager.h"
#include "cmdlib.h"
#include "v_draw.h"
#include "v_font.h"
#include "v_draw.h"
#include "gamecvars.h"
#include "m_fixed.h"
#include "gamecontrol.h"
#include "gamestruct.h"
#include "razemenu.h"
#include "mapinfo.h"

#include "../version.h"

#define XHAIRSHRINKSIZE		(1./18)
#define XHAIRPICKUPSIZE		(2+XHAIRSHRINKSIZE)
#define POWERUPICONSIZE		32

//IMPLEMENT_CLASS(DHUDFont, true, false);

EXTERN_CVAR (Bool, am_showmonsters)
EXTERN_CVAR (Bool, am_showsecrets)
EXTERN_CVAR (Bool, am_showtime)
EXTERN_CVAR (Bool, am_showtotaltime)
EXTERN_CVAR (Bool, noisedebug)
EXTERN_CVAR(Bool, vid_fps)
EXTERN_CVAR(Bool, inter_subtitles)

DStatusBarCore *StatusBar;

extern int setblocks;

CVAR (Bool, idmypos, false, 0);


FGameTexture* CrosshairImage;
static int CrosshairNum;

CVAR(Color, crosshaircolor, 0xff0000, CVAR_ARCHIVE);
CVAR(Int, crosshairhealth, 2, CVAR_ARCHIVE);
CVAR(Float, crosshairscale, 1.0, CVAR_ARCHIVE);
CVAR(Bool, crosshairgrow, false, CVAR_ARCHIVE);
EXTERN_CVAR(Bool, vid_fps)

void ST_LoadCrosshair(int num, bool alwaysload)
{
	char name[16];
	char size;

	if (!alwaysload && CrosshairNum == num && CrosshairImage != NULL)
	{ // No change.
		return;
	}

	if (num == 0)
	{
		CrosshairNum = 0;
		CrosshairImage = NULL;
		return;
	}
	if (num < 0)
	{
		num = -num;
	}
	size = (twod->GetWidth() < 640) ? 'S' : 'B';

	mysnprintf(name, countof(name), "XHAIR%c%d", size, num);
	FTextureID texid = TexMan.CheckForTexture(name, ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
	if (!texid.isValid())
	{
		mysnprintf(name, countof(name), "XHAIR%c1", size);
		texid = TexMan.CheckForTexture(name, ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
		if (!texid.isValid())
		{
			texid = TexMan.CheckForTexture("XHAIRS1", ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
		}
	}
	CrosshairNum = num;
	CrosshairImage = TexMan.GetGameTexture(texid);
}

void ST_UnloadCrosshair()
{
	CrosshairImage = NULL;
	CrosshairNum = 0;
}


//---------------------------------------------------------------------------
//
// DrawCrosshair
//
//---------------------------------------------------------------------------

void ST_DrawCrosshair(int phealth, double xpos, double ypos, double scale)
{
	uint32_t color;
	double size;
	int w, h;

	// Don't draw the crosshair if there is none
	if (CrosshairImage == NULL)
	{
		return;
	}

	if (crosshairscale > 0.0f)
	{
		size = twod->GetHeight() * crosshairscale * 0.005;
	}
	else
	{
		size = 1.;
	}

	if (crosshairgrow)
	{
		size *= scale;
	}

	w = int(CrosshairImage->GetDisplayWidth() * size);
	h = int(CrosshairImage->GetDisplayHeight() * size);

	if (crosshairhealth == 1)
	{
		// "Standard" crosshair health (green-red)
		int health = phealth;

		if (health >= 85)
		{
			color = 0x00ff00;
		}
		else
		{
			int red, green;
			health -= 25;
			if (health < 0)
			{
				health = 0;
			}
			if (health < 30)
			{
				red = 255;
				green = health * 255 / 30;
			}
			else
			{
				red = (60 - health) * 255 / 30;
				green = 255;
			}
			color = (red << 16) | (green << 8);
		}
	}
	else if (crosshairhealth == 2)
	{
		// "Enhanced" crosshair health (blue-green-yellow-red)
		int health = clamp(phealth, 0, 200);
		float rr, gg, bb;

		float saturation = health < 150 ? 1.f : 1.f - (health - 150) / 100.f;

		HSVtoRGB(&rr, &gg, &bb, health * 1.2f, saturation, 1);
		int red = int(rr * 255);
		int green = int(gg * 255);
		int blue = int(bb * 255);

		color = (red << 16) | (green << 8) | blue;
	}
	else
	{
		color = crosshaircolor;
	}

	DrawTexture(twod, CrosshairImage,
		xpos, ypos,
		DTA_DestWidth, w,
		DTA_DestHeight, h,
		DTA_AlphaChannel, true,
		DTA_FillColor, color & 0xFFFFFF,
		TAG_DONE);
}


//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

enum ENumFlags
{
	FNF_WHENNOTZERO = 0x1,
	FNF_FILLZEROS = 0x2,
};

void FormatNumber(int number, int minsize, int maxsize, int flags, const FString& prefix, FString* result)
{
	static int maxvals[] = { 1, 9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999 };

	if (number == 0 && (flags & FNF_WHENNOTZERO))
	{
		*result = "";
		return;
	}
	if (maxsize > 0 && maxsize < 10)
	{
		number = clamp(number, -maxvals[maxsize - 1], maxvals[maxsize]);
	}
	FString& fmt = *result;
	if (minsize <= 1) fmt.Format("%s%d", prefix.GetChars(), number);
	else if (flags & FNF_FILLZEROS) fmt.Format("%s%0*d", prefix.GetChars(), minsize, number);
	else fmt.Format("%s%*d", prefix.GetChars(), minsize, number);
}

void DStatusBarCore::ValidateResolution(int& hres, int& vres) const
{
	if (hres == 0)
	{
		static const int HORIZONTAL_RESOLUTION_DEFAULT = 320;
		hres = HORIZONTAL_RESOLUTION_DEFAULT;
	}

	if (vres == 0)
	{
		static const int VERTICAL_RESOLUTION_DEFAULT = 200;
		vres = VERTICAL_RESOLUTION_DEFAULT;
	}
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::StatusbarToRealCoords(double& x, double& y, double& w, double& h) const
{
	if (SBarScale.X == -1 || ForcedScale)
	{
		int hres = HorizontalResolution;
		int vres = VerticalResolution;
		ValidateResolution(hres, vres);

		VirtualToRealCoords(twod, x, y, w, h, hres, vres, true, true);
	}
	else
	{
		x = ST_X + x * SBarScale.X;
		y = ST_Y + y * SBarScale.Y;
		w *= SBarScale.X;
		h *= SBarScale.Y;
	}
}

//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::DrawGraphic(FTextureID texture, double x, double y, int flags, double Alpha, double boxwidth, double boxheight, double scaleX, double scaleY, PalEntry color, int translation, double rotate, ERenderStyle style)
{
	if (!texture.isValid())
		return;

	FGameTexture* tex = TexMan.GetGameTexture(texture, !(flags & DI_DONTANIMATE));
	DrawGraphic(tex, x, y, flags, Alpha, boxwidth, boxheight, scaleX, scaleY, color, translation, rotate, style);
}

void DStatusBarCore::DrawGraphic(FGameTexture* tex, double x, double y, int flags, double Alpha, double boxwidth, double boxheight, double scaleX, double scaleY, PalEntry color, int translation, double rotate, ERenderStyle style)
{
	double texwidth = tex->GetDisplayWidth() * scaleX;
	double texheight = tex->GetDisplayHeight() * scaleY;
	double texleftoffs = tex->GetDisplayLeftOffset() * scaleY;
	double textopoffs = tex->GetDisplayTopOffset() * scaleY;
	double boxleftoffs, boxtopoffs;

	if (boxwidth > 0 || boxheight > 0)
	{
		if (!(flags & DI_FORCEFILL))
		{
			double scale1 = 1., scale2 = 1.;

			if (boxwidth > 0 && (boxwidth < texwidth || (flags & DI_FORCESCALE)))
			{
				scale1 = boxwidth / texwidth;
			}
			if (boxheight != -1 && (boxheight < texheight || (flags & DI_FORCESCALE)))
			{
				scale2 = boxheight / texheight;
			}

			if (flags & DI_FORCESCALE)
			{
				if (boxwidth <= 0 || (boxheight > 0 && scale2 < scale1))
					scale1 = scale2;
			}
			else scale1 = MIN(scale1, scale2);

			boxwidth = texwidth * scale1;
			boxheight = texheight * scale1;
			boxleftoffs = texleftoffs * scale1;
			boxtopoffs = textopoffs * scale1;
		}
	}
	else
	{
		boxwidth = texwidth;
		boxheight = texheight;
		boxleftoffs = texleftoffs;
		boxtopoffs = textopoffs;
	}

	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	Alpha *= this->Alpha;
	if (Alpha <= 0) return;
	x += drawOffset.X;
	y += drawOffset.Y;

	if (flags & DI_ITEM_RELCENTER)
	{
		if (flags & DI_MIRROR) boxleftoffs = -boxleftoffs;
		if (flags & DI_MIRRORY)	boxtopoffs = -boxtopoffs;
		x -= boxwidth / 2 + boxleftoffs;
		y -= boxheight / 2 + boxtopoffs;
	}
	else
	{
		switch (flags & DI_ITEM_HMASK)
		{
		case DI_ITEM_HCENTER:	x -= boxwidth / 2; break;
		case DI_ITEM_RIGHT:		x -= boxwidth; break;
		case DI_ITEM_HOFFSET:	x -= boxleftoffs; break;
		}

		switch (flags & DI_ITEM_VMASK)
		{
		case DI_ITEM_VCENTER: y -= boxheight / 2; break;
		case DI_ITEM_BOTTOM:  y -= boxheight; break;
		case DI_ITEM_VOFFSET: y -= boxtopoffs; break;
		}
	}

	if (!fullscreenOffsets)
	{
		StatusbarToRealCoords(x, y, boxwidth, boxheight);
	}
	else
	{
		double orgx, orgy;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		// move stuff in the top right corner a bit down if the fps counter is on.
		if ((flags & (DI_SCREEN_HMASK | DI_SCREEN_VMASK)) == DI_SCREEN_RIGHT_TOP && vid_fps) y += 10;

		DVector2 Scale = GetHUDScale();

		x *= Scale.X;
		y *= Scale.Y;
		boxwidth *= Scale.X;
		boxheight *= Scale.Y;
		x += orgx;
		y += orgy;
	}
	DrawTexture(twod, tex, x, y,
		DTA_TopOffset, 0,
		DTA_LeftOffset, 0,
		DTA_DestWidthF, boxwidth,
		DTA_DestHeightF, boxheight,
		DTA_Color, color,
		DTA_TranslationIndex, translation? translation : (flags & DI_TRANSLATABLE) ? GetTranslation() : 0,
		DTA_ColorOverlay, (flags & DI_DIM) ? MAKEARGB(170, 0, 0, 0) : 0,
		DTA_Alpha, Alpha,
		DTA_AlphaChannel, !!(flags & DI_ALPHAMAPPED),
		DTA_FillColor, (flags & DI_ALPHAMAPPED) ? 0 : -1,
		DTA_FlipX, !!(flags & DI_MIRROR),
		DTA_FlipY, !!(flags& DI_MIRRORY),
		DTA_Rotate, rotate,
		DTA_LegacyRenderStyle, style,
		TAG_DONE);
}


//============================================================================
//
// draw a string
//
//============================================================================

void DStatusBarCore::DrawString(FFont* font, const FString& cstring, double x, double y, int flags, double Alpha, int translation, int spacing, EMonospacing monospacing, int shadowX, int shadowY, double scaleX, double scaleY)
{
	bool monospaced = monospacing != EMonospacing::Off;
	double dx = 0;
	int spacingparm = monospaced ? -spacing : spacing;

	switch (flags & DI_TEXT_ALIGN)
	{
	default:
		break;
	case DI_TEXT_ALIGN_RIGHT:
		dx = font->StringWidth(cstring, spacingparm);
		break;
	case DI_TEXT_ALIGN_CENTER:
		dx = font->StringWidth(cstring, spacingparm) / 2;
		break;
	}

	// Take text scale into account
	x -= dx * scaleX;

	const uint8_t* str = (const uint8_t*)cstring.GetChars();
	const EColorRange boldTranslation = EColorRange(translation ? translation - 1 : NumTextColors - 1);
	int fontcolor = translation;
	double orgx = 0, orgy = 0;
	DVector2 Scale;

	if (fullscreenOffsets)
	{
		Scale = GetHUDScale();
		shadowX *= (int)Scale.X;
		shadowY *= (int)Scale.Y;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		// move stuff in the top right corner a bit down if the fps counter is on.
		if ((flags & (DI_SCREEN_HMASK | DI_SCREEN_VMASK)) == DI_SCREEN_RIGHT_TOP && vid_fps) y += 10;
	}
	else
	{
		Scale = { 1.,1. };
	}
	int ch;
	while (ch = GetCharFromString(str), ch != '\0')
	{
		if (ch == ' ')
		{
			x += monospaced ? spacing : font->GetSpaceWidth() + spacing;
			continue;
		}
		else if (ch == TEXTCOLOR_ESCAPE)
		{
			EColorRange newColor = V_ParseFontColor(str, translation, boldTranslation);
			if (newColor != CR_UNDEFINED)
				fontcolor = newColor;
			continue;
		}

		int width;
		FGameTexture* c = font->GetChar(ch, fontcolor, &width);
		if (c == NULL) //missing character.
		{
			continue;
		}
		width += font->GetDefaultKerning();

		if (!monospaced) //If we are monospaced lets use the offset
			x += (c->GetDisplayLeftOffset() + 1); //ignore x offsets since we adapt to character size

		double rx, ry, rw, rh;
		rx = x + drawOffset.X;
		ry = y + drawOffset.Y;
		rw = c->GetDisplayWidth();
		rh = c->GetDisplayHeight();

		if (monospacing == EMonospacing::CellCenter)
			rx += (spacing - rw) / 2;
		else if (monospacing == EMonospacing::CellRight)
			rx += (spacing - rw);

		if (!fullscreenOffsets)
		{
			StatusbarToRealCoords(rx, ry, rw, rh);
		}
		else
		{
			rx *= Scale.X;
			ry *= Scale.Y;
			rw *= Scale.X;
			rh *= Scale.Y;

			rx += orgx;
			ry += orgy;
		}

		// Apply text scale
		rw *= scaleX;
		rh *= scaleY;

		// This is not really such a great way to draw shadows because they can overlap with previously drawn characters.
		// This may have to be changed to draw the shadow text up front separately.
		if ((shadowX != 0 || shadowY != 0) && !(flags & DI_NOSHADOW))
		{
			DrawChar(twod, font, CR_UNTRANSLATED, rx + shadowX, ry + shadowY, ch,
				DTA_DestWidthF, rw,
				DTA_DestHeightF, rh,
				DTA_Alpha, (Alpha * 0.4),
				DTA_FillColor, 0,
				TAG_DONE);
		}
		DrawChar(twod, font, fontcolor, rx, ry, ch,
			DTA_DestWidthF, rw,
			DTA_DestHeightF, rh,
			DTA_Alpha, Alpha,
			TAG_DONE);

		dx = monospaced
			? spacing
			: width + spacing - (c->GetDisplayLeftOffset() + 1);

		// Take text scale into account
		x += dx * scaleX;
	}
}

void SBar_DrawString(DStatusBarCore* self, DHUDFont* font, const FString& string, double x, double y, int flags, int trans, double alpha, int wrapwidth, int linespacing, double scaleX, double scaleY)
{
	if (font == nullptr) ThrowAbortException(X_READ_NIL, nullptr);
	if (!twod->HasBegun2D()) ThrowAbortException(X_OTHER, "Attempt to draw to screen outside a draw function");

	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	if (wrapwidth > 0)
	{
		auto brk = V_BreakLines(font->mFont, int(wrapwidth * scaleX), string, true);
		for (auto& line : brk)
		{
			self->DrawString(font->mFont, line.Text, x, y, flags, alpha, trans, font->mSpacing, font->mMonospacing, font->mShadowX, font->mShadowY, scaleX, scaleY);
			y += (font->mFont->GetHeight() + linespacing) * scaleY;
		}
	}
	else
	{
		self->DrawString(font->mFont, string, x, y, flags, alpha, trans, font->mSpacing, font->mMonospacing, font->mShadowX, font->mShadowY, scaleX, scaleY);
	}
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::TransformRect(double& x, double& y, double& w, double& h, int flags)
{
	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	x += drawOffset.X;
	y += drawOffset.Y;

	if (!fullscreenOffsets)
	{
		StatusbarToRealCoords(x, y, w, h);
	}
	else
	{
		double orgx, orgy;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		// move stuff in the top right corner a bit down if the fps counter is on.
		if ((flags & (DI_SCREEN_HMASK | DI_SCREEN_VMASK)) == DI_SCREEN_RIGHT_TOP && vid_fps) y += 10;

		DVector2 Scale = GetHUDScale();

		x *= Scale.X;
		y *= Scale.Y;
		w *= Scale.X;
		h *= Scale.Y;
		x += orgx;
		y += orgy;
	}
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::Fill(PalEntry color, double x, double y, double w, double h, int flags)
{
	double Alpha = color.a * this->Alpha / 255;
	if (Alpha <= 0) return;

	TransformRect(x, y, w, h, flags);

	int x1 = int(x);
	int y1 = int(y);
	int ww = int(x + w - x1);	// account for scaling to non-integers. Truncating the values separately would fail for cases like 
	int hh = int(y + h - y1);	// y=3.5, height = 5.5 where adding both values gives a larger integer than adding the two integers.

	Dim(twod, color, float(Alpha), x1, y1, ww, hh);
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::SetClipRect(double x, double y, double w, double h, int flags)
{
	TransformRect(x, y, w, h, flags);
	int x1 = int(x);
	int y1 = int(y);
	int ww = int(x + w - x1);	// account for scaling to non-integers. Truncating the values separately would fail for cases like 
	int hh = int(y + h - y1); // y=3.5, height = 5.5 where adding both values gives a larger integer than adding the two integers.
	twod->SetClipRect(x1, y1, ww, hh);
}


