#include <cmath>
#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "utility.hpp"
#include "normals.hpp"


namespace hud
{
  bool on {true};
  float fr {}, ft {};
  long frames {};
  Time statTime {};
  float tLast {};
};


namespace box
{
  static constexpr float tFL [] {0.0, 1.0, 0.0};
  static constexpr float tFR [] {0.0, 1.0, 1.0};
  static constexpr float tBL [] {1.0, 1.0, 0.0};
  static constexpr float tBR [] {1.0, 1.0, 1.0};
  static constexpr float bFL [] {0.0, 0.0, 0.0};
  static constexpr float bFR [] {0.0, 0.0, 1.0};
  static constexpr float bBL [] {1.0, 0.0, 0.0};
  static constexpr float bBR [] {1.0, 0.0, 1.0};
};


namespace randNum
{
  std::random_device randDev;	// (seed)
  std::mt19937 randomNumberEngine {randDev()};
  std::uniform_int_distribution<int> uni(minRange, maxRange); // Controlls spawn (random) rate of cars.
}

/* We also define these function's here because we cannot include Camera.hpp and we still feet that they belong in
   camera. */
void disableLighting();
void enableLighting();


void disableLighting()
{
  glDisable(normals::light0);
  glDisable(GL_LIGHTING);
}

  
void enableLighting()
{
  glEnable(normals::light0);
  glEnable(GL_LIGHTING);
}


void drawGrid(const float xStart, const float yStart, const float zStart, const float xLen, const float zLen,
	      const size_t xTess, const size_t zTess, const bool drawNormals, const float normalScale,
	      const bool useTexture, const GLuint textureHandle, const float textureScale)
{
  using normals::normalLen;
  if(xTess >= minTesselation && zTess >= minTesselation)
    {
      const float xSquareLen (xLen / xTess);
      const float zSquareLen (zLen / zTess);

      if(useTexture)
	{
	  //	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, textureHandle);
	}

      float zCoord {zStart};	// Damn fp numbers can't be trusted to controll the loop >:(
      glBegin(GL_TRIANGLES);
      for(int zIter {}; zIter < (zLen / zSquareLen); zCoord += zSquareLen, ++zIter)
	{
	  float xCoord {};
	  xCoord = xStart;
	  
	  for(int xIter {} ; xIter < (xLen / xSquareLen); xCoord += xSquareLen, ++xIter)
	    { // A triangle that makes up part of the strip is drawn for each of the glVertex3f() calls.
	      if(useTexture)
		{		// Draw first triangle
		  		  glNormal3f(xCoord,		yStart + (normalLen * normalScale), 	zCoord);
		  glTexCoord2f(textureScale * 0, textureScale * 1);
		  glVertex3f(xCoord, 			yStart,				zCoord);
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale), 	zCoord);
		  glTexCoord2f(textureScale * 1, textureScale * 1);
		  glVertex3f(xCoord + xSquareLen,	yStart, 				zCoord);
		  glNormal3f(xCoord,		yStart + (normalLen * normalScale),	zCoord + zSquareLen);
		  glTexCoord2f(textureScale * 0, textureScale * (zSquareLen / xSquareLen));
		  glVertex3f(xCoord,		yStart, 				zCoord + zSquareLen);
		  // Draw second triangle
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale),	zCoord);
		  glTexCoord2f(textureScale * 1, textureScale * 1);
		  glVertex3f(xCoord + xSquareLen,	yStart,					zCoord);
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale),	zCoord + zSquareLen);
		  glTexCoord2f(textureScale * 1, textureScale * (zSquareLen / xSquareLen));
		  glVertex3f(xCoord + xSquareLen,	yStart,					zCoord  + zSquareLen);
		  glNormal3f(xCoord,		yStart + (normalLen * normalScale),	zCoord + zSquareLen);
		  glTexCoord2f(textureScale * 0, textureScale * (zSquareLen / xSquareLen));
		  glVertex3f(xCoord,		yStart,					zCoord + zSquareLen);
		}
	      else
		{
		  glNormal3f(xCoord, 			yStart + (normalLen * normalScale), 	zCoord);
		  glVertex3f(xCoord, 			yStart, 				zCoord);
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale), 	zCoord);
		  glVertex3f(xCoord + xSquareLen,	yStart, 				zCoord);
		  glNormal3f(xCoord, 			yStart + (normalLen * normalScale), 	zCoord + zSquareLen);
		  glVertex3f(xCoord, 			yStart, 				zCoord + zSquareLen);
		  // Draw second triangle
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale),	zCoord);
		  glVertex3f(xCoord + xSquareLen,	yStart,					zCoord);
		  glNormal3f(xCoord + xSquareLen,	yStart + (normalLen * normalScale),	zCoord + zSquareLen);
		  glVertex3f(xCoord + xSquareLen,	yStart,					zCoord  + zSquareLen);
		  glNormal3f(xCoord,			yStart + (normalLen * normalScale),	zCoord + zSquareLen);
		  glVertex3f(xCoord,			yStart,					zCoord + zSquareLen);
		}
	    }
	}
      glEnd();
	  
      if(useTexture)
	{
	  glDisable(GL_TEXTURE_2D);
	  //glDisable(GL_BLEND);
	}
      drawGridNormals(xStart, yStart, zStart, xLen, zLen, xTess, zTess, drawNormals);
    }
  else
    {
      std::cerr<<"Error: (in drawGrid()) xTess and or zTess less then minTesselation\n";
      exit(RANGE_ERROR);
    }
}


