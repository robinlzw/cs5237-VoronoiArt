#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <string>

#include "opencv2/imgproc/imgproc.hpp"

// for OpenGL
#include "platform.h"

using cv::Mat;



/// A Helper class for interaction between OpenCV Mat and OpenGL textures.
class ImageData {
public:
	/// Construct ImageData with given matrix,
	/// also provide code to convert the matrix (BGR or GRAY) to RGB.
	ImageData(const cv::Mat& mat, int codeToRGB);

	int width() const {
		return imageMat_.cols;
	}

	int height() const {
		return imageMat_.rows;
	}

	int textureWidth() const {
		return textureMat_.cols;
	}

	int textureHeight() const {
		return textureMat_.rows;
	}

	void dataAt(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b) const;

	/// Renders a plane with same dimensions as image.
	void renderPlane() const;

	/// Returns the matrix with the image data
	const cv::Mat& getImageMat() const { return imageMat_; };

private:
	cv::Mat imageMat_;
	cv::Mat textureMat_;

	GLuint texture_;
};

ImageData* loadImageData(std::string imgFilename);

#endif // IMAGEDATA_H
