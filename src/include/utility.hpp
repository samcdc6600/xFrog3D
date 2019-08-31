#ifndef UTILITY_HPP_
#define UTILITY_HPP_


#include <vector>
#include <array>
#include <random>
#include "SOIL.h"
#include "Time.hpp"


// Version of clang on OpenBSD 6.4 won't compile without using a macro for abs!   
#define abs(a) ((a) < 0 ? (-a): a)


/* this is unfortunate but that's the price paid for not including world.hpp, it must be keeped up to date with the
   constants in the axis namespace in world.hpp. However we're probably not going to use more then three
   dimensions so it should be alright :) */
namespace vectorAxis
{
  typedef float tCoords;
  constexpr int dimensions {3};
  constexpr int xIndex {};
  constexpr int yIndex {1};
  constexpr int zIndex {2};
};

namespace initModelViewVars
{
  constexpr GLdouble left	 {-1};
  constexpr GLdouble right	  {1};
  constexpr GLdouble bottom	 {-1};
  constexpr GLdouble top	  {1};
  constexpr GLdouble nearVal	 {-1};
  constexpr GLdouble farVal	  {1};
};

namespace hud
{
  constexpr float r {0.25}, g {0.13}, b {1.0};
  constexpr void * hudFont {GLUT_BITMAP_TIMES_ROMAN_24};
  constexpr float yRowHeight {40}, yGap {40};
  constexpr float x {490}, y1 {yGap + yGap / 2}, y2 {y1 + yRowHeight + yGap / 2}, y3 {y2 + yRowHeight + yGap / 2},
    y4 {y3 + yRowHeight + yGap / 2}, y5 {y4 + yRowHeight + yGap / 2}, z {-0.5};
  constexpr int precision {5};
  constexpr int width {16};
  extern bool on;
  extern float fr, ft;
  extern long frames;
  constexpr float updateInterval {1.0};
  extern Time statTime;
  extern float tLast;
};

namespace randNum
{
  extern std::random_device randDev;	// (seed)
  extern std::mt19937 randomNumberEngine; // (Mersenne-Twister)
  extern std::uniform_int_distribution<int> uni;
  constexpr int minRange {}, maxRange {90};
}

namespace lightCtl
{				// We put this here because it cannot go it world.
};

// Errors
enum errors
{
  RANGE_ERROR,
  INSTANCE_ERROR,
  TEXTURE_ERROR
};

constexpr unsigned char ESC_CHAR {27};
/* Might be better as automatic veriable in function's as some functions support different minimum tesselation
   level's. We leave it here for now. */
constexpr size_t minTesselation {8};


// Having these functions take arguments rather then just access the world namespace makes them more general
/* normScale is multiplied by normals::normalLen to determin the length of normals if normals are drawn */
void drawGrid(const float xStart, const float yStart, const float zStart, const float xLen, const float zLen,
	      const size_t xTess, const size_t zTess, const bool drawNormals, const float normalScale,
	      const bool useTexture = false, const GLuint textureHandle = 0, const float textureScale = 0);
void drawGridNormals(const float xStart, const float yStart, const float zStart, const float xLen, const float zLen,
		     const size_t xTess, const size_t zTess, const bool drawNormals);
void drawBox(const float sLength, const float sHeight, const float sWidth, const float drawNormals,
	     const float normalScale = 0);
void drawBoxNormals(const float sLength, const float sHeight, const float sWidth, const float drawNormals,
		    const float normDrawScaleScale);
void drawSphere(const float xStart, const float yStart, const float zStart, const float zScale, const float r,
		const size_t slices, const size_t stacks, const bool drawNormals, const float normalScale = 0);
void drawSphereNormals(const float xStart, const float yStart, const float zStart, const float zScale, const float r,
		       const size_t slices, const size_t stacks, const bool drawNormals, const float normalScale);
void drawCircle(const float z, const float r, const int tess, const int wireframe, const bool drawNormals,
		const float normalScale = 0);
void drawCircleNormals(const float z, const float r, const int tess, const bool drawNormals,
		       const float normalScale);
void drawCylinder(const float xStart, const float yStart, const float zStart, const float radius,
		  const float zLen, const size_t perimeterTess, const size_t zTess, const bool drawNormals,
		  const float normalScale, const bool useTexture = false, const GLuint textureHandle = 0,
		  const float textureScale = 0);
void drawCylinderNormals(const float xStart, const float yStart, const float zStart, const float radius,
			 const float zLen, const int perimeterTess, const int zTess, const bool drawNormals);
void drawEllipse(const float z, const float ratio, const float r, const int tess, const int wireframe);
void drawString(const float x, const float y, const float z, const char * s, void * font);
void drawProjectile(const float x, const float y, const float z, const float ratio, const float theta, const float r,
		    const int tess, const int wireframe);
void drawTrajectory(const std::vector<float> & trajectory);
/* Draws as a line the vector <a, b, c> at (x, y, z) scaled by s, normalized depending on the value of the
   normalize */
void drawVector(const float x, const float y, const float z, float a, float b, float c, const float scale,
		const bool normalize);
/* Draw nice reaffirming sphere (dot) at the end of the trajectory line. (Doen't nesseserily have to be reaffirming
   but is used for indicating that trajectory line is touching good landing spot on log, actually we are using it
   for red dot as well now)*/
void drawDotPotAtEndOfTrajectoryRainbow(const float xOffset, const float yOffset, const float zOffset,
					const float zDrawDepth, const float radius, const int tesselation,
					const bool wireframe);
void calcNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                const float yEnd, const float z, float & run, float & y, const float normScale);
// Returns the <a, b> normalised via a and b.
void normalize3(float & a, float & b, float & c, const float normScale);
void normalize2(float & a, float & b, const float normScale);
float pythagorean(const float x1, const float y1, const float x2, const float y2);
// Returns true if a (xFuture, yFuture) is detected to have collided with the line ((xLn, yLnTop), (xLn, yLnBottom))
bool vertLineCollision(const float xFuture, const float yFuture, const float r, const float xLn, const float yLnTop,
		       const float yLnBottom);
/* Same as vertLineCollision with the exception that it also set's xNew appropriately (used when we are checking for
   collisions comming from the left) */
bool vertLineLeftCollision(const float xFuture, const float yFuture, const float r, const float xLn,
			   const float yLnTop, const float yLnBottom, float & xNew);
/* Same as vertLineCollision with the exception that it also set's xNew appropriately (used when we are checking for
   collisions comming from the right) */
bool vertLineRightCollision(const float xFuture, const float yFuture, const float r, const float xLn,
			    const float yLnTop, const float yLnBottom, float & xNew);
/* Returns true if a (xFuture, yFuture) is detected to have collided with the line ((xLnLeft, yLn), (xLnRight, yLn))
   and also sets yNew appropriately */
bool horizontalLineCollision(const float xFuture, const float  yFuture, const float r, const float xLnLeft,
			     const float xLnRight, const float yLn, float & yNew);

GLuint loadTexture(const char * fileName);

#endif
