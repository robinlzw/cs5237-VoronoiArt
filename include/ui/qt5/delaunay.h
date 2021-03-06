#ifndef UI_QT5_DELAUNAYH
#define UI_QT5_DELAUNAYH

#include <vector>

#include <QObject>
#include <QReadWriteLock>
#include <QRunnable>

#include "delaunay/longint/li.h"

#include "delaunay/delaunay.h"
#include "delaunay/pointsetarray.h"
#include "delaunay/triangle.h"

#include "geometry/polygon.h"



namespace ui {

namespace qt5 {



class Delaunay : public QObject, public QRunnable {
	Q_OBJECT
public:
	Delaunay(const delaunay::PointSetArray<delaunay::LongInt>& inputPoints, QObject *parent = 0);

	virtual void run();

	bool finished() const { return finished_; };

	const std::vector<geometry::Polygon>& getVoronoiPolygons();

	const delaunay::PointSetArray<delaunay::LongInt>& allPoints() const;

	const std::vector<delaunay::TriRecord>& getLeafNodes();

signals:
	void progressed(int done, int total);

private:
	QReadWriteLock leafNodesLock_;
	QReadWriteLock voronoiPolygonsLock_;
	delaunay::DelaunayAlgorithm<delaunay::LongInt> algorithm_;
	std::vector<delaunay::TriRecord> leafNodes_;
	bool finished_ = false;
};



}

}

#endif
