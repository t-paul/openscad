#include <stdlib.h>
#include <iostream>

#include "svgpage.h"

namespace libsvg {

const std::string svgpage::name("svg"); 

svgpage::svgpage()
{
}

svgpage::svgpage(const svgpage& orig)
{
}

svgpage::~svgpage()
{
}

void
svgpage::set_attrs(attr_map_t& attrs)
{
	this->x = 0;
	this->y = 0;
	this->width = atof(attrs["width"].c_str());
	this->height = atof(attrs["height"].c_str());
}

void
svgpage::dump()
{
	std::cout << get_name()
		<< ": x = " << this->x
		<< ": y = " << this->y
		<< ": width = " << this->width
		<< ": height = " << this->height
		<< std::endl;
}

}