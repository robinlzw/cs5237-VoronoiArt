# C++ Voronoi Art - Stained Glass Effect

[![Travis-CI Build Status](https://travis-ci.com/rgoulter/cs5237-VoronoiArt.svg?branch=master)](https://travis-ci.org/rgoulter/cs5237-VoronoiArt)
[![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/ctul394o7fe1189b?svg=true)](https://ci.appveyor.com/project/rgoulter/cs5237-voronoiart)

Create a "stain-glass" effect using the Voronoi diagram of an image, with
randomly generated points.  
This project is based on methods out in the paper:
<http://www.cs.swarthmore.edu/~adanner/cs97/s08/papers/gorbach.pdf>

This was initially developed as a group project for Computational Geometry CS5237 course in NUS in 2013.
Work on this project since then has been to polish/improve it for fun. :-)  
The Voronoi Diagram is computed from the result of the Delaunay Triangulation. It's pretty slow.

## Screenshots

![](https://github.com/rgoulter/cs5237-VoronoiArt/raw/master/screenshot_computing.png)
![](https://github.com/rgoulter/cs5237-VoronoiArt/raw/master/screenshot_voronoi.png)
![](https://github.com/rgoulter/cs5237-VoronoiArt/raw/master/screenshot_voronoi_effect.png)

## Using the GUI

Load an image using the "..." button.

Select a number of points to generate.

Generate random points, either with a uniform distribution, or with a
probability distribution based on detected edges.

The Voronoi diagram is computed when the "Voronoi Effect" radiobutton is selected.

Clicking "Save Image" will output the canvas to "output.bmp" in the current
directory.

## Project Dependencies

Various libraries used in this project include:

* OpenGL - for drawing the image, vertices and polygons.
* OpenCV - for computing find-edge result (& Gaussian blurs) of an image.
* Qt 5 - for the GUI.

## Building on Linux

On Ubuntu/Linux, it's fairly easy to get the above dependencies:
```sudo apt-get install libopencv-dev qt5-default libqt5-opengl5-dev```
should be enough to get all the required dependencies listed above.

Use CMake to generate a Makefile.
e.g.

```
mkdir build
cd build
cmake -Dtest=ON ..
make
```

## Building on Windows

### Visual Studio 15 2017

Visual Studio 2017 (Community Edition) can be downloaded using [Chocolatey](https://chocolatey.org/):

```
choco install cmake visualstudio2017-installer visualstudio2017buildtools visualstudio2017community
```

Microsoft's excellend [vcpkg](https://github.com/Microsoft/vcpkg) can then be used to install our dependencies.
(vcpkg provides the equivalent of MSYS2's use of pacman, for Visual Studio).

```
git clone https://github.com/Microsoft/vcpkg C:\src\vcpkg
cd C:\src\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install opencv qt5
```

(Note that this will require "MFC and ATL support" to be installed, e.g.
[from the Visual Studio installer](https://stackoverflow.com/questions/43074045/mfc-development-in-vs2017)).

```
cmake .. "-DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build .
```

vcpkg will helpfully copy the necessary DLLs to the build directory, making it easy to distribute the program.

### MSYS2 / MinGW-64

[msys2](http://www.msys2.org/) can be installed from [Chocolatey](https://chocolatey.org/)
```
choco install msys2
```

From there, we can install dependencies with:

```
pacman -Syu
pacman -S \
  cmake \
  make \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-mesa \
  mingw-w64-x86_64-opencv \
  mingw-w64-x86_64-qt5
```

Then, with the Mingw64 bin dir `C:\tools\msys64\mingw64\bin` on the `PATH`,

```
mkdir build
cd build
cmake -Dtest=ON -G "MSYS Makefiles" ..
make
```

## General Project Structure

Directories within the root project folder are:

* forms - for Qt .ui form files.
* include - for .h, .hpp header files.
* src - for .cpp source files.
* test - for .cpp unit test source files.