void drawGridNormals(const float xStart, const float yStart, const float zStart, const float xLen, const float zLen,
		     const size_t xTess, const size_t zTess, const bool drawNormals)
{
  using namespace normals;
  
  if(xTess >= minTesselation && zTess >= minTesselation)
    {
      if(drawNormals)
	{			// Draw normals
	  disableLighting();
	  glColor3f(color[rIndex], color[gIndex], color[bIndex]);
	  const float xSquareLen (xLen / xTess);
	  const float zSquareLen (zLen / zTess);
	  float zCoord {zStart};
	  glBegin(GL_LINES);
	  for(int zIter {}; zIter < (zLen / zSquareLen); zCoord += zSquareLen, ++zIter)
	    {
	      int iter {0};
	      float xCoord {};	  // Draw first two verticies of "strip" of triangles.
	      xCoord = xStart;
	      glVertex3f(xCoord, yStart + (normalLen * normDrawScale),	zCoord);
	      glVertex3f(xCoord, yStart,					zCoord);
	      
	      glVertex3f(xCoord, yStart + (normalLen * normDrawScale), 	zCoord + zSquareLen);
	      glVertex3f(xCoord, yStart,					zCoord + zSquareLen);
	      iter++;
	      for(int xIter {} ; xIter < (xLen / xSquareLen); xCoord += xSquareLen, ++xIter)
		{ // A triangle that makes up part of the strip is drawn for each of the glVertex3f() calls.
		  glVertex3f(xCoord + xSquareLen, 	yStart + (normalLen * normDrawScale),	zCoord);
		  glVertex3f(xCoord + xSquareLen, 	yStart,					zCoord);

		  glVertex3f(xCoord + xSquareLen,	yStart + (normalLen * normDrawScale), zCoord + zSquareLen);
		  glVertex3f(xCoord + xSquareLen,	yStart, 				zCoord + zSquareLen);
		  iter++;
		}
	    }
	  glEnd();
	  enableLighting();
	  //	  glColor3f(0, 0, 1);
	}
    }
  else
    {
      std::cerr<<"Error: (in drawGridNormals()) xTess and or zTess less then minTesselation\n";
      exit(RANGE_ERROR);
    }
}


