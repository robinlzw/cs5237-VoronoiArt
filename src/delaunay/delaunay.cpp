#include "delaunay.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <iostream>

#include "li.h"
#include "lmath.h"
#include "pointset.h"

#include "pointsetarray.h"
#include "trist.h"

#include "delaunaytri.h"
#include "directedgraph.h"

using std::vector;
using std::cout;
using std::endl;



void DelaunayTri::legalizeEdge(DirectedGraph& dag, int pIdx1, int pIdx2, int pIdx3) {
	cout << "DTri::legalizeEdge 1" << endl;

	vector<TriRecord> triangles = dag.findTrianglesWithEdge(pIdx2, pIdx3);

	cout << "DTri::legalizeEdge 2" << endl;

	for (int i = 0; i < triangles.size(); i++) {
		for (int j = 0; j < 3; j++) {
			cout << "loop idx " << i << "," << j << endl;
			int pointIdx = triangles[i].pointIndexOf(j);

			if (pointIdx != pIdx1 && pointIdx != pIdx2 && pointIdx != pIdx3) {
				int p4 = pointIdx;

				// Presumably delaunayPointSet === dag.getPointSet()
				// so this is legit
				PointSetArray pointSet = dag.getPointSet();

				cout << "about to check inCircle.." << endl;
				assert(p4 > 0); // check p4!
				if (pointSet.inCircle(pIdx1, pIdx2, pIdx3, p4) > 0) {
					dag.flipTriangles(pIdx1, pIdx2, pIdx3, p4);
					legalizeEdge(dag, pIdx1, pIdx2, p4);
					legalizeEdge(dag, pIdx1, pIdx3, p4);
				}
				cout << "after check inCircle.." << endl;

				return;
			}
		}
	}

	cout << "DTri::legalizeEdge end" << endl;
}



void delaunayIterationStep(vector<int>& delaunayPointsToProcess,
                           DirectedGraph& dag) {
	if (delaunayPointsToProcess.size() == 0) {
		return;
	}

	cout << "dlyIterStep 1" << endl;

	int pIdx = delaunayPointsToProcess[0];
	delaunayPointsToProcess.erase(delaunayPointsToProcess.begin());

	cout << "dlyIterStep 2" << endl;

	// Return the containing triangle for the point i.
	TriRecord tri = dag.addVertex(pIdx);

	cout << "dlyIterStep 3" << endl;

	int triPIdx1, triPIdx2, triPIdx3;
	tri.get(triPIdx1, triPIdx2, triPIdx3);

	cout << "dlyIterStep 4" << endl;

	DelaunayTri::legalizeEdge(dag, pIdx, triPIdx1, triPIdx2);
	DelaunayTri::legalizeEdge(dag, pIdx, triPIdx1, triPIdx3);
	DelaunayTri::legalizeEdge(dag, pIdx, triPIdx2, triPIdx3);

	cout << "dlyIterStep 5" << endl;

	// Redisplay
	// updateGL(); // updateGL is a method of the QGLWidget..
}



// This method checks whether the voronoi edge identified already exists in the existing voronoi edge set.
// bool checkedgeExists(PointSetArray voronoiEdge) {
// 	MyPoint dA, dB;
// 	std::vector<PointSetArray>::iterator iter1;
// 	for (iter1 = voronoiEdges.begin(); iter1 != voronoiEdges.end();) {
// 		PointSetArray vEdge = *iter1;
// 		LongInt x1, y1, x2, y2, vx1, vy1, vx2, vy2;
// 		x1 = vEdge.myPoints[0].x;
// 		y1 = vEdge.myPoints[0].y;
// 		x2 = vEdge.myPoints[1].x;
// 		y2 = vEdge.myPoints[1].y;
// 		vx1 = voronoiEdge.myPoints[0].x;
// 		vy1 = voronoiEdge.myPoints[0].y;
// 		vx2 = voronoiEdge.myPoints[1].x;
// 		vy2 = voronoiEdge.myPoints[1].y;
//
// 		if ((x1==vx1 && y1==vy1 && x2==vx2 && y2==vy2) ||
// 		    (x1==vx2 && y1==vy2 && x2==vx1 && y2==vy1))
// 			return true;
// 		++iter1;
// 	}
//
// 	return false;
// }



