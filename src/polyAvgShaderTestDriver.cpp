// polyAvgShaderTestDriver.cpp - for checking the poly-avg-color shader and things.

#include <cstdio>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

#include "SOIL.h"
#include "polypixel.h"

#include "shader_util.h"
#include "stopWatch.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CONSTANTS & GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

// PolyPixel requires us to have these; it'd be nice to do this some other way.
int loadedImageWidth;
int loadedImageHeight;
unsigned char *loadedImageData;

// Reduction factor.
// During the reduction operation, reduceFact x reduceFactor elements
// in the input texture are reduced to 1 element for the output.
const int reduceFact = 2; // Only works for = 2, at the moment.

// Shaders' filenames.
const char vertShaderFilename[] = "reduce.vert";
const char fragShaderFilename[] = "reduce.frag";

// Shader program object.
GLuint shaderProg = 0;

// GLUT window handle.
GLuint glutWindowHandle;



/////////////////////////////////////////////////////////////////////////////
// Initialize the data.
/////////////////////////////////////////////////////////////////////////////

void loadImageData(string imgFilename) {
	loadedImageData =
		SOIL_load_image(imgFilename.c_str(), &loadedImageWidth, &loadedImageHeight, 0, SOIL_LOAD_RGB);
}



/////////////////////////////////////////////////////////////////////////////
// Free the memory allocated to the data.
/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
// Compute on the CPU.
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// Check framebuffer status.
// Modified from the sample code provided in the 
// GL_EXT_framebuffer_object extension sepcifications.
/////////////////////////////////////////////////////////////////////////////
bool CheckFramebufferStatus() {
    GLenum status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            return true;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            fprintf(stderr, "Framebuffer incomplete, incomplete attachment\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing attachment\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            fprintf(stderr, "Framebuffer incomplete, attached images must have same dimensions\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            fprintf(stderr, "Framebuffer incomplete, attached images must have same format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing draw buffer\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            fprintf(stderr, "Framebuffer incomplete, missing read buffer\n");
            return false;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            fprintf(stderr, "Unsupported framebuffer format\n");
            return false;
    }

    return false;
}



/////////////////////////////////////////////////////////////////////////////
// Compute on the GPU.
/////////////////////////////////////////////////////////////////////////////

// Same params as CPU function
float GPU_findAverageColor3iv(const std::vector<int>& poly, int* colorIv) {
	//-----------------------------------------------------------------------------
	// Calculate bounding box and stuff.
	//-----------------------------------------------------------------------------
	int minX, maxX, minY, maxY;
	boundingBox(poly, minX, maxX, minY, maxY);
	
	int boundingBoxWidth = maxX - minX + 1;
	int boundingBoxHeight = maxY - minY + 1;

	//-----------------------------------------------------------------------------
	// Create two floating-point textures. 
	// Use texture rectangle and texture internal format GL_ALPHA32F_ARB.
	//-----------------------------------------------------------------------------

	// Texture A.
    glActiveTexture(GL_TEXTURE0);
    GLuint texA;
    glGenTextures(1, &texA);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texA);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB,
                 loadedImageWidth, loadedImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    printOpenGLError();

	// Texture B.
    glActiveTexture(GL_TEXTURE1);
    GLuint texB;
    glGenTextures(1, &texB);
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texB );
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB,
                 loadedImageWidth, loadedImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    printOpenGLError();

	//-----------------------------------------------------------------------------
	// Attach the two textures to a FBO.
	//-----------------------------------------------------------------------------
    GLuint fbo;
    glGenFramebuffersEXT(1, &fbo); 
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
                              GL_TEXTURE_RECTANGLE_ARB, texA, 0);
	CheckFramebufferStatus();
    printOpenGLError();

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, 
                              GL_TEXTURE_RECTANGLE_ARB, texB, 0);
    CheckFramebufferStatus();
    printOpenGLError();
	
	//-----------------------------------------------------------------------------
	// Generate a texture which is a "mask" of the given polygon.
	// (Black background, white for the polygon).
	//-----------------------------------------------------------------------------
    glActiveTexture(GL_TEXTURE2);
	GLuint texPolyMask;
	glGenTextures(1, &texPolyMask);

    glBindTexture(GL_TEXTURE_2D, texPolyMask);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Clear mask texture to all black
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw a polygon
	glBegin(GL_POLYGON);
		glColor3f(1, 1, 1); // rgb?

		for(int ptIdx = 0; ptIdx < poly.size() / 2; ptIdx++) {
			int x = poly[2 * ptIdx];
			int y = poly[2 * ptIdx + 1];
			glVertex2d(x, y);
		}
	glEnd();

	int uniform_PolyMaskTex = 2;

	//-----------------------------------------------------------------------------
	// Attach loadedImageData to a texture, which we pass to
	// the fragment shader.
	//-----------------------------------------------------------------------------
    glActiveTexture(GL_TEXTURE3);
	GLuint texImg;
	glGenTextures(1, &texImg);

    glBindTexture(GL_TEXTURE_2D, texImg);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		         0,
				 GL_RGB,
				 loadedImageWidth,
				 loadedImageHeight,
				 0,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 loadedImageData);
	
	int uniform_ImageTex = 3;


	//-----------------------------------------------------------------------------
	// Deploy user-defined shaders.
	//-----------------------------------------------------------------------------
    glUseProgram(shaderProg);

	GLint inputTexLoc = glGetUniformLocation(shaderProg, "InputTex");
	GLint inputColsLoc = glGetUniformLocation(shaderProg, "InputCols");
	GLint inputRowsLoc = glGetUniformLocation(shaderProg, "InputRows");
	GLint reduceFactLoc = glGetUniformLocation(shaderProg, "ReduceFact");
	GLint passCountLoc = glGetUniformLocation(shaderProg, "PassCount");
	GLint offsetXLoc = glGetUniformLocation(shaderProg, "OffsetX");
	GLint offsetYLoc = glGetUniformLocation(shaderProg, "OffsetY");

	GLint polyMaskTexLoc = getUniLoc(shaderProg, "PolyMaskTex");
	GLint imageTexLoc = getUniLoc(shaderProg, "ImageTex");

	glUniform1f(reduceFactLoc, reduceFact);

	//-----------------------------------------------------------------------------
	// Set some OpenGL states and projection.
	//-----------------------------------------------------------------------------
    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_COLOR_LOGIC_OP);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glPolygonMode(GL_FRONT, GL_FILL);

	// Set up projection.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, loadedImageWidth, 0, loadedImageHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glViewport(minX, minY, boundingBoxWidth, boundingBoxHeight);
    
	//-----------------------------------------------------------------------------
	// Perform the rendering passes.
	// Use the ping-pong technique.
	//-----------------------------------------------------------------------------

	GLint inputTextureUnit = 0;
	GLenum outputFBOAttachement = GL_COLOR_ATTACHMENT1_EXT;

	int inputRows = boundingBoxHeight;
	int inputCols = boundingBoxWidth;
	int outputRows = boundingBoxHeight;
	int outputCols = boundingBoxWidth;
	int passCount = 0;

	while (true) {
		//---------------------------------------------------------
		// Perform a rendering pass.
		//---------------------------------------------------------
		glDrawBuffer(outputFBOAttachement);

		glUniform1i(inputTexLoc, inputTextureUnit);
		glUniform1f(inputColsLoc, inputCols);
		glUniform1f(inputRowsLoc, inputRows);
		glUniform1i(passCountLoc, passCount);

		glUniform1i(offsetXLoc, minX);
		glUniform1i(offsetYLoc, minY); 

		glUniform1i(polyMaskTexLoc, uniform_PolyMaskTex);
		glUniform1i(imageTexLoc, uniform_ImageTex);

		// Draw a filled quad with size equal to the output size.
		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(outputCols, 0);
			glVertex2f(outputCols, outputRows);
			glVertex2f(0, outputRows);
		glEnd();
		CheckFramebufferStatus();
		printOpenGLError();

		//---------------------------------------------------------
		// Stop looping when result has become one single scalar.
		//---------------------------------------------------------
		if (outputRows == 1 && outputCols == 1) { break; }

		//---------------------------------------------------------
		// Prepare for next rendering pass.
		//---------------------------------------------------------
		// Swap input and output textures. 
		if (inputTextureUnit == 0) {
			inputTextureUnit = 1;
		} else {
			inputTextureUnit = 0;
		}

		if (outputFBOAttachement == GL_COLOR_ATTACHMENT1_EXT) {
			outputFBOAttachement = GL_COLOR_ATTACHMENT0_EXT;
		} else {
			outputFBOAttachement = GL_COLOR_ATTACHMENT1_EXT;
		}

		// Update the input & output sizes.
		inputRows = outputRows;
		inputCols = outputCols;
		outputRows = (outputRows + (reduceFact - 1)) / reduceFact; // Round up the number of output rows.
		outputCols = (outputCols + (reduceFact - 1)) / reduceFact; // Round up the number of output cols.

		passCount++;
	}

	//-----------------------------------------------------------------------------
	// Read output buffer/texture to CPU memory.
	//-----------------------------------------------------------------------------
	float sum;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(outputFBOAttachement);
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &sum);
    printOpenGLError();

	//-----------------------------------------------------------------------------
	// Clean up.
	//-----------------------------------------------------------------------------
    glDeleteFramebuffersEXT(1, &fbo);
    glDeleteTextures(1, &texA);
    glDeleteTextures(1, &texB);

    return sum;
}