void drawBox(const float sLength, const float sHeight, const float sWidth, const float drawNormals,
	     const float normalScale)
{
  using namespace::box;
  using namespace::vectorAxis;
  using normals::normalLen;

        const float xOffset {(normalLen * normalScale * (1 / sLength))},
	yOffset {(normalLen * normalScale * (1 / sHeight))},
	  zOffset {(normalLen * normalScale * (1 / sWidth))};

  glPushMatrix();
  glScalef(sLength, sHeight, sWidth);
  glBegin(GL_QUADS);		// We are not sure if the normals here are corret.
  // Front face normals
  glVertex3f(bFL[xIndex], 			bFL[yIndex],   	bFL[zIndex]);
  glNormal3f(bFL[xIndex] - xOffset, 	bFL[yIndex], 	bFL[zIndex]);
  glVertex3f(bFR[xIndex], 			bFR[yIndex],   	bFR[zIndex]);
  glNormal3f(bFR[xIndex] - xOffset, 	bFR[yIndex], 	bFR[zIndex]);
  glVertex3f(tFR[xIndex], 			tFR[yIndex],	tFR[zIndex]);
  glNormal3f(tFR[xIndex] - xOffset, 	tFR[yIndex], 	tFR[zIndex]);
  glVertex3f(tFL[xIndex], 			tFL[yIndex],   	tFL[zIndex]);
  glNormal3f(tFL[xIndex] - xOffset, 	tFL[yIndex], 	tFL[zIndex]);
  // Top face normals
  glVertex3f(tFL[xIndex], tFL[yIndex], 		tFL[zIndex]);
  glNormal3f(tFL[xIndex], tFL[yIndex] + yOffset, 	tFL[zIndex]);
  glVertex3f(tFR[xIndex], tFR[yIndex], 		tFR[zIndex]);
  glNormal3f(tFR[xIndex], tFR[yIndex] + yOffset, 	tFR[zIndex]);
  glVertex3f(tBR[xIndex], tBR[yIndex], 		tBR[zIndex]);
  glNormal3f(tBR[xIndex], tBR[yIndex] + yOffset, 	tBR[zIndex]);
  glVertex3f(tBL[xIndex], tBL[yIndex], 		tBL[zIndex]);
  glNormal3f(tBL[xIndex], tBL[yIndex] + yOffset, 	tBL[zIndex]);
  // Back face normals
  glVertex3f(bBL[xIndex], 			bBL[yIndex],   	bBL[zIndex]);
  glNormal3f(bBL[xIndex] + xOffset, 	bBL[yIndex], 	bBL[zIndex]);
  glVertex3f(bBR[xIndex], 			bBR[yIndex],   	bBR[zIndex]);
  glNormal3f(bBR[xIndex] + xOffset, 	bBR[yIndex], 	bBR[zIndex]);
  glVertex3f(tBR[xIndex], 			tBR[yIndex],	tBR[zIndex]);
  glNormal3f(tBR[xIndex] + xOffset, 	tBR[yIndex], 	tBR[zIndex]);
  glVertex3f(tBL[xIndex], 			tBL[yIndex],   	tBL[zIndex]);
  glNormal3f(tBL[xIndex] + xOffset, 	tBL[yIndex], 	tBL[zIndex]);
  // Bottom face normals
  glVertex3f(bFL[xIndex], bFL[yIndex], 		bFL[zIndex]);
  glNormal3f(bFL[xIndex], bFL[yIndex] - yOffset, 	bFL[zIndex]);
  glVertex3f(bFR[xIndex], bFR[yIndex], 		bFR[zIndex]);
  glNormal3f(bFR[xIndex], bFR[yIndex] - yOffset, 	bFR[zIndex]);
  glVertex3f(bBR[xIndex], bBR[yIndex], 		bBR[zIndex]);
  glNormal3f(bBR[xIndex], bBR[yIndex] - yOffset, 	bBR[zIndex]);
  glVertex3f(bBL[xIndex], bBL[yIndex], 		bBL[zIndex]);
  glNormal3f(bBL[xIndex], bBL[yIndex] - yOffset, 	bBL[zIndex]);
  // Left face normals
  glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex]);
  glNormal3f(bFL[xIndex], bFL[yIndex], bFL[zIndex] - zOffset);
  glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex]);
  glNormal3f(tFL[xIndex], tFL[yIndex], tFL[zIndex] - zOffset);
  glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex]);
  glNormal3f(tBL[xIndex], tBL[yIndex], tBL[zIndex] - zOffset);
  glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex]);
  glNormal3f(bBL[xIndex], bBL[yIndex], bBL[zIndex] - zOffset);
  // Right face normals
  glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex]);
  glNormal3f(bFR[xIndex], bFR[yIndex], bFR[zIndex] + zOffset);
  glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex]);
  glNormal3f(tFR[xIndex], tFR[yIndex], tFR[zIndex] + zOffset);
  glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex]);
  glNormal3f(tBR[xIndex], tBR[yIndex], tBR[zIndex] + zOffset);
  glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex]);
  glNormal3f(bBR[xIndex], bBR[yIndex], bBR[zIndex] + zOffset);
  glEnd();  
  glPopMatrix();

  drawBoxNormals(sLength, sHeight, sWidth, drawNormals, normalScale);
}


