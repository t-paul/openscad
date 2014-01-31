/* 
 * File:   ellipse.cpp
 * Author: tp
 * 
 * Created on January 25, 2014, 12:50 AM
 */

#include <stdlib.h>
#include <iostream>

#include "ellipse.h"

namespace libsvg {

const std::string ellipse::name("ellipse"); 

ellipse::ellipse()
{
}

ellipse::ellipse(const ellipse& orig)
{
}

ellipse::~ellipse()
{
}

void
ellipse::set_attrs(attr_map_t& attrs)
{
	this->x = atof(attrs["cx"].c_str());
	this->y = atof(attrs["cy"].c_str());
	this->rx = atof(attrs["rx"].c_str());
	this->ry = atof(attrs["ry"].c_str());
}

void
ellipse::dump()
{
	std::cout << get_name()
		<< ": x = " << this->x
		<< ": y = " << this->y
		<< ": rx = " << this->rx
		<< ": ry = " << this->ry
		<< std::endl;
}

}