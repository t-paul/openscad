#include <math.h>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <boost/foreach.hpp>

#include "module.h"
#include "polyset.h"
#include "dxfdata.h"
#include "loftnode.h"
#include "PolySetCGALEvaluator.h"
#include "CGALEvaluator.h"

#include "printutils.h"
#include "openscad.h"
#include "dxfdata.h" // get_fragments_from_r()

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

	for (int a = 0; a < max_idx; a++) {
		Vector3d normPath = (path[a + 1] - path[a]).normalized();

		// Rotate the shape from X/Y plane to X/Z
		Transform3d rotate0(AngleAxisd(M_PI / 2.0, Vector3d(1, 0, 0)));
		
		// Rotate the shape around the Z axis so that the norm
		// vector of the shape points into the same direction
		// as the path tangent vector in the X/Y plane.
		Vector3d normXY = Vector3d(normPath.x(), normPath.y(), 0).normalized();
		double angle = acos(Vector3d(0, 1, 0).dot(normXY));
		if (Vector3d(1, 0, 0).dot(normXY) >= 0) {
			angle = 2 * M_PI - angle;
		}
		Transform3d rotate1(AngleAxisd(angle, Vector3d(0, 0, 1)));

		// Rotate the shape around the axis in the X/Y plane. The
		// axis is perpendicular to the tangent vector of the path
		// projected to the X/Y plane.
		Vector3d normXYRot90(Transform3d(AngleAxisd(M_PI / 2.0, Vector3d(0, 0, 1))) * normXY);
		double angleZ = acos(normXY.dot(normPath));
		if (Vector3d(0, 0, 1).dot(normPath) >= 0) {
			angleZ = 2 * M_PI - angleZ;
		}
		Transform3d rotate2(AngleAxisd(angleZ, normXYRot90));
		
		Transform3d translate(Translation3d(path[a]));
		path_t tmp(shape);
		tmp = apply_transform(tmp, rotate0);
		tmp = apply_transform(tmp, rotate1);
		tmp = apply_transform(tmp, rotate2);
		tmp = apply_transform(tmp, translate);
		loft.push_back(tmp);
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

static DxfData *evaluateChildNodes(CGALEvaluator *ev, const LoftNode &node)
{
	// Union all (2D) children nodes to a single DxfData
	CGAL_Nef_polyhedron sum;

	BOOST_FOREACH(AbstractNode * v, node.getChildren())
	{
		if (v->modinst->isBackground()) continue;
		CGAL_Nef_polyhedron N = ev->evaluateCGALMesh(*v);
		if (!N.isNull()) {
			if (N.dim != 2) {
				PRINT("ERROR: linear_extrude() is not defined for 3D child objects!");
			} else {
				if (sum.isNull()) sum = N.copy();
				else sum += N;
			}
		}
	}

	if (sum.isNull()) return NULL;
	DxfData *dxf = sum.convertToDxfData();
	return dxf;
}

PolySet *PolySetCGALEvaluator::evaluatePolySet(const LoftNode &node)
{
	PolySet *ps = new PolySet();
	
	DxfData *dxf = evaluateChildNodes(&this->cgalevaluator, node);
	if ((dxf == NULL) || dxf->paths.empty()) {
		return NULL;
	}

	path_t shape;
	DxfData::Path p = dxf->paths[0];
	for (std::vector<int>::iterator it = p.indices.begin();it != p.indices.end();it++) {
		int idx = *it;
		Vector2d v = dxf->points[idx];
		shape.push_back(Vector3d(v.x(), v.y(), 0));
	}
	
	loft_t loft = apply_transform(create_slices(shape, node), node);

	path_t bottom = loft[0];
	ps->append_poly();
	int idx = 0;
	for (path_t::iterator it = bottom.begin();it != bottom.end();it++) {
		Vector3d v = *it;
		ps->append_vertex(v.x(), v.y(), v.z());
	}

	path_t top = loft[loft.size() - 1];
	ps->append_poly();
	for (path_t::reverse_iterator it = top.rbegin();it != top.rend();it++) {
		Vector3d v = *it;
		ps->append_vertex(v.x(), v.y(), v.z());
	}

	for (int a = 1;a < node.max_idx;a++) {
		path_t p1 = loft[a - 1];
		path_t p2 = loft[a];
		
#if 0
		ps->append_poly();
		for (path_t::iterator it = p2.begin();it != p2.end();it++) {
			Vector3d v = *it;
			ps->append_vertex(v.x(), v.y(), v.z());
		}
#else
		for (int idx1 = 0;idx1 < p1.size();idx1++) {
			int idx2 = (idx1 + p1.size() - 1) % p1.size();
			add_poly(ps,
				p1[idx1].x(), p1[idx1].y(), p1[idx1].z(),
				p1[idx2].x(), p1[idx2].y(), p1[idx2].z(),
				p2[idx2].x(), p2[idx2].y(), p2[idx2].z(),
				p2[idx1].x(), p2[idx1].y(), p2[idx1].z());
		}
#endif
	}
	
	return ps;
}