void drawBoxNormals(const float sLength, const float sHeight, const float sWidth, const float drawNormals,
		    const float normDrawScaleScale)
{				// normalLen * normalScale
  using namespace::box;
  using namespace::vectorAxis;
  using namespace::normals;
  
  if(drawNormals)
    {
      glPushMatrix();
      glScalef(sLength, sHeight, sWidth);
      /* drawBoxNormals() is called by drawBox() which is called by Frogject::draw() (indirectly) where a glScaile
	 operation is first applied to change the shape and size of the box drawn. So we need to add an appropriate
	 offset to the second vertice of each normal line. (This may not be the best design but I don't really have
	 time to change it at this point.)*/
      const float xOffset {(normalLen * normals::normDrawScale * normDrawScaleScale * (1 / sLength))},
	yOffset {(normalLen * normals::normDrawScale * normDrawScaleScale * (1 / sHeight))},
	  zOffset {(normalLen * normals::normDrawScale * normDrawScaleScale * (1 / sWidth))};

      disableLighting();
      glColor3f(color[rIndex], color[gIndex], color[bIndex]);
      glBegin(GL_LINES);
      // Front face normals
      glVertex3f(bFL[xIndex], 			bFL[yIndex],   	bFL[zIndex]);
      glVertex3f(bFL[xIndex] - xOffset, 	bFL[yIndex], 	bFL[zIndex]);
      glVertex3f(bFR[xIndex], 			bFR[yIndex],   	bFR[zIndex]);
      glVertex3f(bFR[xIndex] - xOffset, 	bFR[yIndex], 	bFR[zIndex]);
      glVertex3f(tFL[xIndex], 			tFL[yIndex],   	tFL[zIndex]);
      glVertex3f(tFL[xIndex] - xOffset, 	tFL[yIndex], 	tFL[zIndex]);
      glVertex3f(tFR[xIndex], 			tFR[yIndex],	tFR[zIndex]);
      glVertex3f(tFR[xIndex] - xOffset, 	tFR[yIndex], 	tFR[zIndex]);
      // Top face normals
      glVertex3f(tFL[xIndex], tFL[yIndex], 		tFL[zIndex]);
      glVertex3f(tFL[xIndex], tFL[yIndex] + yOffset, 	tFL[zIndex]);
      glVertex3f(tFR[xIndex], tFR[yIndex], 		tFR[zIndex]);
      glVertex3f(tFR[xIndex], tFR[yIndex] + yOffset, 	tFR[zIndex]);
      glVertex3f(tBL[xIndex], tBL[yIndex], 		tBL[zIndex]);
      glVertex3f(tBL[xIndex], tBL[yIndex] + yOffset, 	tBL[zIndex]);
      glVertex3f(tBR[xIndex], tBR[yIndex], 		tBR[zIndex]);
      glVertex3f(tBR[xIndex], tBR[yIndex] + yOffset, 	tBR[zIndex]);
      // Back face normals
      glVertex3f(bBL[xIndex], 			bBL[yIndex],   	bBL[zIndex]);
      glVertex3f(bBL[xIndex] + xOffset, 	bBL[yIndex], 	bBL[zIndex]);
      glVertex3f(bBR[xIndex], 			bBR[yIndex],   	bBR[zIndex]);
      glVertex3f(bBR[xIndex] + xOffset, 	bBR[yIndex], 	bBR[zIndex]);
      glVertex3f(tBL[xIndex], 			tBL[yIndex],   	tBL[zIndex]);
      glVertex3f(tBL[xIndex] + xOffset, 	tBL[yIndex], 	tBL[zIndex]);
      glVertex3f(tBR[xIndex], 			tBR[yIndex],	tBR[zIndex]);
      glVertex3f(tBR[xIndex] + xOffset, 	tBR[yIndex], 	tBR[zIndex]);
      // Bottom face normals
      glVertex3f(bFL[xIndex], bFL[yIndex], 		bFL[zIndex]);
      glVertex3f(bFL[xIndex], bFL[yIndex] - yOffset, 	bFL[zIndex]);
      glVertex3f(bFR[xIndex], bFR[yIndex], 		bFR[zIndex]);
      glVertex3f(bFR[xIndex], bFR[yIndex] - yOffset, 	bFR[zIndex]);
      glVertex3f(bBL[xIndex], bBL[yIndex], 		bBL[zIndex]);
      glVertex3f(bBL[xIndex], bBL[yIndex] - yOffset, 	bBL[zIndex]);
      glVertex3f(bBR[xIndex], bBR[yIndex], 		bBR[zIndex]);
      glVertex3f(bBR[xIndex], bBR[yIndex] - yOffset, 	bBR[zIndex]);
      // Left face normals
      glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex]);
      glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex] - zOffset);
      glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex]);
      glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex] - zOffset);
      glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex]);
      glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex] - zOffset);
      glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex]);
      glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex] - zOffset);
      // Right face normals
      glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex]);
      glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex] + zOffset);
      glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex]);
      glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex] + zOffset);
      glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex]);
      glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex] + zOffset);
      glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex]);
      glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex] + zOffset);
      glEnd();
      enableLighting();
  
      glPopMatrix();
    }
}


/* The code that makes up the basic functionallity of this function was taken from the lecture "Procedural modelling,
   normals and vertex arrays"
   found at the url:
   http://titan.csit.rmit.edu.au/~e20068/teaching/i3dg&a/2017/lecture-promod.html
   I have extended it's base functinallity. */
