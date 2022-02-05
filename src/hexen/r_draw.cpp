//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//


#include "h2def.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "r_local.hpp"
#include "v_video.hpp"
#include "lump.hpp"
#include "memory.hpp"

/*

All drawing to the view buffer is accomplished in this file.  The other refresh
files only know about ccordinates, not the architecture of the frame buffer.

*/

byte *viewimage;
int viewwidth, scaledviewwidth, viewheight, viewwindowx, viewwindowy;
byte *ylookup[MAXHEIGHT];
int columnofs[MAXWIDTH];
//byte translations[3][256]; // color tables for different players

/*
==================
=
= R_DrawColumn
=
= Source is the top of the column to scale
=
==================
*/

lighttable_t *dc_colormap;
int dc_x;
int dc_yl;
int dc_yh;
fixed_t dc_iscale;
fixed_t dc_texturemid;
byte *dc_source;                // first pixel in a column (possibly virtual)

int dccount;                    // just for profiling

void R_DrawColumn()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = dc_colormap[dc_source[(frac >> FRACBITS) & 127]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

void R_DrawColumnLow()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
//      dccount++;
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = dc_colormap[dc_source[(frac >> FRACBITS) & 127]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

void R_DrawTLColumn()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    if (!dc_yl)
        dc_yl = 1;
    if (dc_yh == viewheight - 1)
        dc_yh = viewheight - 2;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawTLColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = tinttable[*dest +
                          (dc_colormap[dc_source[(frac >> FRACBITS) & 127]] <<
                           8)];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

//============================================================================
//
// R_DrawAltTLColumn
//
//============================================================================

void R_DrawAltTLColumn()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    if (!dc_yl)
        dc_yl = 1;
    if (dc_yh == viewheight - 1)
        dc_yh = viewheight - 2;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawAltTLColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = tinttable[((*dest) << 8)
                          + dc_colormap[dc_source[(frac >> FRACBITS) & 127]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

/*
========================
=
= R_DrawTranslatedColumn
=
========================
*/

byte *dc_translation;
byte *translationtables;

void R_DrawTranslatedColumn()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

//============================================================================
//
// R_DrawTranslatedTLColumn
//
//============================================================================

void R_DrawTranslatedTLColumn()
{
    int count;
    byte *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        *dest = tinttable[((*dest) << 8)
                          +
                          dc_colormap[dc_translation
                                      [dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

//============================================================================
//
// R_DrawTranslatedAltTLColumn
//
//============================================================================

/*
void R_DrawTranslatedAltTLColumn ()
{
	int			count;
	byte		*dest;
	fixed_t		frac, fracstep;	

	count = dc_yh - dc_yl;
	if (count < 0)
		return;
				
#ifdef RANGECHECK
	if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
		I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

	dest = ylookup[dc_yl] + columnofs[dc_x];
	
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	do
	{
		*dest = tinttable[*dest
			+(dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]<<8)];
		dest += SCREENWIDTH;
		frac += fracstep;
	} while (count--);
}
*/

//--------------------------------------------------------------------------
//
// PROC R_InitTranslationTables
//
//--------------------------------------------------------------------------

void R_InitTranslationTables()
{
    int i;
    byte *transLump;
    int lumpnum;

    V_LoadTintTable();

    // Allocate translation tables
    translationtables = zmalloc<byte *>(256 * 3 * (maxplayers - 1), PU_STATIC, 0);

    for (i = 0; i < 3 * (maxplayers - 1); i++)
    {
        lumpnum = W_GetNumForName("trantbl0") + i;
        transLump = cache_lump_num<byte *>(lumpnum, PU_STATIC);
        memcpy(translationtables + i * 256, transLump, 256);
        W_ReleaseLumpNum(lumpnum);
    }
}

/*
================
=
= R_DrawSpan
=
================
*/

int ds_y;
int ds_x1;
int ds_x2;
lighttable_t *ds_colormap;
fixed_t ds_xfrac;
fixed_t ds_yfrac;
fixed_t ds_xstep;
fixed_t ds_ystep;
byte *ds_source;                // start of a 64*64 tile image

int dscount;                    // just for profiling

void R_DrawSpan()
{
    fixed_t xfrac, yfrac;
    byte *dest;
    int count, spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= SCREENWIDTH
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
//      dscount++;
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1;
    do
    {
        spot = ((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63);
        *dest++ = ds_colormap[ds_source[spot]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }
    while (count--);
}

void R_DrawSpanLow()
{
    fixed_t xfrac, yfrac;
    byte *dest;
    int count, spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= SCREENWIDTH
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
//      dscount++;
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1;
    do
    {
        spot = ((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63);
        *dest++ = ds_colormap[ds_source[spot]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }
    while (count--);
}



/*
================
=
= R_InitBuffer
=
=================
*/

void R_InitBuffer(int width, int height)
{
    int i;

    viewwindowx = (SCREENWIDTH - width) >> 1;
    for (i = 0; i < width; i++)
        columnofs[i] = viewwindowx + i;
    if (width == SCREENWIDTH)
        viewwindowy = 0;
    else
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;
    for (i = 0; i < height; i++)
        ylookup[i] = I_VideoBuffer + (i + viewwindowy) * SCREENWIDTH;
}


/*
==================
=
= R_DrawViewBorder
=
= Draws the border around the view for different size windows
==================
*/

boolean BorderNeedRefresh;

void R_DrawViewBorder()
{
    byte *src, *dest;
    int x, y;

    if (scaledviewwidth == SCREENWIDTH)
        return;

    src = cache_lump_name<byte *>("F_022", PU_CACHE);
    dest = I_VideoBuffer;

    for (y = 0; y < SCREENHEIGHT - SBARHEIGHT; y++)
    {
        for (x = 0; x < SCREENWIDTH / 64; x++)
        {
            memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (SCREENWIDTH & 63)
        {
            memcpy(dest, src + ((y & 63) << 6), SCREENWIDTH & 63);
            dest += (SCREENWIDTH & 63);
        }
    }
    for (x = (viewwindowx >> crispy->hires); x < (viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires); x += 16)
    {
        V_DrawPatch(x, (viewwindowy >> crispy->hires) - 4, cache_lump_name<patch_t *>("bordt", PU_CACHE));
        V_DrawPatch(x, (viewwindowy >> crispy->hires) + (viewheight >> crispy->hires), cache_lump_name<patch_t *>("bordb",
                                                                 PU_CACHE));
    }
    for (y = (viewwindowy >> crispy->hires); y < (viewwindowy >> crispy->hires) + (viewheight >> crispy->hires); y += 16)
    {
        V_DrawPatch((viewwindowx >> crispy->hires) - 4, y, cache_lump_name<patch_t *>("bordl", PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), y, cache_lump_name<patch_t *>("bordr",
                                                                PU_CACHE));
    }
    V_DrawPatch((viewwindowx >> crispy->hires) - 4, (viewwindowy >> crispy->hires) - 4, cache_lump_name<patch_t *>("bordtl",
                                                                  PU_CACHE));
    V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), (viewwindowy >> crispy->hires) - 4,
                cache_lump_name<patch_t *>("bordtr", PU_CACHE));
    V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), (viewwindowy >> crispy->hires) + (viewheight >> crispy->hires),
                cache_lump_name<patch_t *>("bordbr", PU_CACHE));
    V_DrawPatch((viewwindowx >> crispy->hires) - 4, (viewwindowy >> crispy->hires) + (viewheight >> crispy->hires),
                cache_lump_name<patch_t *>("bordbl", PU_CACHE));
}

/*
==================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows
==================
*/

boolean BorderTopRefresh;

void R_DrawTopBorder()
{
    byte *src, *dest;
    int x, y;

    if (scaledviewwidth == SCREENWIDTH)
        return;

/*	if(gamemode == shareware)
	{
		src = W_CacheLumpName ("FLOOR04", PU_CACHE);
	}
	else
	{
		src = W_CacheLumpName ("FLAT513", PU_CACHE);
	}
*/
    src = cache_lump_name<byte *>("F_022", PU_CACHE);
    dest = I_VideoBuffer;

    for (y = 0; y < 34; y++)
    {
        for (x = 0; x < SCREENWIDTH / 64; x++)
        {
            memcpy(dest, src + ((y & 63) << 6), 64);
            dest += 64;
        }
        if (SCREENWIDTH & 63)
        {
            memcpy(dest, src + ((y & 63) << 6), SCREENWIDTH & 63);
            dest += (SCREENWIDTH & 63);
        }
    }
    if (viewwindowy < 35)
    {
        for (x = (viewwindowx >> crispy->hires); x < (viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires); x += 16)
        {
            V_DrawPatch(x, (viewwindowy >> crispy->hires) - 4,
                        cache_lump_name<patch_t *>("bordt", PU_CACHE));
        }
        V_DrawPatch((viewwindowx >> crispy->hires) - 4, (viewwindowy >> crispy->hires), cache_lump_name<patch_t *>("bordl",
                                                                  PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), (viewwindowy >> crispy->hires),
                    cache_lump_name<patch_t *>("bordr", PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) - 4, (viewwindowy >> crispy->hires) + 16,
                    cache_lump_name<patch_t *>("bordl", PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), (viewwindowy >> crispy->hires) + 16,
                    cache_lump_name<patch_t *>("bordr", PU_CACHE));

        V_DrawPatch((viewwindowx >> crispy->hires) - 4, (viewwindowy >> crispy->hires) - 4,
                    cache_lump_name<patch_t *>("bordtl", PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) + (viewwidth >> crispy->hires), (viewwindowy >> crispy->hires) - 4,
                    cache_lump_name<patch_t *>("bordtr", PU_CACHE));
    }
}