#ifndef LIBSVG_CIRCLE_H
#define	LIBSVG_CIRCLE_H

#include "shape.h"

namespace libsvg {

class circle : public shape {
private:
    
public:
    circle();
    circle(const circle& orig);
    virtual ~circle();

    const std::string& get_name() const { return circle::name; };

    static const std::string name;
};

}

#endif	/* LIBSVG_CIRCLE_H */