void drawSphere(const float xStart, const float yStart, const float zStart, const float zScale, const float r,
		const size_t slices, const size_t stacks, const bool drawNormals, const float normalScale)
{
  using normals::normalLen;
  if(slices >= minTesselation && stacks >= minTesselation)
    {
      float theta {}, phi {};
      float x0 {}, y0 {}, z0 {}, x1 {}, y1 {}, z1 {}; // Used for visible sphere.
      float xN0 {}, yN0 {}, zN0 {}, xN1 {}, yN1 {}, zN1 {}; // Used for normals :)
      const float step_phi {float(M_PI / stacks)};

      for (size_t j = 0; j <= stacks; j++)
        {
          phi = j / (float)stacks * M_PI;
          glBegin(GL_QUAD_STRIP);
          for (size_t i = 0; i <= slices; i++)
            {
              theta = i / (float)slices * 2.0 * M_PI;
              x0 = r * sinf(phi) * cosf(theta); // Calculate inner (visible) circle verticies.
              y0 = r * sinf(phi) * sinf(theta);
              z0 = r * zScale * cosf(phi);
              x1 = r * sinf(phi + step_phi) * cosf(theta);
              y1 = r * sinf(phi + step_phi) * sinf(theta);
              z1 = r * zScale * cosf(phi + step_phi);
	      // Calculate outer circle vertices (used for normal lines.)
              xN0 = (r + normalLen * normalScale) * sinf(phi) * cosf(theta);
              yN0 = (r + normalLen * normalScale) * sinf(phi) * sinf(theta);
              zN0 = (r + normalLen * normalScale) * zScale * cosf(phi);
              xN1 = (r + normalLen * normalScale) * sinf(phi + step_phi) * cosf(theta);
              yN1 = (r + normalLen * normalScale) * sinf(phi + step_phi) * sinf(theta);
              zN1 = (r + normalLen * normalScale) * zScale * cosf(phi + step_phi);

              glNormal3f(xStart + xN0, yStart + yN0, zStart + zN0);
              glVertex3f(xStart + x0, yStart + y0, zStart + z0);
              glNormal3f(xStart + xN1, yStart + yN1, zStart + zN1);
              glVertex3f(xStart + x1, yStart + y1, zStart + z1);
            }
          glEnd();
        }
      drawSphereNormals(xStart, yStart, zStart, zScale, r, slices, stacks, drawNormals, normalScale);
    }
  else
    {
      std::cerr<<"Error: (in drawSphere()) slices or stacks less then minTesselation\n";
      exit(RANGE_ERROR);
    }
}


void drawSphereNormals(const float xStart, const float yStart, const float zStart, const float zScale, const float r,
		       const size_t slices, const size_t stacks, const bool drawNormals, const float normalScale)
{
  using namespace::normals;
  const float offset {(normals::normalLen * normalScale * normals::normDrawScale)};
  //  const float offset {(normals::normalLen * normals::normDrawScale)};
  
  if(drawNormals)
    {
      if(slices >= minTesselation && stacks >= minTesselation)
	{
	  disableLighting();
	  glColor3f(color[rIndex], color[gIndex], color[bIndex]);
      
	  float theta {}, phi {};
	  float x0 {}, y0 {}, z0 {}, x1 {}, y1 {}, z1 {}; // Used for visible sphere.
	  float xN0 {}, yN0 {}, zN0 {}, xN1 {}, yN1 {}, zN1 {}; // Used for normals :)
	  const float step_phi {float(M_PI / stacks)};

	  for (size_t j = 0; j <= stacks; j++)
	    {
	      phi = j / (float)stacks * M_PI;
	      glBegin(GL_LINES);
	      for (size_t i = 0; i <= slices; i++)
		{		// normalize2(float & a, float & b, const float normScale)
		  theta = i / (float)slices * 2.0 * M_PI;
		  x0 = r * sinf(phi) * cosf(theta); // Calculate inner (visible) circle verticies.
		  y0 = r * sinf(phi) * sinf(theta);
		  z0 = r * zScale * cosf(phi);
		  x1 = r * sinf(phi + step_phi) * cosf(theta);
		  y1 = r * sinf(phi + step_phi) * sinf(theta);
		  z1 = r * zScale * cosf(phi + step_phi);
		  /* Calculate outer circle vertices (used for normal lines.) "+ offset" isn't quite right because it
		     only give's the right result for true sphere's. */
		  xN0 = (r + offset) * sinf(phi) * cosf(theta);
		  yN0 = (r + offset) * sinf(phi) * sinf(theta);
		  zN0 = (r + offset) * zScale * cosf(phi);
		  xN1 = (r + offset) * sinf(phi + step_phi) * cosf(theta);
		  yN1 = (r + offset) * sinf(phi + step_phi) * sinf(theta);
		  zN1 = (r + offset) * zScale * cosf(phi + step_phi);
		  
		  glVertex3f(xStart + xN0, yStart + yN0, zStart + zN0);
		  glVertex3f(xStart + x0, yStart + y0, zStart + z0);
		  glVertex3f(xStart + xN1, yStart + yN1, zStart + zN1);
		  glVertex3f(xStart + x1, yStart + y1, zStart + z1);
		}
	      glEnd();
	    }
	  enableLighting();
	}
      else
	{
	  std::cerr<<"Error: (in drawSphereNormals()) slices or stacks less then minTesselation\n";
	  exit(RANGE_ERROR);
	}
    }
}


