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
#ifndef FREETYPERENDERER_H
#define	FREETYPERENDERER_H

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "DrawingCallback.h"

class FreetypeRenderer {
public:
    FreetypeRenderer();
    virtual ~FreetypeRenderer();

    void render(DrawingCallback *cb, std::string text, std::string font, double size, std::string direction, std::string language, std::string script) const;
private:
    const static double scale = 1000;
    
    bool init_ok;
    FT_Library library;
    
    FT_Face find_face(std::string font) const;
    FT_Face find_face_in_path(std::string path, std::string font) const;
    
    static int outline_move_to_func(const FT_Vector *to, void *user);
    static int outline_line_to_func(const FT_Vector *to, void *user);
    static int outline_conic_to_func(const FT_Vector *c1, const FT_Vector *to, void *user);
    static int outline_cubic_to_func(const FT_Vector *c1, const FT_Vector *c2, const FT_Vector *to, void *user);
    
    static inline Vector2d get_scaled_vector(const FT_Vector *ft_vector, double scale) {
        return Vector2d(ft_vector->x / scale, ft_vector->y / scale);
    }
};

#endif	/* FREETYPERENDERER_H */