/////////////////////////////////////////////////////////////////////////////
// Generate data.
/////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////
// Create a OpenGL rendering context. 
// Check for OpenGL 2.0 and the necessary OpenGL extensions. 
// Read in the shaders from files to create a shader program object.
/////////////////////////////////////////////////////////////////////////////
void PrepareGPUExecution(int argc, char** argv) {
	// Initialize GLUT.
    glutInit(&argc, argv);

    // This creates a OpenGL rendering context so that
    // we can start to issue OpenGL commands after this.
    glutWindowHandle = glutCreateWindow("");  
 
	// Initialize GLEW.
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s.\n", glewGetErrorString(err));
        char ch; scanf("%c", &ch); // Prevents the console window from closing.
        exit(1);
    }

	// Make sure OpenGL 2.0 is supported.
    if (!GLEW_VERSION_2_0) {
        fprintf(stderr, "Error: OpenGL 2.0 is not supported.\n");
        char ch;
		scanf("%c", &ch); // Prevents the console window from closing.
        exit(1);
    }

	// Make sure necessary OpenGL extensions are supported.
    if (!GLEW_ARB_texture_float || 
        !GLEW_EXT_framebuffer_object || 
        !GLEW_ARB_texture_rectangle ) {
        fprintf(stderr, "Error: Some necessary OpenGL extensions are not supported.\n");
        char ch; scanf("%c", &ch); // Prevents the console window from closing.
        exit(1);
    }

	// Create shader program object.
	//string shaderPath;
	//bool found = findShaderDirectory(shaderPath, fragShaderFilename); // just assume found.
	string vertShader = string(vertShaderFilename);
	string fragShader = string(fragShaderFilename);

	shaderProg = makeShaderProgramFromFiles(vertShader.c_str(), fragShader.c_str(), NULL);

    if (shaderProg == 0) {
        fprintf(stderr, "Error: Cannot create shader program object.\n");
        char ch; scanf("%c", &ch); // Prevents the console window from closing.
        exit(1);
    }
}