void drawCircle(const float z, const float r, const int tess, const int wireframe, const bool drawNormals,
		const float normalScale)
{     	    
  float theta {r}, x, y;

  if(!wireframe)
      glBegin(GL_POLYGON);
  else
    glBegin(GL_LINE_LOOP);  
  for(int i {}; i <= tess; ++i)
    {
      theta = i / (float)tess * 2.0 * M_PI;
      x = r * cosf(theta);
      y = r * sinf(theta);
      glNormal3f(x, y, z + (normals::normalLen * normalScale));
      glVertex3f(x, y, z);
    }  
  glEnd();

  drawCircleNormals(z, r, tess, drawNormals, normalScale);
}


void drawCircleNormals(const float z, const float r, const int tess, const bool drawNormals,
		const float normalScale)
{
  using namespace::normals;
  if(drawNormals)
    {
      disableLighting();
      glColor3f(color[rIndex], color[gIndex], color[bIndex]);
	  
      float theta {r}, x, y;

      glBegin(GL_LINES);
      for(int i {}; i <= tess; ++i)
	{
	  std::cout<<"Hello "<<std::endl;
	  theta = i / (float)tess * 2.0 * M_PI;
	  x = r * cosf(theta);
	  y = r * sinf(theta);
	  glVertex3f(x, y, z);
	  glVertex3f(x, y, z + (normals::normalLen * normals::normDrawScale));
	}
      glEnd();
    }

  enableLighting();
}


void drawCylinder(const float xStart, const float yStart, const float zStart, const float radius,
		  const float zLen, const size_t perimeterTess, const size_t zTess, const bool drawNormals,
		  const float normalScale, const bool useTexture, const GLuint textureHandle,
		  const float textureScale)
{
  using normals::normalLen;
  using normals::normDrawScale;
  
  if(perimeterTess >= minTesselation && zTess >= minTesselation)
    {
      float thetaInner {radius},	x0Inner {}, y0Inner {}, x1Inner {}, y1Inner {};
      float radiusOutter {radius + normalLen};
      float thetaOutter {radiusOutter}, x0Outter {}, y0Outter {}, x1Outter {}, y1Outter {};
      const float zSquareLen (zLen / zTess);

      if(useTexture)
	{
	  glEnable(GL_BLEND);
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, textureHandle);
	}

      glBegin(GL_TRIANGLES);
      for(size_t iter {}; iter < perimeterTess; ++iter)
	{			// Calculate vertices for inward facing ends of normals.
	  thetaInner = iter / (float)perimeterTess * 2.0 * M_PI;
	  x0Inner = radius * cosf(thetaInner);
	  y0Inner = radius * sinf(thetaInner);
	  thetaInner = (iter + 1) / (float)perimeterTess * 2.0 * M_PI;
	  x1Inner = radius * cosf(thetaInner);
	  y1Inner = radius * sinf(thetaInner);
	  // Calculate vertices for outward facing ends of normals.
	  thetaOutter = iter / (float)perimeterTess * 2.0 * M_PI;
	  x0Outter = radiusOutter * cosf(thetaOutter);
	  y0Outter = radiusOutter * sinf(thetaOutter);
	  thetaOutter = (iter + 1) / (float)perimeterTess * 2.0 * M_PI;
	  x1Outter = radiusOutter * cosf(thetaOutter);
	  y1Outter = radiusOutter * sinf(thetaOutter);

	  float zCoord {zStart};	// Damn fp numbers can't be trusted to controll the loop >:(
	  for(size_t zIter {}; zIter < (zLen / zSquareLen); zCoord += zSquareLen, ++zIter)
	    {
	      glNormal3f(x0Outter,	y0Outter,	zCoord);
	      glTexCoord2f(textureScale * 0, textureScale * 1);
	      glVertex3f(x0Inner,	y0Inner,	zCoord);
	      glNormal3f(x1Outter,	y1Outter,	zCoord);
	      glTexCoord2f(textureScale * 1, textureScale * 1);
	      glVertex3f(x1Inner,	y1Inner,	zCoord);
	      glNormal3f(x0Outter,	y0Outter,	zCoord + zSquareLen);
	      glTexCoord2f(textureScale * 0, textureScale * 0);
	      glVertex3f(x0Inner,	y0Inner,	zCoord + zSquareLen);
	      // Draw second triangle.
	      glNormal3f(x1Outter,	y1Outter,	zCoord);
	      glTexCoord2f(textureScale * 0, textureScale * 0);
	      glVertex3f(x1Inner,	y1Inner,	zCoord);
	      glNormal3f(x1Outter,	y1Outter,	zCoord  + zSquareLen);
	      glTexCoord2f(textureScale * 0, textureScale * 1);
	      glVertex3f(x1Inner,	y1Inner,	zCoord  + zSquareLen);
	      glNormal3f(x0Outter,	y0Outter,	zCoord + zSquareLen);
	      glTexCoord2f(textureScale * 1, textureScale * 1);
	      glVertex3f(x0Inner,	y0Inner,	zCoord + zSquareLen);
	      
	    }
	}
      glEnd();
	  
      if(useTexture)
	{
	  glDisable(GL_TEXTURE_2D);
	}
      
      drawCylinderNormals(xStart, yStart, zStart, radius, zLen, perimeterTess, zTess, drawNormals);
    }
  else
    {
      std::cerr<<"Error: (in drawCylinder()) perimeterTess and or zTess less then minTesselation\n";
      exit(RANGE_ERROR);
    }
}


