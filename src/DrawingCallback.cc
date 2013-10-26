#include <math.h>

#include <iostream>
#include <algorithm>

#include "DrawingCallback.h"
#include "dxfdata.h"

DrawingCallback::DrawingCallback(DxfData *data, double fn) : data(data), fn(fn), x_offset(0)
{
}

DrawingCallback::~DrawingCallback()
{
}

void DrawingCallback::set_xoffset(double x_offset)
{
	this->x_offset = x_offset;
}

void DrawingCallback::add_xoffset(double x_offset)
{
	this->x_offset += x_offset;
}

void DrawingCallback::move_to(point_t to) const
{
	std::cout << "move_to: x = " << to.x << ", y = " << to.y << ", offset = " << x_offset << std::endl;
	data->paths.push_back(DxfData::Path());
	data->paths.back().is_closed = true;
	data->paths.back().indices.push_back(data->addPoint(x_offset + to.x, to.y));
}

void DrawingCallback::line_to(point_t to) const
{
	data->paths.back().indices.push_back(data->addPoint(x_offset + to.x, to.y));
}

void DrawingCallback::curve_to(point_t c1, point_t to) const
{
	const Vector2d pen = data->points.back();
	
	int segments = std::max(((int)floor(fn / 6)) + 2, 2);
	for (int idx = 1;idx <= segments;idx++) {
		const double a = idx * (1.0 / segments);
		const double x = (pen[0] - x_offset) * t(a, 2) + c1.x * 2 * t(a, 1) * a + to.x * a * a;
		const double y = pen[1] * t(a, 2) + c1.y * 2 * t(a, 1) * a + to.y * a * a;
		line_to(point_t(x, y));
	}
}

void DrawingCallback::curve_to(point_t c1, point_t c2, point_t to) const
{
	const Vector2d pen = data->points.back();
	
	int segments = std::max(((int)floor(fn / 6)) + 2, 2);
	for (int idx = 1;idx <= segments;idx++) {
		const double a = idx * (1.0 / segments);
		const double x = (pen[0] - x_offset) * t(a, 3) + c1.x * 3 * t(a, 2) * a + c2.x * 3 * t(a, 1) * a * a + to.x * a * a * a;
		const double y = pen[1] * t(a, 3) + c1.y * 3 * t(a, 2) * a + c2.y * 3 * t(a, 1) * a * a + to.y * a * a * a;
		line_to(point_t(x, y));
	}
}

void DrawingCallback::stroke() const
{
}
