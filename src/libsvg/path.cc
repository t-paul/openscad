#include <stdlib.h>

#include <string>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "polyclipping/clipper.hpp"

#include "path.h"

namespace libsvg {

const std::string path::name("path"); 

/*
PATHSEG_CLOSEPATH z
PATHSEG_MOVETO_ABS M
PATHSEG_MOVETO_REL m
PATHSEG_LINETO_ABS L
PATHSEG_LINETO_REL l
PATHSEG_CURVETO_CUBIC_ABS C
PATHSEG_CURVETO_CUBIC_REL c
PATHSEG_CURVETO_QUADRATIC_ABS Q
PATHSEG_CURVETO_QUADRATIC_REL q
PATHSEG_ARC_ABS A
PATHSEG_ARC_REL a
PATHSEG_LINETO_HORIZONTAL_ABS H
PATHSEG_LINETO_HORIZONTAL_REL h
PATHSEG_LINETO_VERTICAL_ABS V
PATHSEG_LINETO_VERTICAL_REL v
PATHSEG_CURVETO_CUBIC_SMOOTH_ABS S
PATHSEG_CURVETO_CUBIC_SMOOTH_REL s
PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS T
PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL t
*/

path::path()
{
}

path::path(const path& orig)
{
}

path::~path()
{
}

void
path::arc_to(path_t& path, double x, double y, double rx, double ry, double x2, double y2, double angle, bool large, bool sweep)
{
	std::cout
		<< "path: id = " << this->id
		<< ", x = " << x
		<< ", y = " << y
		<< ", rx = " << rx
		<< ", ry = " << ry
		<< ", x2 = " << x2
		<< ", y2 = " << y2
		<< ", angle = " << angle
		<< ", large = " << large
		<< ", sweep = " << sweep
		<< std::endl;
        double cosr = cos(M_PI * angle / 180);
        double sinr = sin(M_PI * angle / 180);
        double dx = (x - x2) / 2;
        double dy = (y - y2) / 2;
        double x1prim = cosr * dx + sinr * dy;
        double x1prim_sq = x1prim * x1prim;
        double y1prim = -sinr * dx + cosr * dy;
        double y1prim_sq = y1prim * y1prim;

        double rx_sq = rx * rx;
        double ry_sq = ry * ry;

        double radius_check = (x1prim_sq / rx_sq) + (y1prim_sq / ry_sq);
        if (radius_check > 1) {
            rx *= sqrt(radius_check);
            ry *= sqrt(radius_check);
            rx_sq = rx * rx;
            ry_sq = ry * ry;
	}

        double t1 = rx_sq * y1prim_sq;
        double t2 = ry_sq * x1prim_sq;
        double c = sqrt(abs((rx_sq * ry_sq - t1 - t2) / (t1 + t2)));
        
        if (large == sweep) {
            c = -c;
	}
        double cxprim = c * rx * y1prim / ry;
        double cyprim = -c * ry * x1prim / rx;

	double centerx = (cosr * cxprim - sinr * cyprim) + ((x + x2) / 2);
	double centery = (sinr * cxprim + cosr * cyprim) + ((y + y2) / 2);

        double ux = (x1prim - cxprim) / rx;
        double uy = (y1prim - cyprim) / ry;
        double vx = (-x1prim - cxprim) / rx;
        double vy = (-y1prim - cyprim) / ry;
        double n1 = sqrt(ux * ux + uy * uy);
        double theta = acos(ux / n1);
	if (uy < 0) {
            theta = -theta;
	}
        theta = fmod(theta, 2 * M_PI);

        double n2 = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
        double p = ux * vx + uy * vy;
        double delta;
	if (p == 0) {
            delta = acos(0);
        } else {
            delta = acos(p / n2);
	}

        if ((ux * vy - uy * vx) < 0) {
            delta = -delta;
	}
        delta = fmod(delta, 2 * M_PI);
	if (!sweep) {
            delta -= 2 * M_PI;
	}
	
	std::cout 
		<< "cx = " << centerx
		<< ", cy = " << centery
		<< ", theta = " << (theta * 180 / M_PI)
		<< ", delta = " << (delta * 180 / M_PI) << std::endl;

	int steps = 10;
	for (int a = 0;a <= steps;a++) {
	        double phi = theta + delta * a / steps;

		double xx = cosr * cos(phi) * rx - sinr * sin(phi) * ry;
		double yy = sinr * cos(phi) * rx + cosr * sin(phi) * ry;
		
		path.push_back(Eigen::Vector3d(xx + centerx, yy + centery, 0));
	}	
}

void
path::curve_to(path_t& path, double x, double y, double cx1, double cy1, double cx2, double cy2, double x2, double y2)
{
	unsigned long fn = 20;
	for (unsigned long idx = 1;idx <= fn;idx++) {
		const double a = idx * (1.0 / (double)fn);
		const double xx = x * t(a, 3) + cx1 * 3 * t(a, 2) * a + cx2 * 3 * t(a, 1) * a * a + x2 * a * a * a;
		const double yy = y * t(a, 3) + cy1 * 3 * t(a, 2) * a + cy2 * 3 * t(a, 1) * a * a + y2 * a * a * a;
		path.push_back(Eigen::Vector3d(xx, yy, 0));
	}
}

void
path::set_attrs(attr_map_t& attrs)
{
	std::string commands = "zmlcqahvstZMLCQAHVST";
	
	shape::set_attrs(attrs);
	this->data = attrs["d"];

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ,", commands.c_str());
	tokenizer tokens(this->data, sep);

	double x = 0;
	double y = 0;
	double xx = 0;
	double yy = 0;
	double rx = 0;
	double ry = 0;
	double cx1 = 0;
	double cy1 = 0;
	double cx2 = 0;
	double cy2 = 0;
	double angle = 0;
	bool large = false;
	bool sweep = false;
	char cmd = ' ';
	int point = 0;
	
	bool path_closed = false;
	path_list.push_back(path_t());
	for (tokenizer::iterator it = tokens.begin();it != tokens.end();++it) {
		std::string v = (*it);

		double p = 0;
		if ((v.length() == 1) && (commands.find(v) != std::string::npos)) {
			point = -1;
			cmd = v[0];
		} else {
			p = atof(v.c_str());
		}	
		
		switch (cmd) {
		case 'a':
		case 'A':
			//(rx ry x-axis-rotation large-arc-flag sweep-flag x y)
			switch (point) {
			case 0:
				rx = abs(p);
				break;
			case 1:
				ry = abs(p);
				//std::cout << cmd << " [1] (" << rx << ", " << ry << ")" << std::endl;
				break;
			case 2:
				angle = p;
				//std::cout << cmd << " [2] (" << angle << ")" << std::endl;
				break;
			case 3:
				large = p > 0.5;
				break;
			case 4:
				sweep = p > 0.5;
				break;
			case 5:
				xx = cmd == 'a' ? x + p : p;
				break;
			case 6:
				yy = cmd == 'a' ? y + p : p;
				//std::cout << cmd << " [3] (" << xx << ", " << yy << ")" << std::endl;
				arc_to(path_list.back(), x, y, rx, ry, xx, yy, angle, large, sweep);
				x = xx;
				y = yy;
				point = -1;
				break;
			}
			break;
		case 'l':
		case 'L':
			switch (point) {
			case 0:
				xx = cmd == 'l' ? x + p : p;
				break;
			case 1:
				yy = cmd == 'l' ? y + p : p;
				//std::cout << cmd << " [1] (" << xx << ", " << yy << ")" << std::endl;
				path_list.back().push_back(Eigen::Vector3d(xx, yy, 0));
				x = xx;
				y = yy;
				point = -1;
				break;
			}
			break;
		case 'c':
		case 'C':
			switch (point) {
			case 0:
				cx1 = p;
				break;
			case 1:
				cy1 = p;
				//std::cout << cmd << " [1] (" << cx1 << ", " << cy1 << ")" << std::endl;
				break;
			case 2:
				cx2 = p;
				break;
			case 3:
				cy2 = p;
				//std::cout << cmd << " [2] (" << cx2 << ", " << cy2 << ")" << std::endl;
				break;
			case 4:
				xx = cmd == 'c' ? x + p : p;
				break;
			case 5:
				yy = cmd == 'c' ? y + p : p;
				cx1 = cmd == 'c' ? x + cx1 : cx1;
				cy1 = cmd == 'c' ? y + cy1 : cy1;
				cx2 = cmd == 'c' ? x + cx2 : cx2;
				cy2 = cmd == 'c' ? y + cy2 : cy2;
				//std::cout << cmd << " [3] (" << xx << ", " << yy << ")" << std::endl;
				curve_to(path_list.back(), x, y, cx1, cy1, cx2, cy2, xx, yy);
				x = xx;
				y = yy;
				point = -1;
				break;
			}
			break;
		case 'm':
		case 'M':
			switch (point) {
			case 0:
				xx = cmd == 'm' ? x + p : p;
				break;
			case 1:
				yy = cmd == 'm' ? y + p : p;
				//std::cout << cmd << " (" << xx << ", " << yy << ")" << std::endl;
				cmd = cmd == 'm' ? 'l' : 'L';
				if (!path_list.back().empty()) {
					path_list.push_back(path_t());
					std::cout << "not closed!" << std::endl;
				}
				path_list.back().push_back(Eigen::Vector3d(xx, yy, 0));
				x = xx;
				y = yy;
				point = -1;
			}
			break;
		case 'v':
		case 'V':
			switch (point) {
			case 0:
				y = cmd == 'v' ? y + p : p;
				path_list.back().push_back(Eigen::Vector3d(x, y, 0));
				point = -1;
				break;
			}
			break;
		case 'h':
		case 'H':
			switch (point) {
			case 0:
				x = cmd == 'h' ? x + p : p;
				path_list.back().push_back(Eigen::Vector3d(x, y, 0));
				point = -1;
				break;
			}
			break;
		case 'z':
		case 'Z':
			if (!path_list.back().empty()) {
				path_list.back().push_back(path_list.back()[0]);
			}
			path_list.push_back(path_t());
			path_closed = true;
			break;
		}
		
		point++;
	}
	
	while (!path_list.empty() && path_list.back().empty()) {
		path_list.pop_back();
	}
	
	if (!path_closed) {
		path_t path = path_list.back();
		path_list.pop_back();
		std::cout << "not closed!" << std::endl;
		ClipperLib::Path line;
		ClipperLib::Paths result;
		for (path_t::iterator it = path.begin();it != path.end();it++) {
			Eigen::Vector3d& v = *it;
			line << ClipperLib::IntPoint(v.x() * 10000, v.y() * 10000);
		}
		
		ClipperLib::ClipperOffset co;
		co.AddPath(line, ClipperLib::jtMiter, ClipperLib::etOpenSquare);
		co.Execute(result, 1000);
		
		for (ClipperLib::Paths::iterator it = result.begin();it != result.end();it++) {
			ClipperLib::Path& p = *it;
			std::cout << "new path, length = " << p.size() << std::endl;
			path_list.push_back(path_t());
			for (ClipperLib::Path::iterator it2 = p.begin();it2 != p.end();it2++) {
				ClipperLib::IntPoint& point = *it2;
				std::cout << point.X << ", " << point.X << std::endl;
				path_list.back().push_back(Eigen::Vector3d(point.X / 10000.0, point.Y / 10000.0, 0));
			}
			path_list.back().push_back(Eigen::Vector3d(p[0].X / 10000.0, p[0].Y / 10000.0, 0));
		}
	}
	
	path::dump();
}

void
path::dump()
{
	std::cout << get_name()
		<< ": x = " << this->x
		<< ", y = " << this->y;
	for (path_list_t::iterator it = path_list.begin();it != path_list.end();it++) {
		path_t& p = *it;
		std::cout << "[";
		for (path_t::iterator it2 = p.begin();it2 != p.end();it2++) {
			Eigen::Vector3d& v = *it2;
			std::cout << " (" << v.x() << ", " << v.y() << ")";
		}
		std::cout << "]";
	}
	std::cout << std::endl;
}

}