void drawCylinderNormals(const float xStart, const float yStart, const float zStart, const float radius,
			 const float zLen, const int perimeterTess, const int zTess, const bool drawNormals)
{
  using namespace::normals;
  
  constexpr int minTesselation {1};

  if(perimeterTess >= minTesselation && zTess >= minTesselation)
    {
      if(drawNormals)
	{
	  disableLighting();
	  glColor3f(color[rIndex], color[gIndex], color[bIndex]);
	  
	  float thetaInner {radius},	x0Inner {}, y0Inner {}, x1Inner {}, y1Inner {};
	  float radiusOutter {radius + (normalLen * normDrawScale)};
	  float thetaOutter {radiusOutter}, x0Outter {}, y0Outter {}, x1Outter {}, y1Outter {};
	  const float zSquareLen (zLen / zTess);
     
	  for(int iter {}; iter < perimeterTess; ++iter)
	    {			// Calculate vertices for inward facing ends of normals.
	      thetaInner = iter / (float)perimeterTess * 2.0 * M_PI;
	      x0Inner = radius * cosf(thetaInner);
	      y0Inner = radius * sinf(thetaInner);
	      thetaInner = (iter + 1) / (float)perimeterTess * 2.0 * M_PI;
	      x1Inner = radius * cosf(thetaInner);
	      y1Inner = radius * sinf(thetaInner);
	      // Calculate vertices for outward facing ends of normals.
	      thetaOutter = iter / (float)perimeterTess * 2.0 * M_PI;
	      x0Outter = radiusOutter * cosf(thetaOutter);
	      y0Outter = radiusOutter * sinf(thetaOutter);
	      thetaOutter = (iter + 1) / (float)perimeterTess * 2.0 * M_PI;
	      x1Outter = radiusOutter * cosf(thetaOutter);
	      y1Outter = radiusOutter * sinf(thetaOutter);

	      glBegin(GL_LINES);
	      float zCoord {zStart};	// Damn fp numbers can't be trusted to controll the loop >:(
	      for(int zIter {}; zIter < (zLen / zSquareLen); zCoord += zSquareLen, ++zIter)
		{
		  glVertex3f(x0Inner,	y0Inner,	zCoord);
		  glVertex3f(x0Outter,	y0Outter,	zCoord);
		  glVertex3f(x1Inner,	y1Inner,	zCoord);
		  glVertex3f(x1Outter,	y1Outter,	zCoord);
		  glVertex3f(x0Inner,	y0Inner,	zCoord + zSquareLen);
		  glVertex3f(x0Outter,	y0Outter,	zCoord + zSquareLen);
		  // Draw second triangle.
		  glVertex3f(x1Inner,	y1Inner,	zCoord);
		  glVertex3f(x1Outter,	y1Outter,	zCoord);	      
		  glVertex3f(x1Inner,	y1Inner,	zCoord  + zSquareLen);
		  glVertex3f(x1Outter,	y1Outter,	zCoord  + zSquareLen);
		  glVertex3f(x0Inner,	y0Inner,	zCoord + zSquareLen);
		  glVertex3f(x0Outter,	y0Outter,	zCoord + zSquareLen);
		}
	      glEnd();
	    }
	}
      enableLighting();
    }
  else
    {
      std::cerr<<"Error: (in drawCylinderNormals()) perimeterTess and or zTess less then minTesselation\n";
      exit(RANGE_ERROR);
    }
}


void drawEllipse(const float z, const float ratio, const float r, const int tess, const int wireframe)
{
  float x {}, y {}, theta {};
  const float a {r}, b {r/ratio};

  if(!wireframe)
    glBegin(GL_POLYGON);
  else
    glBegin(GL_LINE_LOOP);

  for(int iter {}; iter <= tess; ++iter)
    {      
      theta = iter / (float)tess * 2.0 * M_PI;
      x = a * cosf(theta), y = b * sinf(theta);
      glVertex3f(x, y, z);
    }
    glEnd();
}


