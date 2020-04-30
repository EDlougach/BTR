#pragma once

#include <cmath>

#include <algorithm>
#include <functional>
#include <vector>

#include "config.h"

struct T3DTriangle {
	C3Point vertices[3];
};

struct T3DPlane {
	double A, B, C, D;
	inline double conv(const C3Point p) const
	{
		return A * p.X + B * p.Y + C * p.Z + D;
	}
};

// Predicate, which returns whether some point "p"
// is on a "positive" side of a given plane
class plane_side: std::unary_function<C3Point, bool>
{
	T3DPlane Plane;
public:
	plane_side(T3DPlane plane)
		: Plane(plane)
	{}

	inline bool operator() (const C3Point p) const
	{
		double x = Plane.conv(p);
		return (x > 0);
	}
};

// Finds where a line defined by two points crosses the given plane
// Fails if the line is parallel to the plane
inline C3Point CrossLineWithPlane(const C3Point point1, const C3Point point2, T3DPlane plane)
{
	double c1 = plane.conv(point1);
	double c2 = plane.conv(point2);

	return (point2 * c1 - point1 * c2) / (c1 - c2);
}

// Crosses a polyline with the plane. Never fails.
template<typename Iterator>
bool CrossPolylineWithPlane(Iterator begin, Iterator end, const T3DPlane plane, C3Point &result)
{
	int n = end - begin;
	if (n == 0) return false;
	std::vector<bool> planeSideV(n);
	std::transform(begin, end, planeSideV.begin(), plane_side(plane));
	if (planeSideV[0] == true) {
		std::transform(planeSideV.begin(), planeSideV.end(), planeSideV.begin(), logical_not<bool>);
	}
	int pos = lower_bound(planeSideV.begin(), planeSideV.end(), true) - planeSideV.begin();
	if (pos == n)
	{
		return false;
	}

	C3Point point1, point2;
	point1 = *(begin + (pos - 1));
	point2 = *(begin +  pos);
	result = CrossLineWithPlane(point1, point2, plane);
	return true;
};

// Checks if the point is inside 3D-triangle tr.
// When point isn't on the same plane as tr, treats triangle as ortogonal prism

inline bool PointInTriangle(const C3Point p, T3DTriangle tr)
{
	C3Point m1, m2, m3;
	m1 = VectProd(p - tr.vertices[0], tr.vertices[1] - tr.vertices[0]);
	m2 = VectProd(p - tr.vertices[1], tr.vertices[2] - tr.vertices[1]);
	m3 = VectProd(p - tr.vertices[2], tr.vertices[0] - tr.vertices[2]);
	return (ScalProd(m1, m2) >= 0) && (ScalProd(m2, m3) >= 0) && (ScalProd(m3, m1) >= 0);
}
