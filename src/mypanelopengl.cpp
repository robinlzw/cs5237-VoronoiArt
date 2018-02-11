#include "mypanelopengl.h"

#include <cmath>
#include <cstdio>
#include <stdlib.h>

#include <QtGui/QMouseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QString>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <utility>

#include "platform.h"
#include "stopwatch.h"

#include "delaunay/longint/li.h"

#include "delaunay/pointsetarray.h"
#include "delaunay/delaunay.h"

#include "ui/opengl/graphics.h"

#include "imagedata.h"

using cv::Mat;
using cv::imread;

using std::pair;
using std::string;
using std::vector;

using delaunay::LongInt;
using delaunay::PointSetArray;

using geometry::Polygon;



// TODO: the stopwatch code used makes code less readable.
static StopWatch globalSW;



// DELAUNAY & VORONOI
void MyPanelOpenGL::insertPoint(LongInt x, LongInt y) {
	// DELAUNAY
	inputPointSet_.addPoint(x, y);
}



MyPanelOpenGL::MyPanelOpenGL(QWidget *parent) : QGLWidget (parent) {
}



void MyPanelOpenGL::initializeGL() {
	glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}



void MyPanelOpenGL::resizeGL(int width, int height) {
	refreshProjection(width, height, canvasOffsetX_, canvasOffsetY_, imData_);
}



void MyPanelOpenGL::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	switch (currentRenderType_) {
		case EFFECT:
			drawColoredPolygons(renderedPolygons_);
			break;

		case EDGE_RAW:
			pdfTextures_.edgesTexture->renderPlane();
			break;

		case EDGE_SHARP:
			pdfTextures_.edgesSharpTexture->renderPlane();
			break;

		case EDGE_BLUR:
			pdfTextures_.edgesBlurTexture->renderPlane();
			break;

		case PDF:
			pdfTextures_.pdfTexture->renderPlane();
			break;

		case IMAGE:
			imData_->renderPlane();
			break;

		default:
		case NONE:
			break;
	}

	if (showVoronoiEdges_) {
		// DELAUNAY (voronoiEdges)
		drawVoronoiPolygons(voronoiPolygons_);
	}

	if (showVoronoiSites_) {
		drawPointSetArray(inputPointSet_);
	}

	glPopMatrix();
}



void MyPanelOpenGL::mousePressEvent(QMouseEvent *event) {
	//qDebug("Window: %d, %d\n", event->x(), event->y());
	if (!hasLoadedImage()) {
		return;
	}

	int loadedImageWidth = imData_->width();
	int loadedImageHeight = imData_->height();

	QSize widgetSize = size();
	int windowWidth = widgetSize.width();
	int windowHeight = widgetSize.height();

	double imageRatio = ((double) loadedImageWidth) / loadedImageHeight;
	double windowRatio = ((double) windowWidth) / windowHeight;

	int renderWidth = 2;
	int renderHeight = 2;

	int deltaX = 1;
	int deltaY = 1;

	if (loadedImageWidth > 0) {
		if (imageRatio > windowRatio) {
			double ratio = ((double) windowWidth) / windowHeight;

			renderWidth = loadedImageWidth;
			renderHeight = (int) (loadedImageWidth / ratio);

			deltaY = (renderHeight - loadedImageHeight) / 2;
		} else {
			double ratio = ((double) windowWidth) / windowHeight;

			renderWidth = (int) (loadedImageHeight * ratio);
			renderHeight = loadedImageHeight;

			deltaX = (renderWidth - loadedImageWidth) / 2;
		}

		double viewScale = (double) renderWidth / windowWidth;

		int px = (event->x() * viewScale) - deltaX;
		int py = (event->y() * viewScale) - deltaY;

		qDebug("Insert Point: %d, %d\n", px, py);

		insertPoint(px, py);
		updateNumPoints(inputPointSet_.noPt());

		updateGL();
	}
}



void MyPanelOpenGL::doVoronoiDiagram() {
	//qDebug("Do Voronoi creation\n");

	StopWatch voroSW;

	voroSW.reset();
	voroSW.resume();

	voronoiPolygons_ = delaunay::runDelaunayAlgorithm(inputPointSet_);


	// Make the colored polygons from Voronoi.
	assert(imData_ != NULL);
	renderedPolygons_ = generateColoredPolygons(voronoiPolygons_, *imData_);
	currentRenderType_ = EFFECT;

	voroSW.pause();
	double timePolyColor = voroSW.ms();
	qDebug("TIME: generateColoredPolygons(..) is %f", timePolyColor);
	voroSW.reset();
	voroSW.resume();

	setVoronoiComputed(true);

	updateGL();
}



