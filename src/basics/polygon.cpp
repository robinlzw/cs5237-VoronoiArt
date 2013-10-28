#include "lmath.h"
#include "polygon.h"
#include <vector>

MyPoint::MyPoint() {
	this->x = 0;
	this->y = 0;
}

MyPoint::MyPoint(LongInt x, LongInt y) {
	this->x = x;
	this->y = y;
}

int orientation(const MyPoint& p1, const MyPoint& p2, const MyPoint& p3) {
	return signDet(p1.x, p1.y, LongInt(1),
				   p2.x, p2.y, LongInt(1),
				   p3.x, p3.y, LongInt(1));
}



// Adapted from basics/polygon.cpp
int orientation(int x1, int y1, int x2, int y2, int x3, int y3) {
	return signDet(x1, y1, 1,
				   x2, y2, 1,
				   x3, y3, 1);
}



bool intersects(const MyPoint& a, const MyPoint& b, const MyPoint& c, const MyPoint& d) {
	return (orientation(a, b, c) * orientation(a, b, d) <= 0) &&
		   (orientation(c, d, a) * orientation(c, d, b) <= 0);
}



bool intersects(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy) {
	return (orientation(ax, ay, bx, by, cx, cy) * orientation(ax, ay, bx, by, dx, dy) <= 0) &&
		   (orientation(cx, cy, dx, dy, ax, ay) * orientation(cx, cy, dx, dy, bx, by) <= 0);
}

int inPoly(const std::vector<MyPoint>& poly, const MyPoint & pt) {
	int n = poly.size();

	// Find some point outsize of the poly.
	int smallestXPtIdx = 0;
	
	for (int i = 1; i < n; i++) {
		if (poly[i].x < poly[smallestXPtIdx].x) {
			smallestXPtIdx = i;
		}
	}

	MyPoint outsidePt;
	outsidePt.x = poly[smallestXPtIdx].x - 1; // must be outside of the polygon.
	outsidePt.y = poly[smallestXPtIdx].y;

	// Now count intersections
	int numIntersections = 0;

	for (int i = 0; i < n; i++) {
		if (intersects(outsidePt, pt, poly[i], poly[(i + 1) % n])) {
			numIntersections++;
		}
	}

	return (numIntersections % 2) == 1;
}



// Adapted from basics/lmath.cpp
int signDet(int x1, int y1, int w1,
			int x2, int y2, int w2,
			int x3, int y3, int w3) {
	int det = x1 * (y2 * w3 - y3 * w2) -
		      x2 * (y1 * w3 - y3 * w1) +
			  x3 * (y1 * w2 - y2 * w1);

	if(det > 0) return 1;
	if(det < 0) return -1;
	return 0;
}



int inPoly(const std::vector<int>& poly, int x, int y) {
	int n = poly.size() / 2;

	// Find some point outsize of the poly.
	int smallestXPtIdx = 0;
	
	for (int i = 2; i < n * 2 - 1; i += 2) {
		if (poly[i] < poly[smallestXPtIdx]) {
			smallestXPtIdx = i;
		}
	}

	int outsidePtX;
	int outsidePtY;
	outsidePtX = poly[smallestXPtIdx + 0] - 1; // must be outside of the polygon.
	outsidePtY = poly[smallestXPtIdx + 1];

	// Now count intersections
	int numIntersections = 0;

	for (int i = 0; i < n * 2; i += 2) {
		int p1XIdx = i;
		int p1YIdx = p1XIdx + 1;
		int p2XIdx = (p1XIdx + 2) % (n * 2);
		int p2YIdx = p2XIdx + 1;

		if (intersects(outsidePtX, outsidePtY, x, y, poly[p1XIdx], poly[p1YIdx], poly[p2XIdx], poly[p2YIdx])) {
			numIntersections++;
		}
	}

	return (numIntersections % 2) == 1;
}