/////////////////////////////////////////////////////////////////////////////
// The main function.
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    double starttime, endtime;
	StopWatch stopwatch;

    srand(927); // To use same set of random numbers every run.

//-----------------------------------------------------------------------------
// Initialise data
//-----------------------------------------------------------------------------
    string imgFilename = "./sampleImage1.jpg";

	loadImageData(imgFilename);

	if (loadedImageData == NULL) {
		cout << "Failed to load image " << imgFilename << endl;
		getchar();
		return 1;
	}

	// Now pick some points for a polygon..
	// (667, 110)
	// (645, 150)
	// (640, 105)
	vector<int> poly;
	poly.push_back(667); poly.push_back(110);
	poly.push_back(645); poly.push_back(150);
	poly.push_back(640); poly.push_back(105);

//-----------------------------------------------------------------------------
// Perform computation on GPU.
//-----------------------------------------------------------------------------
    printf("GPU COMPUTATION:\n");

	PrepareGPUExecution(argc, argv);
	
	stopwatch.reset();
	stopwatch.resume();

    // GPU Calculation here.

	stopwatch.pause();
	printf("Time elapsed = %.4f msec\n", stopwatch.ms());

    // Print some results.
    printf("Result = ????\n");
    printf("\n\n");

//-----------------------------------------------------------------------------
// Perform computation on CPU.
//-----------------------------------------------------------------------------
    printf("CPU COMPUTATION:\n");
	StopWatch cpuComputationSW;
	cpuComputationSW.resume();

    // CPU Calculation here.
	int cpuResult[3];
	findAverageColor3iv(poly, cpuResult);
	
	cpuComputationSW.pause();
	double time = cpuComputationSW.ms();
	printf("Time elapsed = %.4f msec\n", time);

    // Print some results.
    printf("Result = (%3d, %3d, %3d)\n", cpuResult[0], cpuResult[1], cpuResult[2]);
    printf("\n\n");

//-----------------------------------------------------------------------------
// Clean up.
//-----------------------------------------------------------------------------	
    
    char ch; scanf("%c", &ch); // Prevents the console window from closing.
    return 0;
}