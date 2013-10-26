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
    void stroke() const;
private:
    DxfData *data;
    double fn;
    double x_offset;
    
    inline double t(double t, int exp) const {
	return pow(1.0 - t, exp);
    }
};

#endif	/* DRAWINGCALLBACK_H */