void MyPanelOpenGL::doOpenImage() {
	//get a filename to open
	QString qStr_fileName =
		QFileDialog::getOpenFileName(Q_NULLPTR,
	                                 tr("Open Image"),
	                                 ".",
	                                 tr("Image Files (*.png *.jpg *.bmp)"));
	if (qStr_fileName == "") {
		return;
	}

	string filenameStr = qStr_fileName.toStdString();

	emit updateFilename(qStr_fileName); // to Qt textbox

	imData_ = loadImageData(filenameStr);
	loadedImageFilename_ = filenameStr;

	QSize widgetSize = size();
	refreshProjection(widgetSize.width(), widgetSize.height(),
	                  canvasOffsetX_, canvasOffsetY_,
	                  imData_);
	emit imageLoaded();

	currentRenderType_ = IMAGE;
	updateGL();
}



void MyPanelOpenGL::doSaveImage() {
	// TODO: This impl. feels impure
	// TODO: Should be a way to set output filename
	// TODO: ImageData stuff applies here, too.
	string outputImageFilename = "output.bmp";

	int x = canvasOffsetX_;
	int y = canvasOffsetY_;

	QSize widgetSize = size();
	int windowWidth = widgetSize.width();
	int windowHeight = widgetSize.height();

	int copyWidth = (windowWidth - (2 * x));
	int copyHeight = (windowHeight - (2 * y));

	int numComponents = 3; // RGB

	// Memory for width * height * RGB pixel values.
	unsigned char *data;
	data = (unsigned char *) malloc(numComponents * copyWidth * copyHeight * sizeof(unsigned char));

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // align to the byte..
	glReadBuffer(GL_FRONT);

	// Read in "correct" row...
	// (I think I have to do this since the texture coordinates' y-axis is flipped
	//  compared to the rendered y-axis).

	for (int rowOffset = 0; rowOffset < copyHeight; rowOffset++) {
		int copyRow = y + copyHeight - rowOffset - 1; // reverse this.
		unsigned char * copyAddress = data + rowOffset * (copyWidth * numComponents);
		glReadPixels(x, copyRow, copyWidth, 1, GL_BGR, GL_UNSIGNED_BYTE, copyAddress);
	}

	Mat img(copyHeight, copyWidth, CV_8UC3, data);
	imwrite(outputImageFilename.c_str(), img);
}



void MyPanelOpenGL::doDrawImage() {
	currentRenderType_ = IMAGE;
	updateGL();
}



void MyPanelOpenGL::doDrawEdge() {
	currentRenderType_ = EDGE_RAW;
	updateGL();
}



void MyPanelOpenGL::doDrawEdgeSharp() {
	currentRenderType_ = EDGE_SHARP;
	updateGL();
}



void MyPanelOpenGL::doDrawEdgeBlur() {
	currentRenderType_ = EDGE_BLUR;

	updateGL();
}



void MyPanelOpenGL::doDrawPDF() {
	currentRenderType_ = PDF;
	updateGL();
}



void MyPanelOpenGL::doDrawEffect() {
	currentRenderType_ = EFFECT;
	updateGL();
}



void MyPanelOpenGL::doGenerateUniformRandomPoints() {
	if (!hasLoadedImage()) return;

	// POINTREP:INTVEC
	// Returns {x0, y0, x1, y1,...}
	int width = imData_->width();
	int height = imData_->height();
	vector< pair<int,int> > points = generateUniformRandomPoints(width, height, numPDFPoints_);

	for (pair<int,int> pt : points) {
		int x = pt.first;
		int y = pt.second;

		insertPoint(x, y);
	}

	updateNumPoints(inputPointSet_.noPt());

	updateGL();
}



void MyPanelOpenGL::doPDF() {
	// POINTREP:INTVEC
	// Returns {x0, y0, x1, y1,...}
	vector< pair<int,int> > points = generatePointsWithPDF(loadedImageFilename_, numPDFPoints_, &pdfTextures_);

	for (pair<int,int> pt : points) {
		int x = pt.first;
		int y = pt.second;

		insertPoint(x, y);
	}

	updateNumPoints(inputPointSet_.noPt());
	setUsePDF(true);

	updateGL();
}



void MyPanelOpenGL::clearAll() {
	// Clear all our points, and such data.

	// Clear all the colored polygons.
	renderedPolygons_.clear();

	voronoiPolygons_.clear();

	// Clear all the points.
	inputPointSet_.eraseAllPoints();

	// Signals and stuff
	updateNumPoints(inputPointSet_.noPt());
	setUsePDF(false);
	setVoronoiComputed(false);

	currentRenderType_ = NONE;

	updateGL();
}



void MyPanelOpenGL::mouseMoveEvent(QMouseEvent *) {
}



void MyPanelOpenGL::setShowVoronoiSites(bool b) {
	showVoronoiSites_ = b;
	updateGL();
}



void MyPanelOpenGL::setShowVoronoiEdges(bool b) {
	showVoronoiEdges_ = b;
	updateGL();
}



void MyPanelOpenGL::setNumPoints1k() {
	setNumPoints(1000);
}



void MyPanelOpenGL::setNumPoints5k() {
	setNumPoints(5000);
}



void MyPanelOpenGL::setNumPoints(int n) {
	numPDFPoints_ = n;
	updateNumPointsToGenerate(n);
}



void MyPanelOpenGL::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			close();
			break;
		default:
			event->ignore();
			break;
	}
}

