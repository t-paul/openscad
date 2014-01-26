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
#include "dxfdata.h"
#include "context.h" // get_fragments_from_r()

using namespace Eigen;

static void add_poly(PolySet *ps,
        double x1, double y1, double z1,
        double x2, double y2, double z2,
        double x3, double y3, double z3,
        double x4, double y4, double z4)
{
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
	
	double sx = 1.0, sy = 1.0;
	if (!node.values_scale.empty()) {
		node.values_scale[idx].getVec2(sx, sy);
	}
	
	double ox = 0.0, oy = 0.0;
	if (!node.values_offset.empty()) {
		node.values_offset[idx].getVec2(ox, oy);
	}
	
	Transform3d t(AngleAxisd(M_PI * r / 360, Vector3d(0, 0, 1)));
	t *= Scaling(sx, sy, 1.0);
	t *= Translation3d(ox, oy, 0.0);
	return apply_transform(path, t);
}

static Vector3d make_orthogonal(Vector3d u, Vector3d v) {
	return (u - v.normalized().cross((v.normalized().cross(u)))).normalized();
}

static loft_t create_slices(path_t shape, const LoftNode &node) {
	loft_t loft;

	double height = node.height;
	double max_idx = node.max_idx;
	
	path_t path;
	path_t tangents;
	for (int a = 0;a <= max_idx;a++) {
		if (node.values_path.empty()) {
			path.push_back(Vector3d(0, 0, (a * height) / max_idx));
			tangents.push_back(Vector3d(0, 0, 1));
		} else {
			double x, y, z;

			Value v = node.values_path[a];
			v.getVec3(x, y, z);
			path.push_back(Vector3d(x, y, z));
			
			Value t = node.values_path_tangent[a];
			t.getVec3(x, y, z);
			tangents.push_back(Vector3d(x, y, z));
		}
	}

	Matrix3d last_transform = Matrix3d::Identity();
	for (int a = 0; a < max_idx; a++) {		
		Vector3d tangent = tangents[a];
		Vector3d col3 = last_transform.col(2);
		Vector3d axis = col3.cross(tangent).normalized();
		
		Matrix3d rotate_from_to(Matrix3d::Identity());
		if (axis.dot(axis) >= 0.99) {
			Matrix3d rm1;
			rm1 << tangent.normalized(), axis, axis.cross(tangent.normalized());
			
			Matrix3d rm2;
			rm2 << col3.normalized(), axis, axis.cross(col3.normalized());
			
			rotate_from_to = rm1 * rm2.transpose();
		}
		
		Matrix3d rotation_matrix(Transform3d(last_transform).rotation());
		Matrix3d rotation = rotate_from_to * rotation_matrix;

		Vector3d c0 = rotation.col(0);
		Vector3d c1 = rotation.col(1);
		Vector3d c2 = rotation.col(2);
		rotation << c0.normalized(), make_orthogonal(c1, c0), make_orthogonal(make_orthogonal(c2, c0), c1);
		
		last_transform = rotation;

		Matrix4d transform;
		transform <<
			rotation(0,0), rotation(0,1), rotation(0,2), path[a].x(),
			rotation(1,0), rotation(1,1), rotation(1,2), path[a].y(),
			rotation(2,0), rotation(2,1), rotation(2,2), path[a].z(),
			0, 0, 0, 1;
		
		path_t result;
		path_t p = apply_transform(shape, node, a);
		for (path_t::const_iterator it = p.begin();it != p.end();it++) {
			Vector3d v = *it;
			Vector4d v4(v.x(), v.y(), v.z(), 1);
			Vector4d r;
			r = transform * v4;
			result.push_back(Vector3d(r.x(), r.y(), r.z()));
		}
		loft.push_back(result);
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
	
	bool connect = false;
	if (!node.values_path.empty()) {
		Value p0 = node.values_path[0];
		Value p1 = node.values_path[node.values_path.size() - 1];
		double x0, y0, z0;
		p0.getVec3(x0, y0, z0);
		double x1, y1, z1;
		p1.getVec3(x1, y1, z1);
		double x = x1 - x0;
		double y = y1 - y0;
		double z = z1 - z0;
		double distance = std::sqrt(x * x + y * y + z * z);
		if (distance < 1e-6) {
			connect = true;
		}
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
	if (!connect) {
		ps->append_poly();
		for (path_t::iterator it = bottom.begin();it != bottom.end();it++) {
			Vector3d v = *it;
			ps->append_vertex(v.x(), v.y(), v.z());
		}
	}

	path_t top = loft[loft.size() - 1];
	if (!connect) {
		ps->append_poly();
		for (path_t::reverse_iterator it = top.rbegin();it != top.rend();it++) {
			Vector3d v = *it;
			ps->append_vertex(v.x(), v.y(), v.z());
		}
	}

	for (int a = 1;a < node.max_idx;a++) {
		path_t p1 = loft[a - 1];
		path_t p2 = loft[a];
		
#if 0
		ps->append_poly();
		for (path_t::iterator it = p2.begin();it != p2.end();it++) {
			Vector3d v = *it;
                        //std::cout << v.x() << ", " << v.y() << ", " << v.z() << " - ";
			ps->append_vertex(v.x(), v.y(), v.z());
		}
                //std::cout << std::endl;
#else
		for (unsigned int idx1 = 0;idx1 < p1.size();idx1++) {
			unsigned int idx2 = (idx1 + p1.size() - 1) % p1.size();
			add_poly(ps,
				p1[idx1].x(), p1[idx1].y(), p1[idx1].z(),
				p1[idx2].x(), p1[idx2].y(), p1[idx2].z(),
				p2[idx2].x(), p2[idx2].y(), p2[idx2].z(),
				p2[idx1].x(), p2[idx1].y(), p2[idx1].z());
		}
		
#endif
	}
	
	if (connect) {
		for (unsigned int idx1 = 0;idx1 < top.size();idx1++) {
			unsigned int idx2 = (idx1 + top.size() - 1) % top.size();
			add_poly(ps,
				top[idx1].x(), top[idx1].y(), top[idx1].z(),
				top[idx2].x(), top[idx2].y(), top[idx2].z(),
				bottom[idx2].x(), bottom[idx2].y(), bottom[idx2].z(),
				bottom[idx1].x(), bottom[idx1].y(), bottom[idx1].z());
		}
	}
	
	return ps;
}
