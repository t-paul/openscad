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
#ifndef DRAWINGCALLBACK_H
#define	DRAWINGCALLBACK_H

#include <math.h>

#include "dxfdata.h"

class point_t {
public:
    double x, y;

    point_t(const double x, const double y) {
        this->x = x;
        this->y = y;
    }

    point_t(const double x, const double y, const double div) {
        this->x = x / div;
        this->y = y / div;
    }
};

class DrawingCallback {
public:
    DrawingCallback(DxfData *data, double fn);
    virtual ~DrawingCallback();
    
    void set_xoffset(double x_offset);
    void add_xoffset(double x_offset);
    void move_to(point_t to) const;
    void line_to(point_t to) const;
    void curve_to(point_t c1, point_t to) const;
    void curve_to(point_t c1, point_t c2, point_t to) const;
private:
    DxfData *data;
    double fn;
    double x_offset;
    
    inline double t(double t, int exp) const {
	return pow(1.0 - t, exp);
    }
};

#endif	/* DRAWINGCALLBACK_H */