void tryDelaunayTriangulation(DirectedGraph& dag) {
	vector<int> delaunayPointsToProcess;

	cout << "tryDelaunayTriangulation 1" << endl;

	PointSetArray delaunayPointSet = dag.getPointSet();

	cout << "tryDelaunayTriangulation 2" << endl;

	// Add points 1 ... n - 3 (inclusive) into the set of points to be tested.
	// (0 ... < n - 3 since delaunayPointSet includes bounding triangle at end)
	for (int i = 1; i <= delaunayPointSet.noPt() - 3; i++) {
		cout << "tryDelaunayTriangulation loop idx=" << i << endl;
		delaunayPointsToProcess.push_back(i);
	}

	cout << "tryDelaunayTriangulation 3" << endl;

	// TODO: Shuffle these points of delaunayPointsToProcess
	srand (time(NULL));
	for (int i = 0; i < delaunayPointsToProcess.size() / 2; i++) {
		cout << "tryDelaunayTriangulation loop idx=" << i << endl;
		int j = rand() % delaunayPointsToProcess.size();

		// swap
		int tmp = delaunayPointsToProcess[i];
		delaunayPointsToProcess[i] = delaunayPointsToProcess[j];
		delaunayPointsToProcess[j] = tmp;
	}

	cout << "tryDelaunayTriangulation 4" << endl;

	// Iterate through the points we need to process.
	// NO ANIMATION, just run each step immediately.
	while (delaunayPointsToProcess.size() > 0) {
		delaunayIterationStep(delaunayPointsToProcess, dag);
	}

	cout << "tryDelaunayTriangulation 5 (end)" << endl;
}



vector<PointSetArray> createVoronoi(DirectedGraph& dag) {
	vector<PointSetArray> voronoiEdges; // Data structure to hold voronoi edges.

	cout << "createVoronoi 1 (begin)" << endl;

	PointSetArray delaunayPointSet = dag.getPointSet();

	cout << "createVoronoi 2" << endl;

	for (int dppIdx = 1; dppIdx <= delaunayPointSet.noPt() - 3; dppIdx++) {
		cout << "createVoronoi for loop, idx=" << dppIdx << endl;

		// Find delaunay triangles to which this point is linked
		cout << "createVoronoi, dag.findTrianglesWithVertex()" << endl;
		std::vector<TriRecord> linkedTriangles = dag.findTrianglesWithVertex(dppIdx);
		PointSetArray polygon;

		// findlinkedNodes method gives an ordered list of triangles. Iterate through and find circumcenters.
		std::vector<TriRecord>::iterator iter1;
		for (iter1 = linkedTriangles.begin(); iter1 != linkedTriangles.end(); ++iter1) {
			cout << "inner-for, idx=" << (iter1 - linkedTriangles.begin()) << endl;
			TriRecord tri = *iter1;
			MyPoint circum;
            std::cout << "circumCircle" << std::endl;
			// TODO circumCircle may benefit from using TriRecord
			int triPIdx1, triPIdx2, triPIdx3;
			tri.get(triPIdx1, triPIdx2, triPIdx3);
			delaunayPointSet.circumCircle(triPIdx1, triPIdx2,triPIdx3, circum);
			polygon.addPoint(circum.x,circum.y);
		}

		voronoiEdges.push_back(polygon);
		cout << "createVoronoi for loop, idx=" << dppIdx << " (end)" << endl;
	}

	cout << "createVoronoi 1 (end)" << endl;

	return voronoiEdges;
}

