#include <stdio.h>
#include <string>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "shape.h"
#include "circle.h"
#include "ellipse.h"
#include "line.h"
#include "polygon.h"
#include "polyline.h"
#include "rect.h"
#include "svgpage.h"
#include "path.h"
#include "group.h"

#include "transformation.h"

namespace libsvg {

shape::shape() : parent(NULL), x(0), y(0)
{
}

shape::shape(const shape& orig)
{
}

shape::~shape()
{
}

shape *
shape::create_from_name(const char *name)
{
	if (circle::name == name) {
		return new circle();
	} else if (ellipse::name == name) {
		return new ellipse();
	} else if (line::name == name) {
		return new line();
	} else if (polygon::name == name) {
		return new polygon();
	} else if (polyline::name == name) {
		return new polyline();
	} else if (rect::name == name) {
		return new rect();
	} else if (svgpage::name == name) {
		return new svgpage();
	} else if (path::name == name) {
		return new path();
	} else if (group::name == name) {
		return new group();
	} else {
		return NULL;
	}
}

void
shape::set_attrs(attr_map_t& attrs)
{
	this->id = attrs["id"];
	this->transform = attrs["transform"];
}

void
shape::collect_transform_matrices(std::vector<Eigen::Matrix3d>& matrices, shape *s)
{
	std::string transform_arg(s->transform);
	
	boost::replace_all(transform_arg, "matrix", "m");
	boost::replace_all(transform_arg, "translate", "t");
	boost::replace_all(transform_arg, "scale", "s");
	boost::replace_all(transform_arg, "rotate", "r");
	boost::replace_all(transform_arg, "skewX", "x");
	boost::replace_all(transform_arg, "skewY", "y");

	std::string commands = "mtsrxy";
	
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ,()", commands.c_str());
	tokenizer tokens(transform_arg, sep);

	transformation *t = NULL;
	for (tokenizer::iterator it = tokens.begin();it != tokens.end();++it) {
		std::string v = (*it);
		if ((v.length() == 1) && (commands.find(v) != std::string::npos)) {
			if (t) {
				std::vector<Eigen::Matrix3d> m = t->get_matrices();
				matrices.insert(matrices.end(), m.begin(), m.end());
				delete t;
				t = NULL;
			}
			switch (v[0]) {
			case 'm':
				t = new matrix();
				break;
			case 't':
				t = new translate();
				break;
			case 's':
				t = new scale();
				break;
			case 'r':
				t = new rotate();
				break;
			case 'x':
				t = new skew_x();
				break;
			case 'y':
				t = new skew_y();
				break;
			default:
				std::cout << "unknown transform op " << v << std::endl;
				t = NULL;
			}
		} else {
			if (t) {
				t->add_arg(v);
			}
		}
	}
	if (t) {
		std::vector<Eigen::Matrix3d> m = t->get_matrices();
		matrices.insert(matrices.end(), m.begin(), m.end());
		delete t;
	}
	
	std::cout << "matrices: " << std::endl;
	for (std::vector<Eigen::Matrix3d>::iterator it = matrices.begin();it != matrices.end();it++) {
		std::cout << *it << std::endl << "---" << std::endl;
	}
	std::cout << "===" << std::endl;
}

void
shape::apply_transform()
{
	std::vector<Eigen::Matrix3d> matrices;
	for (shape *s = this;s->get_parent() != NULL;s = s->get_parent()) {
		collect_transform_matrices(matrices, s);
	}

	path_list_t result_list;
	for (path_list_t::iterator it = path_list.begin();it != path_list.end();it++) {
		path_t& p = *it;
		
		result_list.push_back(path_t());
		for (path_t::iterator it2 = p.begin();it2 != p.end();it2++) {
			Eigen::Vector3d result((*it2).x(), (*it2).y(), 1);
			for (std::vector<Eigen::Matrix3d>::reverse_iterator it3 = matrices.rbegin();it3 != matrices.rend();it3++) {
				result = *it3 * result;
			}
			
			result_list.back().push_back(result);
		}
	}
	path_list = result_list;
}

std::ostream & operator<<(std::ostream &os, const shape& s)
{
    return os << s.get_name() << " | id = '" << s.id << "', transform = '" << s.transform << "'";
}

}