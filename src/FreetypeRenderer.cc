/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <stdio.h>

#include <iostream>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include "printutils.h"
#include "FreetypeRenderer.h"

#include FT_GLYPH_H
#include FT_OUTLINE_H

namespace fs = boost::filesystem;

FreetypeRenderer::FreetypeRenderer()
{
	init_ok = false;
	const FT_Error error = FT_Init_FreeType(&library);
	if (error) {
		PRINT("Can't initialize freetype library, text() objects will not be rendered");
	} else {
		init_ok = true;
	}
}

FreetypeRenderer::~FreetypeRenderer()
{
}

int
FreetypeRenderer::Outline_MoveToFunc(const FT_Vector *to, void *user)
{
	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->move_to(point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_LineToFunc(const FT_Vector *to, void *user)
{
	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->line_to(point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_ConicToFunc(const FT_Vector *c1, const FT_Vector *to, void *user)
{
	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->curve_to(point_t(c1->x, c1->y, scale), point_t(to->x, to->y, scale));
	return 0;
}

int
FreetypeRenderer::Outline_CubicToFunc(const FT_Vector *c1, const FT_Vector *c2, const FT_Vector *to, void *user)
{
	const DrawingCallback *cb = reinterpret_cast<const DrawingCallback *>(user);
	
	cb->curve_to(point_t(c1->x, c1->y, scale), point_t(c2->x, c2->y, scale), point_t(to->x, to->y, scale));
	return 0; 
}

FT_Face
FreetypeRenderer::find_face(std::string font) const
{
	const char *env_font_path = getenv("OPENSCAD_FONT_PATH");
	const std::string path = (env_font_path == NULL) ? "/usr/share/fonts/truetype" : env_font_path;
	
	return find_face_in_path(path, font);
}

FT_Face
FreetypeRenderer::find_face_in_path(std::string path, std::string font) const
{
	FT_Error error;
	
	if (!fs::is_directory(path)) {
		PRINTB("Font path '%s' does not exist or is not a directory.", path);
	}

	for (fs::recursive_directory_iterator it(path);it != fs::recursive_directory_iterator();it++) {
		fs::directory_entry entry = (*it);
		if (fs::is_regular(entry.path())) {
			FT_Face face;
			error = FT_New_Face(library, entry.path().c_str(), 0, &face);
			if (error) {
				continue;
			}
			const char *name = FT_Get_Postscript_Name(face);
			if (font == name) {
				return face;
			}
			FT_Done_Face(face);
		}
	}
	return NULL;
}

void
FreetypeRenderer::render(DrawingCallback *callback, std::string text, std::string font, double size) const
{
	FT_Face face;
	FT_Error error;

	if (!init_ok) {
		return;
	}
	
	face = find_face(font);
	if (face == NULL) {
		return;
	}
	
	error = FT_Set_Char_Size(face, 0, size * scale, 96, 96);
	if (error) {
		PRINTB("Can't set font size for font %s", font);
		return;
	}
	
	bool use_kerning = FT_HAS_KERNING(face);
	
	double x_offset = 0;
	callback->set_xoffset(x_offset);
	FT_UInt prev_glyph_index = 0;
	
	for (unsigned int idx = 0;idx < text.length();idx++) {
		int c = text.at(idx);
		FT_UInt glyph_index = FT_Get_Char_Index(face, c);

		if (use_kerning && (idx > 0)) {
			FT_Vector delta;
			FT_Get_Kerning(face, prev_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta );
		}
		prev_glyph_index = glyph_index;

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error) {
			PRINTB("Could not load glyph %u for char '%c'", glyph_index % c);
			continue;
		}

		FT_Glyph glyph;
		error = FT_Get_Glyph(face->glyph, &glyph);
		if (error) {
			PRINTB("Could not load glyph %u for char '%c'", glyph_index % c);
			continue;
		}

		FT_Outline_Funcs funcs;
		funcs.move_to = Outline_MoveToFunc;
		funcs.line_to = Outline_LineToFunc;
		funcs.conic_to = Outline_ConicToFunc;
		funcs.cubic_to = Outline_CubicToFunc;
		funcs.delta = 0;
		funcs.shift = 0;

		FT_Outline outline = reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
		FT_Outline_Decompose(&outline, &funcs, callback);

		double adv  = glyph->advance.x / scale / 64.0 / 16.0;
		callback->add_xoffset(adv);
		
		FT_Done_Glyph(glyph);
	}
}
