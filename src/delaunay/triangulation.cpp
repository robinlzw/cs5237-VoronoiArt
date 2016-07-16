#include "delaunay/triangulation.h"

#include <assert.h>

#include <iostream>

#include "delaunay/li.h"
#include "delaunay/pointsetarray.h"

using std::cout;
using std::endl;
using std::vector;



namespace delaunay {

void LinkedTriangle::getEdgeIndices(int iIndex, int& edgeIJ, int& edgeJK, int& edgeKI) const {
	int pIdx1, pIdx2, pIdx3;
	tri_.get(pIdx1, pIdx2, pIdx3);

	if (iIndex == pIdx1) {
		edgeIJ = 0;
		edgeJK = 1;
		edgeKI = 2;
	} else if (iIndex == pIdx2) {
		edgeIJ = 1;
		edgeJK = 2;
		edgeKI = 0;
	} else if (iIndex == pIdx3) {
		edgeIJ = 2;
		edgeJK = 0;
		edgeKI = 1;
	} else {
		edgeIJ = -1;
		edgeJK = -1;
		edgeKI = -1;
	}
}



Triangulation::Triangulation() {
}



Triangulation::~Triangulation() {
	// Delete any triangles which aren't null
	for (vector<LinkedTriangle*>::iterator iter = linkedTriangles_.begin();
	     iter != linkedTriangles_.end();
	     ++iter) {
		LinkedTriangle* tri = *iter;
		if (tri != nullptr) {
			delete tri;
		}
	}
}



FIndex Triangulation::addLinkedTri(const TriRecord& tri) {
	LinkedTriangle* ltri = new LinkedTriangle(tri);
	linkedTriangles_.push_back(ltri);

	// FIndex starts from 1.
	return linkedTriangles_.size();
}



void Triangulation::removeLinkedTri(FIndex triIdx) {
	LinkedTriangle* ltri = linkedTriangles_[triIdx - 1];

	if (ltri != nullptr) {
		delete ltri;
		linkedTriangles_[triIdx - 1] = nullptr;
	}
}



void Triangulation::setLink(FIndex triIJK, int edgeIdx, FIndex otherTri, int otherEdgeIdx) {
#ifdef TRIANGULATION_CHECK
	assert(isLinkedTri(triIJK));
	assert(isLinkedTri(otherTri));
#endif

	LinkedTriangle* ltriIJK   = linkedTriangles_[triIJK - 1];
	ltriIJK->links_[edgeIdx] = otherTri;

	LinkedTriangle* ltriOther = linkedTriangles_[otherTri - 1];
	ltriOther->links_[otherEdgeIdx] = triIJK;
}



// triIJK should be an extant tri; otherTri doesn't need to be.
void Triangulation::setLink(FIndex triIJK, int edgeIdx, FIndex otherTri) {
	if (!isLinkedTri(otherTri)) {
		return;
	}

	// find the index of i,j,k
	LinkedTriangle* ltriIJK = linkedTriangles_[triIJK - 1];
	int iIdx, jIdx, kIdx;
	ltriIJK->tri_.get(iIdx, jIdx, kIdx);


	if (edgeIdx == 0) {
		// otherTri shares edge IJ
		LinkedTriangle* ltriXJI = linkedTriangles_[otherTri - 1];
		int edgeIdxJI, edgeIdxIX, edgeIdxXJ;
		ltriXJI->getEdgeIndices(jIdx, edgeIdxJI, edgeIdxIX, edgeIdxXJ);

		setLink(triIJK, edgeIdx, otherTri, edgeIdxJI);
	} else if (edgeIdx == 1) {
		// otherTri shares edge JK
		LinkedTriangle* ltriXKJ = linkedTriangles_[otherTri - 1];
		int edgeIdxKJ, edgeIdxJX, edgeIdxXK;
		ltriXKJ->getEdgeIndices(kIdx, edgeIdxKJ, edgeIdxJX, edgeIdxXK);

		setLink(triIJK, edgeIdx, otherTri, edgeIdxKJ);
	} else if (edgeIdx == 2) {
		// otherTri shares edge KI
		LinkedTriangle* ltriXIK = linkedTriangles_[otherTri - 1];
		int edgeIdxIK, edgeIdxKX, edgeIdxXI;
		ltriXIK->getEdgeIndices(iIdx, edgeIdxIK, edgeIdxKX, edgeIdxXI);

		setLink(triIJK, edgeIdx, otherTri, edgeIdxIK);
	}
}



const LinkedTriangle& Triangulation::operator[](FIndex idx) const {
#ifdef TRIANGULATION_CHECK
	assert(1 <= idx && idx <= linkedTriangles_.size());
	assert(linkedTriangles_[idx - 1] != nullptr);
#endif

	return *(linkedTriangles_[idx - 1]);
}



LinkedTriangle& Triangulation::operator[](FIndex idx) {
#ifdef TRIANGULATION_CHECK
	assert(1 <= idx && idx <= linkedTriangles_.size());
	assert(linkedTriangles_[idx - 1] != nullptr);
#endif

	return *(linkedTriangles_[idx - 1]);
}



vector<FIndex> Triangulation::getLinkedTriangles() const {
	vector<FIndex> notDeleted;

	for (FIndex fIdx = 1; fIdx <= linkedTriangles_.size(); ++fIdx) {
		if (linkedTriangles_[fIdx - 1] != nullptr) {
			notDeleted.push_back(fIdx);
		}
	}

	return notDeleted;
}



bool Triangulation::checkConsistent() const {
	int numLinksTo0 = 0;
	for (vector<LinkedTriangle*>::const_iterator iter = linkedTriangles_.begin();
	     iter != linkedTriangles_.end();
	     ++iter) {
		LinkedTriangle *tri = *iter;

		if (tri != nullptr) {
			// An link is consistent if it's 0, or a valid Tri.
			for (int idx = 0; idx < 3; idx++) {
				if (tri->links_[idx] == 0) {
					++numLinksTo0;
				} else if (!isLinkedTri(tri->links_[idx])) {
					// link doesn't point to valid tri.
					cout << "Triangulation::checkConsistent(): Bad link exists!" << endl;
					return false;
				}
			}
		}
	}

	// Should be no more than 3 links to outside.
	if (numLinksTo0 > 3) {
		cout << "Triangulation::checkConsistent(): Too many links to 0!" << endl;
		return false;
	}

	return true;
}



bool hasValidLinks(const Triangulation& trist, const LinkedTriangle* ltri) {
	return trist.isLinkedTri(ltri->links_[0]) &&
	       trist.isLinkedTri(ltri->links_[1]) &&
	       trist.isLinkedTri(ltri->links_[2]);
}

}

