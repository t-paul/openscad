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

#include <fontconfig/fontconfig.h>

#include "printutils.h"
#include "FreetypeRenderer.h"

#include FT_OUTLINE_H

FreetypeRenderer::FreetypeRenderer()
{
	funcs.move_to = outline_move_to_func;
	funcs.line_to = outline_line_to_func;
	funcs.conic_to = outline_conic_to_func;
	funcs.cubic_to = outline_cubic_to_func;
	funcs.delta = 0;
	funcs.shift = 0;
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

void FreetypeRenderer::render(DrawingCallback *callback, std::string text, std::string font, double size, double spacing, std::string direction, std::string language, std::string script) const
{
	FT_Face face;
	FT_Error error;
	
	FontCache *cache = FontCache::instance();
	if (!cache->is_init_ok()) {
		return;
	}

	face = cache->get_font(font);
	if (face == NULL) {
		return;
	}
	
	error = FT_Set_Char_Size(face, 0, size * scale, 100, 100);
	if (error) {
		PRINTB("Can't set font size for font %s", font);
		return;
	}
	
	hb_font_t *hb_ft_font = hb_ft_font_create(face, NULL);

	hb_buffer_t *hb_buf = hb_buffer_create();
	hb_buffer_set_direction(hb_buf, hb_direction_from_string(direction.c_str(), -1));
	hb_buffer_set_script(hb_buf, hb_script_from_string(script.c_str(), -1));
	hb_buffer_set_language(hb_buf, hb_language_from_string(language.c_str(), -1));
	hb_buffer_add_utf8(hb_buf, text.c_str(), strlen(text.c_str()), 0, strlen(text.c_str()));
	hb_shape(hb_ft_font, hb_buf, NULL, 0);
	
	unsigned int glyph_count;
        hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buf, &glyph_count);
        hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buf, &glyph_count);	
	
	GlyphArray glyph_array;
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
		const GlyphData *glyph_data = new GlyphData(glyph, &glyph_info[idx], &glyph_pos[idx]);
		glyph_array.push_back(glyph_data);
	}

	for (GlyphArray::iterator it = glyph_array.begin();it != glyph_array.end();it++) {
		const GlyphData *glyph = (*it);
		
		callback->start_glyph();
		callback->set_glyph_offset(glyph->get_x_offset(), glyph->get_y_offset());
		FT_Outline outline = reinterpret_cast<FT_OutlineGlyph>(glyph->get_glyph())->outline;
		FT_Outline_Decompose(&outline, &funcs, callback);

		double adv_x  = glyph->get_x_advance() * spacing;
		double adv_y  = glyph->get_y_advance() * spacing;
		callback->add_glyph_advance(adv_x, adv_y);
		callback->finish_glyph();
	}

	hb_buffer_destroy(hb_buf);
        hb_font_destroy(hb_ft_font);
}
