#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <iostream>		// For cerr
#include "utility.hpp"		// For RANGE_ERROR

class Camera
{				// Member veriables. *~<===========================================================>~*
private:

  static constexpr int light0 {GL_LIGHT0};
  const float light0Pos [4] {1, 1, 1, 0};

  static float aspectRatio;
  
  bool rotating {};		// Are we currently rotating.
  bool zooming {};		// Are we currently zooming.
  static constexpr float FULL_DEG {360}; // :)
  static constexpr float ROT_MUL {4};    // We wan't slower (and smother rotation)
  static constexpr float ROT_DIV {1.0 / ROT_MUL};  
  /* X, y, xOffset and yOffset are not stored as accumulated values because they could over flow (we use the '%'
     operator on the rhs of the assignments.) */
  float x {};
  float y {};
  float xOffset {};
  float yOffset {};
  // Zooming
  static constexpr float zoomOutMax {3.0}; // Maximum out 
  static constexpr float zoomInMin {0.002};
  static constexpr float zoomFactorMouseLook {0.0005};
  static constexpr float zoomFactorScrollLook {0.00325};
  const float initXRot, initYRot, initZoom, camHeight;
  float zoom {zoomInMin};
  float yZoomLast {};		// We only zoom with the mouse in the y direction!  
  /* We simply put this here because it is convenient and there's no where better to put it.
     These values are not gauranteed to be correct for all machines but they are on mine. */
  static constexpr int MOUSE_SCROLL_1 {3}, MOUSE_SCROLL_2 {4}; 
  
public:
  static constexpr float fovy {75}, zNear {0.01}, zFar {100};
  static float width, height;
  
private:			// Member functions. *~<===========================================================>~*
  
public:
  Camera(const float xRot, const float yRot, const float zoom, const float cH) : initXRot(xRot), initYRot(yRot),
										 initZoom(zoom), camHeight(cH)
  {
    if(initZoom <= zoomOutMax && initZoom >= zoomInMin)
      {
	this->zoom = initZoom;
      }
    else
      {
	std::cerr<<"Error: initZoom out of range in Camera constructor\n";
	exit(RANGE_ERROR);
      }
  }

  
  void cameraUpdate(const int x, const int y); // Update OpenGl with camera state eg rotation & zooming
  // Update camera object state after button press (ScrollLook zooming is also done here.)
  void buttonEventUpdate(const int button, const int state, const int x, const int y);
  void applyCameraTransformations();
  void applyWorldTranslations(const float x, const float y, const float z);  
  static void reshape(const int width, const int height);
  void orthoOn();
  void perspectiveOn();
  void disableDepthTestAndLighting();
  void enableDepthTestAndLighting();
  void disableLighting();
  void enableLighting();
  void resetAngleAndZoom();
};

#endif
