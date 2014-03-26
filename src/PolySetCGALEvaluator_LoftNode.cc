#include <math.h>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <boost/foreach.hpp>

#include "module.h"
#include "polyset.h"
#include "loftnode.h"
#include "PolySetCGALEvaluator.h"
#include "CGALEvaluator.h"

#include "printutils.h"
#include "openscad.h" // get_fragments_from_r()

using namespace Eigen;

static void add_poly(PolySet *ps,
        double x1, double y1, double z1,
        double x2, double y2, double z2,
        double x3, double y3, double z3,
        double x4, double y4, double z4)
{
#if 0
        std::cout
                << std::right
                << std::fixed
                << std::setprecision(3)
                << "[" << std::setw(8) << x1
                << ", " << std::setw(8) << y1
                << ", " << std::setw(8) << z1
                << "], [" << std::setw(8) << x2
                << ", " << std::setw(8) << y2
                << ", " << std::setw(8) << z2
                << "], [" << std::setw(8) << x3
                << ", " << std::setw(8) << y3
                << ", " << std::setw(8) << z3
                << "], [" << std::setw(8) << x4
                << ", " << std::setw(8) << y4
                << ", " << std::setw(8) << z4
                << "],"
                << std::endl;
#endif
	ps->append_poly();
	ps->append_vertex(x1, y1, z1);
	ps->append_vertex(x2, y2, z2);
	ps->append_vertex(x3, y3, z3);
	ps->append_poly();
	ps->append_vertex(x3, y3, z3);
	ps->append_vertex(x4, y4, z4);
	ps->append_vertex(x1, y1, z1);
}

static double x(double x1, double y1, double sx1, double ox1, double r1) {
	return (cos(r1) * x1 - sin(r1) * y1) * sx1 + ox1;
}

static double y(double x1, double y1, double sy1, double oy1, double r1) {
	return (sin(r1) * x1 + cos(r1) * y1) * sy1 + oy1;
}

typedef std::vector<Vector3d> path_t;
typedef std::vector<path_t> loft_t;

static path_t apply_transform(const path_t &path, const Transform3d &t)
{
	path_t result;
	for (path_t::const_iterator it = path.begin();it != path.end();it++) {
		Vector3d v = *it;
		result.push_back(t * v);
	}
	return result;
}

static path_t apply_transform(const path_t &path, const LoftNode &node, int idx)
{
	double r = node.values_rotate.empty() ? 0.0 : node.values_rotate[idx];
	double sx = node.values_scale_x.empty() ? 1.0 : node.values_scale_x[idx];
	double sy = node.values_scale_y.empty() ? 1.0 : node.values_scale_y[idx];
	double ox = node.values_offset_x.empty() ? 0.0 : node.values_offset_x[idx];
	double oy = node.values_offset_y.empty() ? 0.0 : node.values_offset_y[idx];
	
	Transform3d t(AngleAxisd(r, Vector3d(0, 0, 1)));
	t *= Scaling(sx, sy, 1.0);
	t *= Translation3d(ox, oy, 0.0);
	return apply_transform(path, t);
	
}
static loft_t create_slices(path_t shape, const LoftNode &node) {
	loft_t loft;

	double height = node.height;
	double max_idx = node.max_idx;
	
	path_t path;
	for (int a = 0;a <= max_idx;a++) {
		if (node.values_path.empty()) {
			path.push_back(Vector3d(0, 0, (a * height) / max_idx));
		} else {
			double x, y, z;
			Value v = node.values_path[a];
			v.getVec3(x, y, z);
			path.push_back(Vector3d(x, y, z));
		}
	}	

	Vector3d norm_old(0, 0, 1);
	for (int a = 0;a < max_idx;a++) {
		Vector3d norm = (path[a + 1] - path[a]).normalized();
		Vector3d axis = Vector3d(0, 0, 1).cross(norm);
		double angle = acos(Vector3d(0, 0, 1).dot(norm));
		Transform3d rotate(AngleAxisd(angle, axis));
		Transform3d translate(Translation3d(path[a]));
		loft.push_back(apply_transform(apply_transform(shape, rotate), translate));
		norm_old = norm;
	}
	return loft;
}

static loft_t apply_transform(loft_t loft, const LoftNode &node)
{
	loft_t result;

	int idx = 0;
	for (loft_t::iterator it = loft.begin();it != loft.end();it++, idx++) {
		path_t p = *it;
		result.push_back(apply_transform(p, node, idx));
	}
	
	return result;
}

PolySet *PolySetCGALEvaluator::evaluatePolySet(const LoftNode &node)
{
	PolySet *ps = new PolySet();

	double x1 = -10, x2 = 10, y1 = -10, y2 = 10;
	path_t shape;
	shape.push_back(Vector3d(x1, y1, 0));
	shape.push_back(Vector3d(x1, y2, 0));
	shape.push_back(Vector3d(x2, y2, 0));
	shape.push_back(Vector3d(x2, y1, 0));
	
	loft_t loft = apply_transform(create_slices(shape, node), node);

	path_t bottom = loft[0];
        add_poly(ps,
		bottom[0].x(), bottom[0].y(), bottom[0].z(),
		bottom[1].x(), bottom[1].y(), bottom[1].z(),
		bottom[2].x(), bottom[2].y(), bottom[2].z(),
		bottom[3].x(), bottom[3].y(), bottom[3].z());
	path_t top = loft[loft.size() - 1];
	add_poly(ps,
		top[3].x(), top[3].y(), top[3].z(),
		top[2].x(), top[2].y(), top[2].z(),
		top[1].x(), top[1].y(), top[1].z(),
		top[0].x(), top[0].y(), top[0].z());
	for (int a = 1;a < node.max_idx;a++) {
		path_t p1 = loft[a - 1];
		path_t p2 = loft[a];

                add_poly(ps,
			p1[0].x(), p1[0].y(), p1[0].z(),
			p1[3].x(), p1[3].y(), p1[3].z(),
			p2[3].x(), p2[3].y(), p2[3].z(),
			p2[0].x(), p2[0].y(), p2[0].z());

                add_poly(ps,
			p1[3].x(), p1[3].y(), p1[3].z(),
			p1[2].x(), p1[2].y(), p1[2].z(),
			p2[2].x(), p2[2].y(), p2[2].z(),
			p2[3].x(), p2[3].y(), p2[3].z());
		
                add_poly(ps,
			p1[2].x(), p1[2].y(), p1[2].z(),
			p1[1].x(), p1[1].y(), p1[1].z(),
			p2[1].x(), p2[1].y(), p2[1].z(),
			p2[2].x(), p2[2].y(), p2[2].z());
		
                add_poly(ps,
			p1[1].x(), p1[1].y(), p1[1].z(),
			p1[0].x(), p1[0].y(), p1[0].z(),
			p2[0].x(), p2[0].y(), p2[0].z(),
			p2[1].x(), p2[1].y(), p2[1].z());
	}
	
	return ps;
}