void drawString(const float x, const float y, const float z, const char * s, void * font)
{
  glRasterPos3f(x, y, z);
  for(const char * c {s}; *c != '\0'; ++c)
    {
      glutBitmapCharacter(font, *c);
    }
}


void drawProjectile(const float x, const float y, const float z, const float ratio, const float theta, const float r,
		    const int tess, const int wireframe)
{
  glPushMatrix();               // Save current coordinate's
  glTranslatef(x, y, 0.0);
  glRotatef(theta, 0, 0, 1);
  //  glColor3f(rC, gC, bC);
  drawEllipse(z, ratio,  r, tess, wireframe);
  glPopMatrix();                // Restore coordinate state 
}


void drawTrajectory(const std::vector<float> & trajectory)
{
  using namespace vectorAxis;
  //  glColor3f(rC, gC, bC);
  glBegin(GL_LINE_STRIP);
  for(unsigned iter {}; iter < trajectory.size(); iter += dimensions)
    {
      glVertex3f(trajectory[iter + xIndex], trajectory[iter + yIndex], trajectory[iter + zIndex]);
    }

  glEnd();
}


void drawVector(const float x, const float y, const float z, float a, float b, float c, const float scale,
		const bool normalize)
{
  if(normalize)
    normalize3(a, b, c, normals::normScale);

  //  glColor3f(rC, gC, bC);
  glBegin(GL_LINES);
  glVertex3f(x, y, z);
  glVertex3f(x + (a * scale), y + (b * scale), z + (c * scale));
  glEnd();
}


void drawDotPotAtEndOfTrajectoryRainbow(const float xOffset, const float yOffset, const float zOffset,
					const float zDrawDepth, const float radius, const int tesselation,
					const bool wireframe)
{
  constexpr float zScale {1};		// We wan't a sphere.
  drawSphere(xOffset, yOffset, zOffset, zScale, radius, tesselation, tesselation, normals::drawNormals,
	     normals::normScale);
}


void normalize3(float & a, float & b, float & c, const float normScale)
{
  const float d {sqrtf(a*a + b*b + c*c)};
  a /= d;
  b /= d;
  c /= d;
  a *= normScale;
  b *= normScale;
  c *= normScale;
}


void normalize2(float & a, float & b, const float normScale)
{
  const float c {float(sqrt(a * a + b * b))};
  a /= c;
  b /= c;
  a *= normScale;
  b *= normScale;
}


void calcNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                const float yEnd, const float z, float & run, float & y, const float normScale)
{
  y = 1;
  run = (((yCenter - yStart) / (xCenter - xStart)) + ((yEnd - yCenter) / (xEnd - xCenter))) / -2;
  normalize2(run, y, normScale);
}


float pythagorean(const float x1, const float y1, const float x2, const float y2)
{
  float a {(x2 > x1) ? (x2 - x1) : (x1 - x2)}, b {(y2 > y1) ? (y2 - y1) : (y1 - y2)};
  return sqrtf(a*a + b*b);
}


bool vertLineCollision(const float xFuture, const float yFuture, const float r, const float xLn, const float yLnTop,
		       const float yLnBottom)
{
  bool ret {false};

  if(((xFuture - r) <= xLn && (xFuture + r) >= xLn) &&
     ((yFuture - r) <= yLnTop &&
      (yFuture + r) >= yLnBottom))
    {
      ret = true;
    }
  return ret;
}


bool vertLineLeftCollision(const float xFuture, const float yFuture, const float r, const float xLn,
			   const float yLnTop, const float yLnBottom, float & xNew)
{
  bool ret {false};


  if(vertLineCollision(xFuture, yFuture, r, xLn, yLnTop, yLnBottom))
    {
      xNew = xLn - r;
      ret = true;
    }

  return ret;
}


bool vertLineRightCollision(const float xFuture, const float yFuture, const float r, const float xLn,
			    const float yLnTop, const float yLnBottom, float & xNew)
{
  bool ret {false};

  if(vertLineCollision(xFuture, yFuture, r, xLn, yLnTop, yLnBottom))
    {
      xNew = xLn + r;
      ret = true;
    }

  return ret;
}


bool horizontalLineCollision(const float xFuture, const float  yFuture, const float r, const float xLnLeft,
			     const float xLnRight, const float yLn, float & yNew)
{
  bool ret {false};

  if((xFuture - r) >= xLnLeft && (xFuture + r) <= xLnRight)
    {
      if((yFuture - r) <= yLn)
	{
	  yNew = yLn + r;
	  ret = true;
	}
    }

  return ret;
}


GLuint loadTexture(const char * fileName)
{
  GLuint texture =
    SOIL_load_OGL_texture(fileName, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
  
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texture;
}
