/*
 * Seven Kingdoms 2: The Fryhtan War
 *
 * Copyright 1999 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Filename    : IMGFUN.H
// Description : Declaration of assembly functions

#ifndef __IMGFUN_H
#define __IMGFUN_H

#include <asmfun.h>

// ----- define enum type PixelFormatFlag16 -------//

enum PixelFormatFlag
{
	PIXFORM_RGB_555 = 0,	// red on low bits
	PIXFORM_RGB_565 = 1,
	PIXFORM_BGR_555 = 2,	// blue on low bits
	PIXFORM_BGR_565 = 3,
};


extern "C"
{
	// basic functions

	void IMGcall INITeffect(int pixelFormatFlag) __asmsym__("_INITeffect");

	// 8->16 blt

	// functions with blend bitmap

	void IMGcall IMGbltBlendRemap( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		short *colorTable ) __asmsym__("_IMGbltBlendRemap");
	void IMGcall IMGbltBlendRemapHMirror( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		short *colorTable ) __asmsym__("_IMGbltBlendRemapHMirror");
	void IMGcall IMGbltBlendAreaRemap( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		int srcX1, int srcY1, int srcX2, int srcY2, short *colorTable ) __asmsym__("_IMGbltBlendAreaRemap");
	void IMGcall IMGbltBlendAreaRemapHMirror( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		int srcX1, int srcY1, int srcX2, int srcY2, short *colorTable ) __asmsym__("_IMGbltBlendAreaRemapHMirror");

	// functions with weaker blend bitmap

	void IMGcall IMGbltWeakblendRemap( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		short *colorTable ) __asmsym__("_IMGbltWeakblendRemap");
	void IMGcall IMGbltWeakblendRemapHMirror( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		short *colorTable ) __asmsym__("_IMGbltWeakblendRemapHMirror");
	void IMGcall IMGbltWeakblendAreaRemap( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		int srcX1, int srcY1, int srcX2, int srcY2, short *colorTable ) __asmsym__("_IMGbltWeakblendAreaRemap");
	void IMGcall IMGbltWeakblendAreaRemapHMirror( short *imageBuf, int pitch, int x, int y, char *bitmapBuf,
		int srcX1, int srcY1, int srcX2, int srcY2, short *colorTable ) __asmsym__("_IMGbltWeakblendAreaRemapHMirror");
};

#endif
