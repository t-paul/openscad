#ifndef LIBSVG_POLYLINE_H
#define	LIBSVG_POLYLINE_H

#include "shape.h"

namespace libsvg {

class polyline : public shape {
private:

public:
    polyline();
    polyline(const polyline& orig);
    virtual ~polyline();

    const std::string& get_name() const { return polyline::name; };
    
    static const std::string name;
};

}

#endif	/* LIBSVG_POLYLINE_H */

