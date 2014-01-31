#include <boost/tokenizer.hpp>

#include "polygon.h"

namespace libsvg {

const std::string polygon::name("polygon"); 

polygon::polygon()
{
}

polygon::polygon(const polygon& orig)
{
}

polygon::~polygon()
{
}

void
polygon::set_attrs(attr_map_t& attrs)
{
	shape::set_attrs(attrs);
	this->points = attrs["points"];
	
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ,");
	tokenizer tokens(this->points, sep);

	double x;
	path_t path;
	bool first = true;
	for (tokenizer::iterator it = tokens.begin();it != tokens.end();++it) {
		std::string v = (*it);
		double p = atof(v.c_str());
		
		if (first) {
			x = p;
		} else {
			path.push_back(Eigen::Vector3d(x, p, 0));
		}
		first = !first;
	}
	path_list.push_back(path);
}

}