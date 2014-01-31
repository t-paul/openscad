#ifndef LIBSVG_LINE_H
#define	LIBSVG_LINE_H

#include "shape.h"

namespace libsvg {

class line : public shape {
private:

public:
    line();
    line(const line& orig);
    virtual ~line();

    const std::string& get_name() const { return line::name; };
    
    static const std::string name;
};

}

#endif	/* LIBSVG_LINE_H */

