#ifndef LIBSVG_PATH_H
#define LIBSVG_PATH_H

#include <math.h>

#include <vector>

#include "shape.h"

namespace libsvg {

class path : public shape {
protected:
    std::string data;

private:
    inline double t(double t, int exp) const {
	return pow(1.0 - t, exp);
    }

    void arc_to(path_t& path, double x, double y, double rx, double ry, double x2, double y2, double angle, bool large, bool sweep);
    void curve_to(path_t& path, double x, double y, double cx1, double cy1, double cx2, double cy2, double x2, double y2);

public:
    path();
    path(const path& orig);
    virtual ~path();

    virtual void set_attrs(attr_map_t& attrs);
    virtual void dump();
    const std::string& get_name() const { return path::name; };
    
    static const std::string name;
};

}

#endif	/* LIBSVG_PATH_H */

