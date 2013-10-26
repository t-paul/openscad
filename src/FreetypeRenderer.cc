/* 
 * File:   FreetypeRenderer.cpp
 * Author: tp
 * 
 * Created on October 21, 2013, 10:43 PM
 */
#include <stdio.h>

#include <iostream>

#include "FreetypeRenderer.h"

#include FT_GLYPH_H
#include FT_OUTLINE_H

FreetypeRenderer::FreetypeRenderer()
{
	const FT_Error error = FT_Init_FreeType(&library);
	if (error) {
		printf("can't initialize freetype.\n");
	}
}

FreetypeRenderer::~FreetypeRenderer()
{
}

int
FreetypeRenderer::Outline_MoveToFunc(const FT_Vector *to, void *user)
{
	std::cout << "Outline_MoveToFunc: " << to->x << ", " << to->y << std::endl;
	
	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->move_to(point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_LineToFunc(const FT_Vector *to, void *user)
{
	std::cout << "Outline_LineToFunc: " << to->x << ", " << to->y << std::endl;

	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->line_to(point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_ConicToFunc(const FT_Vector *c1, const FT_Vector *to, void *user)
{
	std::cout << "Outline_ConicToFunc: " << to->x << ", " << to->y << std::endl;

	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->curve_to(point_t(c1->x, c1->y, scale), point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_CubicToFunc(const FT_Vector *c1, const FT_Vector *c2, const FT_Vector *to, void *user)
{
	std::cout << "Outline_CubicToFunc: " << to->x << ", " << to->y << std::endl;

	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->curve_to(point_t(c1->x, c1->y, scale), point_t(c2->x, c2->y, scale), point_t(to->x, to->y, scale));
	return 0; 
}

void
FreetypeRenderer::render(DrawingCallback *callback, std::string text, std::string font, double size) const
{
	FT_Face face;
	FT_Error error;
	
	std::string path = font.empty() ? "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf" : font;
	error = FT_New_Face(library, path.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		printf("unknown file format\n");
		return;
	} else if (error) {
		printf("error opening font face\n");
		return;
	}

	printf("num_glyphs: %ld\n", face->num_glyphs);
	
	error = FT_Set_Char_Size(face, 0, size * scale, 96, 96);
	if (error) {
		printf("can't set font size\n");
		return;
	}
	
	bool use_kerning = FT_HAS_KERNING(face);
	
	double x_offset = 0;
	callback->set_xoffset(x_offset);
	FT_UInt prev_glyph_index = 0;
	for (unsigned int idx = 0;idx < text.length();idx++) {
		int c = text.at(idx);
		FT_UInt glyph_index = FT_Get_Char_Index(face, c);
		printf("glyph_index: %u\n", glyph_index);

		if (use_kerning && (idx > 0)) {
			FT_Vector delta;
			FT_Get_Kerning(face, prev_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta );
			std::cout << "kerning delta: x = " << delta.x << ", y = " << delta.y << std::endl;
		}
		prev_glyph_index = glyph_index;

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error) {
			printf("could not load glyph %u\n", glyph_index);
			continue;
		}

		FT_Glyph glyph;
		error = FT_Get_Glyph(face->glyph, &glyph);
		if (error) {
			printf("could not get glyph %u\n", glyph_index);
			continue;
		}

		FT_BBox bbox;
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_UNSCALED, &bbox);
		int width = bbox.xMax - bbox.xMin;
		int height = bbox.yMax - bbox.yMin;
		printf("width = %d, height = %d\n", width, height);

		FT_Outline_Funcs funcs;
		funcs.move_to = Outline_MoveToFunc;
		funcs.line_to = Outline_LineToFunc;
		funcs.conic_to = Outline_ConicToFunc;
		funcs.cubic_to = Outline_CubicToFunc;
		funcs.delta = 0;
		funcs.shift = 0;

		FT_Outline outline = reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
		FT_Outline_Decompose(&outline, &funcs, callback);
		callback->stroke();

		double adv  = glyph->advance.x / scale / 64.0 / 16.0;
		printf("advance.x = %ld, advance.y = %ld, adv = %.2f\n", glyph->advance.x, glyph->advance.y, adv);
		callback->add_xoffset(adv);
		
		FT_Done_Glyph(glyph);
	}
}
