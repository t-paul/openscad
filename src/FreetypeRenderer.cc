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
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "boosty.h"
#include "printutils.h"
#include "FreetypeRenderer.h"

#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <hb.h>
#include <hb-ft.h>

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

int FreetypeRenderer::outline_move_to_func(const FT_Vector *to, void *user)
{
	DrawingCallback *cb = reinterpret_cast<DrawingCallback *>(user);
	
	cb->move_to(get_scaled_vector(to, scale));
	return 0;
}

int FreetypeRenderer::outline_line_to_func(const FT_Vector *to, void *user)
{
	DrawingCallback *cb = reinterpret_cast<DrawingCallback *>(user);
	
	cb->line_to(get_scaled_vector(to, scale));
	return 0;
}

int FreetypeRenderer::outline_conic_to_func(const FT_Vector *c1, const FT_Vector *to, void *user)
{
	DrawingCallback *cb = reinterpret_cast<DrawingCallback *>(user);

	cb->curve_to(get_scaled_vector(c1, scale), get_scaled_vector(to, scale));
	return 0;
}

int FreetypeRenderer::outline_cubic_to_func(const FT_Vector *c1, const FT_Vector *c2, const FT_Vector *to, void *user)
{
	DrawingCallback *cb = reinterpret_cast<DrawingCallback *>(user);
	
	cb->curve_to(get_scaled_vector(c1, scale), get_scaled_vector(c2, scale), get_scaled_vector(to, scale));
	return 0; 
}

FT_Face FreetypeRenderer::find_face(std::string font) const
{
	const char *env_font_path = getenv("OPENSCAD_FONT_PATH");
	
	std::string paths = (env_font_path == NULL) ? "/usr/share/fonts/truetype" : env_font_path;
	typedef boost::split_iterator<std::string::iterator> string_split_iterator;
	for (string_split_iterator it = make_split_iterator(paths, first_finder(":", boost::is_iequal()));it != string_split_iterator();it++) {
		std::string path = boosty::absolute(fs::path(boost::copy_range<std::string>(*it))).string();
		FT_Face face = find_face_in_path(path, font);
		if (face) {
			return face;
		}
	}
	return NULL;
}

FT_Face FreetypeRenderer::find_face_in_path(std::string path, std::string font) const
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

void FreetypeRenderer::render(DrawingCallback *callback, std::string text, std::string font, double size, std::string direction, std::string language, std::string script) const
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
	
	hb_font_t *hb_ft_font = hb_ft_font_create(face, NULL);
	
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_set_direction(buf, hb_direction_from_string(direction.c_str(), -1));
	hb_buffer_set_script(buf, hb_script_from_string(script.c_str(), -1));
	hb_buffer_set_language(buf, hb_language_from_string(language.c_str(), -1));
	hb_buffer_add_utf8(buf, text.c_str(), strlen(text.c_str()), 0, strlen(text.c_str()));
	hb_shape(hb_ft_font, buf, NULL, 0);
	
	unsigned int glyph_count;
        hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);	

	for (unsigned int idx = 0;idx < glyph_count;idx++) {
		FT_UInt glyph_index = glyph_info[idx].codepoint;
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error) {
			PRINTB("Could not load glyph %u for char at index %u in text '%s'", glyph_index % idx % text);
			continue;
		}

		FT_Glyph glyph;
		error = FT_Get_Glyph(face->glyph, &glyph);
		if (error) {
			PRINTB("Could not get glyph %u for char at index %u in text '%s'", glyph_index % idx % text);
			continue;
		}

		FT_Outline_Funcs funcs;
		funcs.move_to = outline_move_to_func;
		funcs.line_to = outline_line_to_func;
		funcs.conic_to = outline_conic_to_func;
		funcs.cubic_to = outline_cubic_to_func;
		funcs.delta = 0;
		funcs.shift = 0;

		callback->start_glyph();
		callback->set_glyph_offset(glyph_pos[idx].x_offset / 64.0 / 16.0, glyph_pos[idx].y_offset / 64.0 / 16.0);
		FT_Outline outline = reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
		FT_Outline_Decompose(&outline, &funcs, callback);

		double adv_x  = glyph_pos[idx].x_advance / 64.0 / 16.0;
		double adv_y  = glyph_pos[idx].y_advance / 64.0 / 16.0;
		callback->add_glyph_advance(adv_x, adv_y);
		callback->finish_glyph();
		
		FT_Done_Glyph(glyph);
	}
	
	hb_buffer_clear_contents(buf);
	hb_buffer_destroy(buf);
        hb_font_destroy(hb_ft_font);
}
