#include <GL/glut.h>
#include <cmath>
#include "Camera.hpp"


float Camera::aspectRatio {1.0};
float Camera::width {1.0}, Camera::height {1.0};


void Camera::cameraUpdate(const int x, const int y)
{
  if(rotating)
    {
      this->x = fmod(x, FULL_DEG * ROT_MUL) + xOffset;
      this->y = fmod(y, FULL_DEG * ROT_MUL) + yOffset;
    }
  if(zooming)
    {				// Need scailing code?
      if(yZoomLast > y)
	{ // Eradicate that pesky "stickyness" when changing zooming direction!
	  yZoomLast = y;	
	  if((zoom -zoomFactorMouseLook) >= zoomInMin)
	    zoom -= zoomFactorMouseLook;
	}
      else
	{
	  yZoomLast = y;
	  if((zoom + zoomFactorMouseLook) <= zoomOutMax)
	    zoom += zoomFactorMouseLook;
	}
    }
}


void Camera::buttonEventUpdate(const int button, const int state, const int x, const int y)
{
  if(state == GLUT_DOWN)
    {
      switch(button)
        { // Some nice jump tables (probably turned into nested conditional jumps by the compiter though :'( .)
        case GLUT_LEFT_BUTTON:  // Start viewing angle rotation operation.
	  rotating = true;
	  xOffset -= fmod(x, FULL_DEG * ROT_MUL);
	  yOffset -= fmod(y, FULL_DEG * ROT_MUL);
          break;
	case GLUT_RIGHT_BUTTON:	// Start zoom operation.
	case MOUSE_SCROLL_1:
	case MOUSE_SCROLL_2:
	  if(button == GLUT_RIGHT_BUTTON)
	    {
	      zooming = true;
	      yZoomLast = y;
	    }
	  else
	    {			// We do this here and not in cameraUpdate because it is most convinient.	      
	      if(button == MOUSE_SCROLL_1)
		{
		  if((zoom -zoomFactorScrollLook) >= zoomInMin)
		    zoom -= zoomFactorScrollLook;
		}
	      else
		{
		  if((zoom + zoomFactorScrollLook) <= zoomOutMax)
		    zoom += zoomFactorScrollLook;
		}
	    }	  
        }
    }
  else
    {
      switch(button)
        {
        case GLUT_LEFT_BUTTON:  // Stop viewing angle rotation operation.
          rotating = false;
	  xOffset = xOffset + fmod(x, FULL_DEG * ROT_MUL);
	  yOffset = yOffset + fmod(y, FULL_DEG * ROT_MUL);
          break;
	case GLUT_RIGHT_BUTTON:	// Stop zoom operation.
	  {			// We don't need to check for MOUSE_SCROLL_1/2 because they don't set zooming!
	    zooming = false;
	  }
        }
    }
}

void Camera::applyCameraTransformations()
{
  glTranslatef(0.0, -camHeight, -zoom);	// Zoom
  glRotatef(y * Camera::ROT_DIV + initYRot, 1.0, 0, 0); // Rotate the scene
  glRotatef(x * Camera::ROT_DIV + initXRot, 0, 1.0, 0);
}


// Translate world at frogs will!
void Camera::applyWorldTranslations(const float x, const float y, const float z)
{
  glTranslatef(x, y ,z);
}


void Camera::reshape(const int widthNew, const int heightNew)
{
  glViewport(0, 0, widthNew, heightNew);
  aspectRatio = float(widthNew) / float(heightNew);
  width = widthNew;
  height = heightNew;
}

  
void Camera::orthoOn()
{
  using namespace initModelViewVars;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(left, right, bottom, top, nearVal, farVal);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
}

  
void Camera::perspectiveOn()
{
  using namespace initModelViewVars;


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fovy, aspectRatio, zNear, zFar);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LIGHTING);
  glEnable(light0);
  glLightfv(light0, GL_POSITION, light0Pos);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
}


void Camera::disableDepthTestAndLighting()
{
  glDisable(GL_DEPTH_TEST);
  glDisable(light0);
  glDisable(GL_LIGHTING);
}

void Camera::enableDepthTestAndLighting()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(light0);
  glEnable(GL_LIGHTING);
}


void Camera::disableLighting()
{
  glDisable(light0);
  glDisable(GL_LIGHTING);
}


void Camera::enableLighting()
{
  glEnable(light0);
  glEnable(GL_LIGHTING);
}

void Camera::resetAngleAndZoom()
{
  y = 0;
  x = 0;
  xOffset = 0;
  yOffset = 0;
  zoom = initZoom;
}
