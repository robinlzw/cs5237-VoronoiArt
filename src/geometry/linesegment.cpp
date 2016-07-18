#include "geometry/linesegment.h"

#include "delaunay/li.h"



namespace geometry {

// Adapted from basics/lmath.cpp
template<typename I>
int signDet(const I& x1, const I& y1, const I& w1,
            const I& x2, const I& y2, const I& w2,
            const I& x3, const I& y3, const I& w3) {
	const I& det = x1 * (y2 * w3 - y3 * w2) -
	          x2 * (y1 * w3 - y3 * w1) +
	          x3 * (y1 * w2 - y2 * w1);

	if (det > 0)
		return 1;
	if (det < 0)
		return -1;
	return 0;
}



/// ASSUMPTION that <I> can be constructed from `int`.
template<typename I>
int orientation(const LineSegment<I>& p1p2, const Point<I>& p3) {
	const Point<I>& p1 = p1p2.first;
	const Point<I>& p2 = p1p2.second;
	return signDet(p1.x, p1.y, I(1),
	               p2.x, p2.y, I(1),
	               p3.x, p3.y, I(1));
}

// Instantiate for int, LongInt
template int orientation(const LineSegment<delaunay::LongInt>&, const Point<delaunay::LongInt>&);



bool intersects(const LineSegment<int>& ab, const LineSegment<int>& cd) {
	const Point<int>& a = ab.first;
	const Point<int>& b = ab.second;
	const Point<int>& c = cd.first;
	const Point<int>& d = cd.second;
	return (orientation({a, b}, c) * orientation({a, b}, d) <= 0) &&
	       (orientation({c, d}, a) * orientation({c, d}, b) <= 0);
}



int crossProduct2D(const Point<int>& v, const Point<int>& w) {
	// cross-product of 2D V,W is vx*wy - vy*wx
	return (v.x * w.y) - (v.y * w.x);
}



/// Assumes the points actually intersect.
Point<int> findIntersectionPoint(const LineSegment<int>& ab, const LineSegment<int>& cd) {
	int ax = ab.first.x;
	int ay = ab.first.y;
	int bx = ab.second.x;
	int by = ab.second.y;
	int cx = cd.first.x;
	int cy = cd.first.y;
	int dx = cd.second.x;
	int dy = cd.second.y;

	// Logic from StackOverflow, Gareth Rees's answer.
	// See: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

	// t = cp((C-A), S) / cp(R, S)
	// where R = (B - A),
	//       S = (D - C)

	// TODO: Code would be simpler if Point<I> had operator-
	int rx = bx - ax;
	int ry = by - ay;
	int sx = dx - cx;
	int sy = dy - cy;

	float t = float(crossProduct2D({cx - ax, cy - ay}, {sx, sy})) /
	                crossProduct2D({rx, ry}, {sx, sy});

	// TODO This needs clarification; or use `u = 1 - t` instead.
	// with our definitions, we want to use (1 - t), actually.
	t = 1 - t;

	// A * t + (1 - t) * B
	return Point<int>((int) (ax * t + (1 - t) * bx),
	                  (int) (ay * t + (1 - t) * by));
}

}