#include <vector>

#include "catch.hpp"

#include "delaunay/polygon.h"
#include "delaunay/pointsetarray.h"
#include "delaunay/triangle.h"
#include "delaunay/trianglegeometry.h"

#include "geometry/linesegment.h"

using std::vector;

using namespace geometry;

#define SUT_NAME "geometry/LineSegment"
#define SUT_TAGS "[geometry][LineSegment]"



TEST_CASE(SUT_NAME "/orientation, Trivial", SUT_TAGS "[orientation]") {
	using geometry::Point;
	using geometry::orientation;

	Point<int> originPt(0, 0);
	Point<int> endPt(100, 0);

	Point<int> ccwPt(50, 50);
	Point<int> colPt(200, 0);
	Point<int> cwPt(50, -50);

	// Test CCW,
	CHECK(1 == orientation({originPt, endPt}, ccwPt));

	// Test Co-linear
	CHECK(0 == orientation({originPt, endPt}, colPt));

	// Test CW
	CHECK(-1 == orientation({originPt, endPt}, cwPt));
}



TEST_CASE(SUT_NAME "/orientation, Trivial 2", SUT_TAGS "[orientation]") {
	using geometry::Point;
	using geometry::orientation;

	Point<int> originPt(1000, 1000);
	Point<int>    endPt(1100, 1000);

	Point<int> ccwPt(1050, 1050);
	Point<int> colPt(1200, 1000);
	Point<int>  cwPt(1050,  950);

	// Test CCW,
	CHECK(1 == orientation({originPt, endPt}, ccwPt));

	// Test Co-linear
	CHECK(0 == orientation({originPt, endPt}, colPt));

	// Test CW
	CHECK(-1 == orientation({originPt, endPt}, cwPt));
}



TEST_CASE(SUT_NAME "/intersects, Segment-Segment", SUT_TAGS "[intersects]") {
	using geometry::Point;
	using geometry::intersects;
	using geometry::Intersection;
	using geometry::isOverlapping;

	Point<int> p1(-10, 0);
	Point<int> p2(0, 0);
	Point<int> p3(10, 0);
	Point<int> p4(0, -10);
	Point<int> p5(0, 10);
	Point<int> p6(0, 20);

	// The compiler doesn't like brace-enclosed initialiser lists to intersects,
	// so be explicit

	CHECK(Intersection::Overlap == intersects<int>({p1, p3}, {p4, p5}));

	// Incident, but not intersecting
	CHECK(Intersection::Incidental == intersects<int>({p1, p3}, {p2, p5}));
	CHECK(Intersection::Incidental == intersects<int>({p1, p3}, {p4, p2}));

	// Completele not touching
	CHECK(Intersection::None == intersects<int>({p1, p3}, {p5, p6}));
}



TEST_CASE(SUT_NAME "/intersects, Segment-Segment Colinear Overlapping", SUT_TAGS "[intersects]") {
	using geometry::Point;
	using geometry::intersects;
	using geometry::Intersection;
	using geometry::isOverlapping;

	Point<int> p1(  0, 0);
	Point<int> p2( 10, 0);
	Point<int> p3( 20, 0);
	Point<int> p4( 30, 0);

	// The compiler doesn't like brace-enclosed initialiser lists to intersects,
	// so be explicit
	CHECK(Intersection::Incidental == intersects<int>({p1, p2}, {p2, p3}));

	CHECK(Intersection::ColinearOverlap == intersects<int>({p1, p3}, {p2, p4}));

	CHECK(Intersection::None == intersects<int>({p1, p2}, {p3, p4}));
}



TEST_CASE(SUT_NAME "/findIntersectionPoint, Basic", SUT_TAGS "[findIntersectionPoint]") {
	Point<int> a = {  0,  0 };
	Point<int> b = { 10, 10 };
	Point<int> c = {  0,  2 };
	Point<int> d = {  2,  0 };

	// Should be this
	Point<int> expectedIP(1,1);

	Point<int> actualIP =
		findIntersectionPoint({ a, b }, { c, d });

	CHECK(expectedIP == actualIP);
}
