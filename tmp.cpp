#include <GL/glut.h>
#include <cmath>
#include "include/Camera.hpp"


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
#include "include/Frogject.hpp"
#include "include/normals.hpp"
#include "include/Time.hpp"



bool Frogject::instantiated {false}; // This veriable is declared as static!



/* We are using an anymous namespace here because we had trouble integrating these variables into the Frogject class
   as inner classes. As long as no functions other then Frogject member functions are defined in this translation
   unit then the veriables in this namespace can effectively act as private members of Frogject (with the exception
   that we should only be allowed to create one instance of frogject because not all of the veriables are
   constant's.) */
namespace
{
  float angle {180}; // Frog should be facing away from the player (at least initially :) .)
  constexpr float xRot {}, yRot {1.0}, zRot {};

  namespace color
  {
    constexpr float materialAmbientBody [] {0.1, 0.4, 0.1, 0.98}, materialDiffuseBody [] {0.2, 1, 0.2, 0.98};
    constexpr float materialAmbientSclera [] {1, 1, 1, 0.99}, materialDiffuseSclera [] {1, 1, 1, 0.99};
    constexpr float materialAmbientPupil [] {0, 0, 0, 1}, materialDiffusePupil [] {0, 0, 0, 1};
    constexpr float materialSpecularBody [] {0.0, 1.0, 0.0, 0.98}, materialSpecularSclera [] {1.0, 1.0, 1.0, 0.99},
      materialSpecularPupil [] {0.0, 0.0, 0.0, 0.99};
    constexpr float materialShininess {127};

  }
  
  namespace torso
  {
    // Only used for normals in drawTorsoOrBraincase{} and drawTorsoOrBraincaseNormals()
    constexpr float xTorsoScale {1}, yTorsoScale {1}, zTorsoScale {1};
    constexpr float width {6.5}, length {10}, height {6};
    constexpr float pancakeFactor {1.5}; // The frog's sides will "jut out" by this much :)
    // Top
    constexpr float tFL [] {0.0, height, 0.0}; // Front left.
    constexpr float tFR [] {0.0, height, width}; // Front right.
    constexpr float tBL [] {length, height, 0.0}; // Back left.
    constexpr float tBR [] {length, height, width}; // Back right.
    // Bottom
    constexpr float bFL [] {0.0, 0.0, 0.0}; // Front left.
    constexpr float bFR [] {0.0, 0.0, width}; // Front right.
    constexpr float bBL [] {length, 0.0, 0.0}; // Back left.
    constexpr float bBR [] {length, 0.0, width}; // Back right.
    // Sides
    // Front left
    constexpr float sFL [] {tFL[vectorAxis::xIndex] - pancakeFactor, tFL[vectorAxis::yIndex] / 2,
	tFL[vectorAxis::zIndex]};
    constexpr float sFR [] {sFL[vectorAxis::xIndex], tFR[vectorAxis::yIndex] / 2, tFR[vectorAxis::zIndex]};
    // Back left
    constexpr float sBL [] {tBL[vectorAxis::xIndex] + pancakeFactor, tBL[vectorAxis::yIndex] / 2,
	tBL[vectorAxis::zIndex]};
    constexpr float sBR [] {sBL[vectorAxis::xIndex], tBR[vectorAxis::yIndex] / 2, tBR[vectorAxis::zIndex]};
    // Left front
    constexpr float sLF [] {tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex] / 2,
	tFL[vectorAxis::zIndex] - pancakeFactor};
    constexpr float sLB [] {tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex] / 2, sLF[vectorAxis::zIndex]};
    // Right front
    constexpr float sRF [] {tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex] / 2,
	tFR[vectorAxis::zIndex] + pancakeFactor};
    constexpr float sRB [] {tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex] / 2, sRF[vectorAxis::zIndex]};
  }

  namespace head
  { // Note: THE BRAINCASE OF THE HEAD IS DRAW USING THE TORSO VALUES ONLY TRANSLATED AND SCALEED!
    // Braincase -----------------------------------------------------------------------------------------------------
    // Dimensions
    // Rotations
    // Translations
    // Since we are scaleing the shape used for the torso there is no need to explicitly use any torso values here.
    constexpr float xBraincaseScale {0.18}, yBraincaseScale {0.8}, zBraincaseScale {0.9};
    constexpr float xBraincaseTran {-torso::length * xBraincaseScale},
      yBraincaseTran {(torso::height - (torso::height * yBraincaseScale)) / 1.25},
      zBraincaseTran {(torso::width - (torso::width * zBraincaseScale)) /2};
    
    // Mouth ---------------------------------------------------------------------------------------------------------
    // Dimensions
    // The ratio in height of the top and bottom mouth sections when compared to the braincase
    constexpr float yMouthSectionScale {0.25};
    constexpr float xMouthSectionScale {1.5}; // Simmilar to above
    constexpr float mouthLength
    {(torso::tFL[vectorAxis::xIndex] + torso::tBL[vectorAxis::xIndex]) * xBraincaseScale * xMouthSectionScale},
      mouthHeight
    {(torso::tFL[vectorAxis::yIndex] + torso::bFL[vectorAxis::yIndex]) * yBraincaseScale * yMouthSectionScale},
      mouthWidth {(torso::tFL[vectorAxis::zIndex] + torso::tFR[vectorAxis::zIndex]) * zBraincaseScale};
    // Rotations
    constexpr float initialAngleBottomMouth {195};
    float angleBottomMouth {initialAngleBottomMouth};
    constexpr float xRot {}, yRot {}, zRot {1};
    // Translations
    constexpr float xMouthTopTran {-mouthLength}, yMouthTopTran {mouthHeight}, zMouthTopTran {};
    constexpr float xMouthBottomTran {}, yMouthBottomTran {mouthHeight}, zMouthBottomTran {};

    // Eyeballs ------------------------------------------------------------------------------------------------------
    //    constexpr float rOuterEye {0.95}, gOuterEye {0.95}, bOuterEye {1.0}; // Color of Sclera
    // Dimensions
    constexpr float rOuter {mouthHeight * 2.3 / 2}, rInner {rOuter * 0.7};
    // Rotations
    // Translations
    constexpr float xStart {0.0}, yStart {0.0}, zStart {0.0};
    constexpr float zScale {1.0};	// Our eye's are round :)
    constexpr float eyeSunkenness {mouthLength * 0.1}, eyeHeight {rOuter /2},
      eyeSeperation {rOuter * 0.030}, pupleProtrusionFactor {- (rOuter * 0.08)};
    constexpr float xEyeLTran {mouthLength + eyeSunkenness}, yEyeLTran {mouthHeight + eyeHeight},
      zEyeLTran {rOuter + eyeSeperation};
    constexpr float xEyeLInnerTran {-(rOuter - rInner) + pupleProtrusionFactor}, yEyeLInnerTran {}, zEyeLInnerTran {};
    constexpr float xEyeRTran {xEyeLTran}, yEyeRTran {yEyeLTran}, zEyeRTran {mouthWidth -zEyeLTran};
    constexpr float xEyeRInnerTran {-(rOuter - rInner) + pupleProtrusionFactor}, yEyeRInnerTran {}, zEyeRInnerTran {};
  }

  
  namespace legs
  { /* All segment's of the legs should be rotated by this after translating to the appropriate initial position for
       the legs e.g., translating to where the front legs should be drawn */
    // Dimensions
    // Rotation
    float angleABS {90};
    constexpr float xRotABS {}, yRotABS {1}, zRotABS {};
    // Translations
    // Circle Translations
    constexpr float xStart {0.0}, yStart {0.0}, zStart {0.0}; // Start of circles used at top of legs.
    
    namespace front
    {
      // Shoulders ---------------------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthShoulder {1.8}, heightShoulder {1.25}, lengthShoulder {-2.5}; // For shoulder square
      constexpr float rShoulders {(widthShoulder /2) + widthShoulder /10}; // For shoulder circle
      // Rotations
      // Dynamic rotations (main shoulder movement)
      constexpr float initialAngleShoulders {-65};
      float angleShoulders {initialAngleShoulders};
      constexpr float xZRot {1}, yZRot {}, zZRot {}; // Sholder only rotate's around z!
      // Static rotations (outwars shoulder splay :) )
      constexpr float angleShoulderLSplay {14};
      constexpr float angleShoulderRSplay {-angleShoulderLSplay};
      constexpr float xRot {}, yRot {1}, zRot {0.5};
      // Translations
      // For shoulder square (We want to start drawing the shoulder's a little inside the frog!)
      constexpr float xStartRatio {8.2}, yStartRatio {10}, zStartRatio {10};
      constexpr float xShoulderTranL {torso::length / xStartRatio}; // Left shoulder start.
      constexpr float yShoulderTranL {torso::height / yStartRatio};
      constexpr float zShoulderTranL {torso::width / zStartRatio};
      /* The right shoulder is drawn from the same point on the leg model so we need to make adjustments to it's
	 starting position :'( */
      const float xShoulderTranR // Right shoulder start.
      {xShoulderTranL + (sinf(SphericalCoords::degToRad(angleShoulderRSplay)) * widthShoulder * yRot)};
      const float yShoulderTranR
      {yShoulderTranL - (sinf(SphericalCoords::degToRad(angleShoulderRSplay)) * widthShoulder * zRot)};
      const float zShoulderTranR {torso::width + widthShoulder /2};
      constexpr float zShoulderScale {abs(lengthShoulder) / 2.35}; // For shoulder circle
      constexpr float xShoulderSphereTran {widthShoulder /2}, yShoulderSphereTran {heightShoulder /2},
	zShoulderSphereTran {}; // For shoulder circle

      // Lower leg bit's idk :) --------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthLowerLeg {1.5}, heightLowerLeg {1}, lengthLowerLeg {lengthShoulder};
      // Rotations
      constexpr float initialAngleLowerLeg {-initialAngleShoulders - 30};
      float angleLowerLeg {initialAngleLowerLeg};      
      // Translations
      constexpr float xLowerLegTran {(widthShoulder - widthLowerLeg) /2}, yLowerLegTran {0.0},
	zLowerLegTran {lengthShoulder};
    }

    namespace back
    {
      // Shoulders ---------------------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthShoulder {2.2}, heightShoulder {1.35}, lengthShoulder {-4.2}; // For shoulder square
      constexpr float rShoulders {(widthShoulder /2) + widthShoulder /10}; // For shoulder circle
      // Rotations
      // Dynamic rotations (main shoulder movement)
      constexpr float initialAngleShoulders {-40};
      float angleShoulders {initialAngleShoulders};
      constexpr float xZRot {1}, yZRot {}, zZRot {}; // Sholder only rotate's around z!
      // Static rotations (outwars shoulder splay :) )
      constexpr float angleOneShoulderLSplay {33};
      constexpr float angleOneShoulderRSplay {-angleOneShoulderLSplay};
      constexpr float angleTwoShoulderLSplay {-20};
      constexpr float angleTwoShoulderRSplay {-angleTwoShoulderLSplay};
      constexpr float xRotOne {}, yRotOne {}, zRotOne {1.0};
      constexpr float xRotTwo {}, yRotTwo {1.0}, zRotTwo {};
      // Translations
      // For shoulder square (We want to start drawing the shoulder's a little inside the frog!)
      constexpr float xStartRatio {16}, yStartRatio {10}, zStartRatio {10};
      constexpr float xShoulderTranL {torso::length - torso::length / xStartRatio}; // Left shoulder start.
      constexpr float yShoulderTranL {torso::height / yStartRatio};
      constexpr float zShoulderTranL {torso::width / zStartRatio};
      /* Sadly I had to put a couple of magic number's in here because I don't quite know how to make it work with
	 the multiple rotations. I'm sure I could figure it out if I spen't more time on it, but i've already spen't
	 way to much time on this frog :'( */
      // Right shoulder start.
      const float xShoulderTranR {float(-0.44) + xShoulderTranL -
	  (sinf(SphericalCoords::degToRad(angleOneShoulderRSplay)) * widthShoulder * yRotTwo)};
      const float yShoulderTranR {float(-0.175) + yShoulderTranL -
	  (sinf(SphericalCoords::degToRad(angleOneShoulderRSplay)) * widthShoulder * yRotTwo)};
      const float zShoulderTranR {torso::width + widthShoulder /2};
      constexpr float zShoulderScale {abs(lengthShoulder) / 3.75}; // For shoulder circle
      constexpr float xShoulderSphereTran // For shoulder circle
      {widthShoulder /2}, yShoulderSphereTran {heightShoulder /2}, zShoulderSphereTran {};

      // Lower leg bit's idk :) --------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthLowerLeg {1.6}, heightLowerLeg {1.15}, lengthLowerLeg {lengthShoulder * 0.8};
      // Rotations
      // Dynamic rotations (main leg movement's)
      constexpr float initialAngleLowerLeg {-initialAngleShoulders + 40};
      float angleLowerLeg {initialAngleLowerLeg};
      // Static rotations (flip horizon so leg moves into itself at joint.)
      constexpr float angleFlipHorizon {180};
      constexpr float xRotFlipHorizon {}, yRotFlipHorizon {}, zRotFlipHorizon {1};
      // Translations
      constexpr float xLowerLegTran {(-(widthShoulder - widthLowerLeg) /2) + widthShoulder},
	yLowerLegTran {heightShoulder}, zLowerLegTran {lengthShoulder};

      // Lower lower leg bit's :) ------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthLowerLowerLeg {widthLowerLeg}, heightLowerLowerLeg {heightLowerLeg},
	lengthLowerLowerLeg {lengthShoulder};
      // Rotations
      // Dynamic rotations (main leg movement's)
      constexpr float initialAngleLowerLowerLeg {-initialAngleShoulders + 60};
      float angleLowerLowerLeg {initialAngleLowerLowerLeg};
      // Static rotations
      // The horizon should be "fliped" using the static rotation values from lower leg bit's.
      // Translations
      constexpr float xLowerLowerLegTran {widthLowerLeg}, yLowerLowerLegTran {heightLowerLeg * 0.8},
	zLowerLowerLegTran {lengthLowerLeg};
    }

    namespace feet
    {
      // Feet --------------------------------------------------------------------------------------------------------
      // Dimensions
      constexpr float widthFeet {front::widthLowerLeg}, heightFeet {front::heightLowerLeg}, lengthFeet {-0.5};
      // Rotations
      // Translations

      // Toes --------------------------------------------------------------------------------------------------------
      // Dimensions
      constexpr float rToes {0.25};
      // Rotations
      constexpr float angleToeOne {20}, angleToeTwo {0.0}, angleToeThree {-angleToeOne};
      constexpr float xToeRot {}, yToeRot {1}, zToeRot {};
      // Translations
      constexpr float zToeScale {3.25};
      // All toes to be translated by these values.
      constexpr float xToeTranABS {}, yToeTranABS {heightFeet + (rToes) / 0.75}, zToeTranABS {lengthFeet/2};
      /* r/2 and not r because the end connected to the foot will move into the foot a bit when we rotate the toe
	 because drawSphere draws the sphere relative to it's center (Which is in front of the foot.) */
      constexpr float xToeMinorOffset {rToes /2};
      constexpr float xToeOneTran {xToeMinorOffset}, yToeOneTran {0.0}, zToeOneTran {0.0},
	xToeTwoTran {xToeOneTran + (widthFeet /2) - xToeMinorOffset}, yToeTwoTran {yToeOneTran},
	zToeTwoTran {zToeOneTran},
	xToeThreeTran {widthFeet - xToeMinorOffset}, yToeThreeTran {}, zToeThreeTran {};

      namespace front
      {
	// Feet ------------------------------------------------------------------------------------------------------
	// Dimensions
	// Rotations
	constexpr float initalAngleFeet {-legs::front::initialAngleLowerLeg};
	float angleFeet {initalAngleFeet};
	// Translations

	// Toes ------------------------------------------------------------------------------------------------------
	// Dimensions
	// Rotations
	constexpr float initialAngleToeABS {90}; // All toes to be rotated by this value
	float angleToeABS {initialAngleToeABS};  // Toes must move a bit too :)
	constexpr float xToeRotABS {1}, yToeRotABS {}, zToeRotABS {}; // To be used with initialAngleToeABS
	// Translations
	constexpr float xFeetTran {0.0}, yFeetTran {legs::front::heightLowerLeg / 4},
	  zFeetTran {legs::front::lengthLowerLeg};
      }

      namespace back
      {
	// Feet ------------------------------------------------------------------------------------------------------
	// Dimensions
	// Rotations
	constexpr float initalAngleFeet {-legs::back::initialAngleLowerLeg};
	float angleFeet {initalAngleFeet};
	// Translations

	// Toes ------------------------------------------------------------------------------------------------------
	// Dimensions
	// Rotations
	constexpr float initialAngleToeABS {90}; // All toes to be rotated by this value
	float angleToeABS {initialAngleToeABS};  // Toes must move a bit too :)
	constexpr float xToeRotABS {1}, yToeRotABS {}, zToeRotABS {}; // To be used with initialAngleToeABS
	// Translations
	constexpr float xFeetTran {0.0}, yFeetTran {legs::back::heightLowerLowerLeg / 2.5},
	  zFeetTran {legs::back::lengthLowerLowerLeg};
      }
    }
  }
}


void Frogject::drawTorsoOrBraincase(const vectorAxis::tCoords xNormScale, const vectorAxis::tCoords yNormScale,
				    const vectorAxis::tCoords zNormScale,
				    const vectorAxis::tCoords normDrawScaleScale)
{
  using namespace torso;

  {
    using namespace color;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientBody);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseBody);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecularBody);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
  }
  /* Man we had to redo all this crap because we didn't realise that using GL_X_STRIPE doesn't work well with
     glNormal3f(), we had to redo drawBox as well >:(. */
  const float xOffset {normals::normalLen * normScale * (1 / xNormScale)},
    yOffset {normals::normalLen * normScale * (1 / yNormScale)},
      zOffset {normals::normalLen * normScale * (1 / zNormScale)};
      // Draw top front back and bottom of frog's torso or braincase.
  glBegin(GL_QUADS);
    glNormal3f(bFL[vectorAxis::xIndex] -xOffset, bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]); // Draw bottom front side.
    glNormal3f(bFR[vectorAxis::xIndex] -xOffset, bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
  glNormal3f(sFR[vectorAxis::xIndex] -xOffset, sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
  glNormal3f(sFL[vectorAxis::xIndex] -xOffset, sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glNormal3f(sFR[vectorAxis::xIndex] -xOffset, sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]); // Draw top front side.
  glNormal3f(sFL[vectorAxis::xIndex] -xOffset, sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glNormal3f(tFL[vectorAxis::xIndex] -xOffset, tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
  glNormal3f(tFR[vectorAxis::xIndex] -xOffset, tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);

    glNormal3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex] + yOffset, tFL[vectorAxis::zIndex]);
  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]); // Draw top of frog torso.

    glNormal3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex] + yOffset, tFR[vectorAxis::zIndex]);
  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);

    glNormal3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex] + yOffset, tBR[vectorAxis::zIndex]);
  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);

    glNormal3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex] + yOffset, tBL[vectorAxis::zIndex]);
  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
  glNormal3f(tBR[vectorAxis::xIndex] + xOffset, tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]); // Draw top back side.
  glNormal3f(tBL[vectorAxis::xIndex] + xOffset, tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
  glNormal3f(sBL[vectorAxis::xIndex] + xOffset, sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
  glNormal3f(sBR[vectorAxis::xIndex] + xOffset, sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glNormal3f(sBL[vectorAxis::xIndex] + xOffset, sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]); // Draw bottom back side.
  glNormal3f(sBR[vectorAxis::xIndex] + xOffset, sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glNormal3f(bBR[vectorAxis::xIndex] + xOffset, bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
  glNormal3f(bBL[vectorAxis::xIndex] + xOffset, bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
  glNormal3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex] - yOffset, bBR[vectorAxis::zIndex]);
  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]); // Draw bottom.
  glNormal3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex] - yOffset, bBL[vectorAxis::zIndex]);
  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
  glNormal3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex] - yOffset, bFL[vectorAxis::zIndex]);
  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
  glNormal3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex] -yOffset, bFR[vectorAxis::zIndex]);
  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);

  // Draw left and right sides of frog's torso or braincase.
    glNormal3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]); // Draw left bottom side.
  glNormal3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
  glNormal3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
  glNormal3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
  glNormal3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]); // Draw left top side.
  glNormal3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
  glNormal3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
  glNormal3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
  glNormal3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]); // Draw right bottom side.
  glNormal3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
  glNormal3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
  glNormal3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
  glNormal3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]); // Draw right top side.
  glNormal3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
  glNormal3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
  glNormal3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);

  glEnd();
  // Fill in corners.
  glBegin(GL_TRIANGLES);
  // Front left.
    glNormal3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
  glNormal3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glNormal3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
  glNormal3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
  glNormal3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
  glNormal3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex] - zOffset);
  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);

  // Front right.
    glNormal3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
  glNormal3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
  glNormal3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
  glNormal3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
  glNormal3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
  glNormal3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex] + zOffset);
  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);

  // Back left.
    glNormal3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
  glNormal3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
  glNormal3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
  glNormal3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
  glNormal3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
  glNormal3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex] - zOffset);
  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);

  // Back right.
    glNormal3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
  glNormal3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glNormal3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
  glNormal3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
  glNormal3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
  glNormal3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex] + zOffset);
  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);

  glEnd();
  
  drawTorsoOrBraincaseNormals(xNormScale, yNormScale, zNormScale, normDrawScaleScale);
}


/* The normal lines drawn in this function arn't 100% corrent but we decided to to do them like we did because of
   time constraint's. They could be made compleatly accurate with an appropriate application of the pythagorean
   theorem. */
void Frogject::drawTorsoOrBraincaseNormals(const vectorAxis::tCoords xNormScale, const vectorAxis::tCoords yNormScale,
					   const vectorAxis::tCoords zNormScale,
					   const vectorAxis::tCoords normDrawScaleScale)
{
  using namespace torso;
  /*  const float xOffset {normals::normalLen * normScale * (1 / xNormScale)},
      yOffset {normals::normalLen * normScale * (1 / yNormScale)},
      zOffset {normals::normalLen * normScale * (1 / zNormScale)};*/
  if(normals::drawNormals)
    {
      const float xOffset {normals::normalLen * normals::normDrawScale * normDrawScaleScale * (1 / xNormScale)},
	yOffset {normals::normalLen * normals::normDrawScale * normDrawScaleScale *(1 / yNormScale)},
	  zOffset {normals::normalLen * normals::normDrawScale * normDrawScaleScale *(1 / zNormScale)};

	  //	  glColor3f(1, 1, 0);
	  glBegin(GL_LINES);
	  // Draw bottom front side.
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
	  glVertex3f(bFL[vectorAxis::xIndex] -xOffset, bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex] -xOffset, bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFR[vectorAxis::xIndex] -xOffset, sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex] -xOffset, sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  /* We draw the next four vertices that are duplicates of the above four to make it easier to correct the
	     normals in the furture */
	  // Draw top front side.
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFR[vectorAxis::xIndex] -xOffset, sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex] -xOffset, sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
	  glVertex3f(tFL[vectorAxis::xIndex] -xOffset, tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex] -xOffset, tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
	  // Draw top of frog torso.
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex] + yOffset, tFL[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex] + yOffset, tFR[vectorAxis::zIndex]);
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex] + yOffset, tBR[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex] + yOffset, tBL[vectorAxis::zIndex]);
	  // Draw top back side.
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
	  glVertex3f(tBR[vectorAxis::xIndex] + xOffset, tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex] + xOffset, tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBL[vectorAxis::xIndex] + xOffset, sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex] + xOffset, sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  /* We draw the next four vertices that are duplicates of the above four to make it easier to correct the
	     normals int the furture */
	  // Draw bottom back side.
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBL[vectorAxis::xIndex] + xOffset, sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex] + xOffset, sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
	  glVertex3f(bBR[vectorAxis::xIndex] + xOffset, bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex] + xOffset, bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
	  // Draw bottom.
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex] - yOffset, bBR[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex] - yOffset, bBL[vectorAxis::zIndex]);
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex] - yOffset, bFL[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex] -yOffset, bFR[vectorAxis::zIndex]);
	  // Draw left and right sides of frog's torso or braincase.
	  // Draw left bottom side.
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
	  /* We draw the next four vertices that are duplicates of the above four to make it easier to correct the
	     normals int the furture */
	  // Draw left top side.
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex] - zOffset);
	  // Draw right bottom side.
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
	  /* We draw the next four vertices that are duplicates of the above four to make it easier to correct the
	     normals int the furture */
	  // Draw right top side.
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex] + zOffset);
	  // Front left.
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex]);
	  glVertex3f(tFL[vectorAxis::xIndex], tFL[vectorAxis::yIndex], tFL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex]);
	  glVertex3f(sFL[vectorAxis::xIndex], sFL[vectorAxis::yIndex], sFL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex]);
	  glVertex3f(sLF[vectorAxis::xIndex], sLF[vectorAxis::yIndex], sLF[vectorAxis::zIndex] - zOffset);
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex]);
	  glVertex3f(bFL[vectorAxis::xIndex], bFL[vectorAxis::yIndex], bFL[vectorAxis::zIndex] - zOffset);
	  // Front right.
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex]);
	  glVertex3f(tFR[vectorAxis::xIndex], tFR[vectorAxis::yIndex], tFR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex]);
	  glVertex3f(sFR[vectorAxis::xIndex], sFR[vectorAxis::yIndex], sFR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex]);
	  glVertex3f(sRF[vectorAxis::xIndex], sRF[vectorAxis::yIndex], sRF[vectorAxis::zIndex] + zOffset);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex]);
	  glVertex3f(bFR[vectorAxis::xIndex], bFR[vectorAxis::yIndex], bFR[vectorAxis::zIndex] + zOffset);
	  // Back left.
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex]);
	  glVertex3f(tBL[vectorAxis::xIndex], tBL[vectorAxis::yIndex], tBL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex]);
	  glVertex3f(sBL[vectorAxis::xIndex], sBL[vectorAxis::yIndex], sBL[vectorAxis::zIndex] - zOffset);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex]);
	  glVertex3f(sLB[vectorAxis::xIndex], sLB[vectorAxis::yIndex], sLB[vectorAxis::zIndex] - zOffset);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex]);
	  glVertex3f(bBL[vectorAxis::xIndex], bBL[vectorAxis::yIndex], bBL[vectorAxis::zIndex] - zOffset);
	  // Back right.
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex]);
	  glVertex3f(tBR[vectorAxis::xIndex], tBR[vectorAxis::yIndex], tBR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex]);
	  glVertex3f(sBR[vectorAxis::xIndex], sBR[vectorAxis::yIndex], sBR[vectorAxis::zIndex] + zOffset);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex]);
	  glVertex3f(sRB[vectorAxis::xIndex], sRB[vectorAxis::yIndex], sRB[vectorAxis::zIndex] + zOffset);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex]);
	  glVertex3f(bBR[vectorAxis::xIndex], bBR[vectorAxis::yIndex], bBR[vectorAxis::zIndex] + zOffset);
	  glEnd();
	  //	  glColor3f(0, 0, 1);
    }
}


void Frogject::drawLegs()
{  
  {				// Relocate this latter!
    Time t {};
    static bool lOrR {true};
    t.update();
    static float time {t.getT()};

    if(t.getT() - time > 0.003)
      {
	time = t.getT();
	if(lOrR)
	  {			// front
	    legs::front::angleShoulders -= 0.90;
	    legs::front::angleLowerLeg -= 0.35;
	    legs::feet::front::angleFeet += 0.45;
	    legs::feet::front::angleToeABS += 0.20;
	    // back
	    legs::back::angleShoulders -= 0.75;
	    legs::back::angleLowerLeg -= 0.35;
	    legs::back::angleLowerLowerLeg -= 0.35;
	    legs::feet::back::angleFeet += 0.45;
	    legs::feet::back::angleToeABS += 0.20;
	    // head
	    head::angleBottomMouth += 0.20; // mouth	    
	    if(legs::front::angleShoulders < -150)
	    lOrR = false;
	  }
	else
	  {			// front
	    legs::front::angleShoulders += 0.90;
	    legs::front::angleLowerLeg += 0.35;
	    legs::feet::front::angleFeet -= 0.45;
	    legs::feet::front::angleToeABS -= 0.20;
	    // back
	    legs::back::angleShoulders += 0.75;
	    legs::back::angleLowerLeg += 0.35;
	    	    legs::back::angleLowerLowerLeg += 0.35;
	    legs::feet::back::angleFeet -= 0.45;
	    legs::feet::back::angleToeABS -= 0.20;
	    // head
	    head::angleBottomMouth -= 0.20;
	    if(legs::front::angleShoulders > -45)
	    lOrR = true;
	  }
      }
  }

  drawFrontLeg(true);
  drawFrontLeg(false);
  drawBackLeg(true);
  drawBackLeg(false);
}


void Frogject::drawFrontLeg(const bool leftLeg)
{
  glPushMatrix();
  {
    using namespace legs::front;
    glPushMatrix();
    {				// Draw left front leg shoulder.
      if(leftLeg)		// Do right translation for leg.
	glTranslatef(xShoulderTranL, yShoulderTranL, zShoulderTranL);
      else
	glTranslatef(xShoulderTranR, yShoulderTranR, zShoulderTranR);

      // Make sure leg's are facing the right way.
      glRotatef(legs::angleABS, legs::xRotABS, legs::yRotABS, legs::zRotABS);
      
      if(leftLeg)		// Do right rotation for leg.
	glRotatef(angleShoulderLSplay, legs::front::xRot, legs::front::yRot, legs::front::zRot);
      else
	glRotatef(angleShoulderRSplay, legs::front::xRot, legs::front::yRot, legs::front::zRot);

      drawFrontShoulder();
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::drawFrontShoulder()
{
  using namespace legs::front;
  using namespace legs;
  using normals::drawNormals;
  
  glRotatef(angleShoulders, xZRot, yZRot, zZRot);
  
  glPushMatrix();
  {			 // Draw nice spherical frog shoulder "cap" (Frog has beautiful round shoulders :) .)
    glTranslatef(xShoulderSphereTran, yShoulderSphereTran, zShoulderSphereTran);
    drawSphere(xStart, yStart, zStart, zShoulderScale, rShoulders, t.get(), t.get(), drawNormals, normScale);
  }
  glPopMatrix();
  
  drawBox(widthShoulder, heightShoulder, lengthShoulder, drawNormals, normScale); // Draw shoulder box
  drawFrontLowerLeg();
}


void Frogject::drawFrontLowerLeg()
{
  using namespace legs::front;
  
  glPushMatrix();
  {				// Draw left front lower leg.
    glTranslatef(xLowerLegTran, yLowerLegTran, zLowerLegTran);
    glRotatef(angleLowerLeg, xZRot, yZRot, zZRot);      
    drawBox(widthLowerLeg, heightLowerLeg, lengthLowerLeg, normals::drawNormals, normScale);

    drawFrontFeet();
  }
  glPopMatrix();
}


void Frogject::drawFrontFeet()
{
  using namespace legs::front;
  glPushMatrix();
  {			// Draw frog feet :) :) :)
    using namespace legs::feet;
    using namespace legs::feet::front;
    
    glTranslatef(xFeetTran, yFeetTran, zFeetTran);
    glRotatef(angleFeet, xZRot, yZRot, zZRot);
    drawBox(widthFeet, heightFeet, lengthFeet, normals::drawNormals, normScale);

    drawFrontToes();
  }
  glPopMatrix();
}


void Frogject::drawFrontToes()
{
  using namespace legs;
  using namespace feet;
  using namespace legs::feet::front;
  using normals::drawNormals;
  
  glPushMatrix();
  {			// Draw frog toes. :) :) :)
    glTranslatef(xToeTranABS, yToeTranABS, zToeTranABS);
    glRotatef(angleToeABS, xToeRotABS, yToeRotABS, zToeRotABS);
    glPushMatrix();
    {			// Draw first toe.
      glTranslatef(xToeOneTran, yToeOneTran, zToeOneTran);
      glRotatef(angleToeOne, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
	    
    glPushMatrix();
    {			// Draw second...
      glTranslatef(xToeTwoTran, yToeTwoTran, zToeTwoTran);
      glRotatef(angleToeTwo, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
	    
    glPushMatrix();
    {
      glTranslatef(xToeThreeTran, yToeThreeTran, zToeThreeTran);
      glRotatef(angleToeThree, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::drawBackLeg(const bool leftLeg)
{
  glPushMatrix();
  {
    using namespace legs::back;
    using namespace legs;
    glPushMatrix();
    {				// Draw left back leg shoulder.
      if(leftLeg)
	glTranslatef(xShoulderTranL, yShoulderTranL, zShoulderTranL);
      else
	glTranslatef(xShoulderTranR, yShoulderTranR, zShoulderTranR);

      // Make sure leg's are facing the right way
      glRotatef(legs::angleABS, legs::xRotABS, legs::yRotABS, legs::zRotABS);
      
      if(leftLeg)
	{
	  glRotatef(angleOneShoulderLSplay, xRotOne, yRotOne, zRotOne);
	  glRotatef(angleTwoShoulderLSplay, xRotTwo, yRotTwo, zRotTwo);
	}
      else
	{
	  glRotatef(angleOneShoulderRSplay, xRotOne, yRotOne, zRotOne);
	  glRotatef(angleTwoShoulderRSplay, xRotTwo, yRotTwo, zRotTwo);
	}
      
      glRotatef(angleShoulders, xZRot, yZRot, zZRot);

      drawBackShoulder();
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::drawBackShoulder()
{
  using namespace legs::back;
  using namespace legs;
  using normals::drawNormals;
  
  glPushMatrix();
  {			 // Draw nice spherical frog shoulder "cap" (Frog has beautiful round shoulders :) .)
    glTranslatef(xShoulderSphereTran, yShoulderSphereTran, zShoulderSphereTran);
    drawSphere(xStart, yStart, zStart, zShoulderScale, rShoulders, t.get(), t.get(), drawNormals, normScale);
  }
  glPopMatrix();
  drawBox(widthShoulder, heightShoulder, lengthShoulder, normals::drawNormals, normScale); // Draw shoulder box
  drawBackLowerLeg();
}


void Frogject::drawBackLowerLeg()
{
  using namespace legs::back;
  using namespace legs;
  
  glPushMatrix();
  {				// Draw left back lower leg.
    glTranslatef(xLowerLegTran, yLowerLegTran, zLowerLegTran);
    glRotatef(angleFlipHorizon, xRotFlipHorizon, yRotFlipHorizon, zRotFlipHorizon); // Flip horizon (180).
    glRotatef(angleLowerLeg, xZRot, yZRot, zZRot);
    drawBox(widthLowerLeg, heightLowerLeg, lengthLowerLeg, normals::drawNormals, normScale);

    drawBackLowerLowerLeg();
  }
  glPopMatrix();
}


void Frogject::drawBackLowerLowerLeg()
{
  using namespace legs::back;
  using namespace legs;
  
  glPushMatrix();
  {				// Draw back lower lower leg ;)
    glTranslatef(xLowerLowerLegTran, yLowerLowerLegTran, zLowerLowerLegTran);
    glRotatef(angleFlipHorizon, xRotFlipHorizon, yRotFlipHorizon, zRotFlipHorizon); // Flip horizon back!
    glRotatef(angleLowerLowerLeg, xZRot, yZRot, zZRot);
    drawBox(widthLowerLowerLeg, heightLowerLowerLeg, lengthLowerLowerLeg, normals::drawNormals, normScale);

    drawBackFeet();
  }
  glPopMatrix();
}


void Frogject::drawBackFeet()
{
  using namespace legs::back;
  using namespace legs;
  glPushMatrix();
  {			// Draw frog feet :) :) :)
    using namespace feet;
    using namespace legs::feet::back;
    
    glTranslatef(xFeetTran, yFeetTran, zFeetTran);
    glRotatef(angleFeet, xZRot, yZRot, zZRot);	  
    drawBox(widthFeet, heightFeet, lengthFeet, normals::drawNormals, normScale);

    drawBackToes();
  }
  glPopMatrix();
}


void Frogject::drawBackToes()
{
  using namespace legs;
  using namespace feet;
  using namespace legs::feet::back;
  using normals::drawNormals;
  glPushMatrix();
  {			// Draw frog toes. :) :) :)
    glTranslatef(xToeTranABS, yToeTranABS, zToeTranABS);
    glRotatef(angleToeABS, xToeRotABS, yToeRotABS, zToeRotABS);
    glPushMatrix();
    {			// Draw first toe.
      glTranslatef(xToeOneTran, yToeOneTran, zToeOneTran);
      glRotatef(angleToeOne, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
	    
    glPushMatrix();
    {			// Draw second...
      glTranslatef(xToeTwoTran, yToeTwoTran, zToeTwoTran);
      glRotatef(angleToeTwo, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
	    
    glPushMatrix();
    {
      glTranslatef(xToeThreeTran, yToeThreeTran, zToeThreeTran);
      glRotatef(angleToeThree, xToeRot, yToeRot, zToeRot);
      drawSphere(xStart, yStart, zStart, zToeScale, rToes, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::drawHead()
{
  glPushMatrix();
  {
    using namespace head;
    glTranslatef(xBraincaseTran, yBraincaseTran, zBraincaseTran);

    glPushMatrix();
    {				// Braincase
      glScalef(xBraincaseScale, yBraincaseScale, zBraincaseScale);
      drawTorsoOrBraincase(xBraincaseScale, yBraincaseScale, zBraincaseScale, normScale);
    }
    glPopMatrix();

    glPushMatrix();
    {				// Top of mouth
      glTranslatef(xMouthTopTran, yMouthTopTran, zMouthTopTran);
      drawBox(mouthLength, mouthHeight, mouthWidth, normals::drawNormals, normScale);
      drawEye(true);		// Draw left eye
      drawEye(false);		// Draw right eye
    }
    glPopMatrix();

    glPushMatrix();
    {				// Bottom of mouth
      using namespace color;
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientBody);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseBody);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecularBody);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
      
      glTranslatef(xMouthBottomTran, yMouthBottomTran, zMouthBottomTran);
      glRotatef(angleBottomMouth, head::xRot, head::yRot, head::zRot);
      drawBox(mouthLength, mouthHeight, mouthWidth, normals::drawNormals, normScale);
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::drawEye(const bool leftEye)
{
  using namespace::head;
  using namespace::color;
  using normals::drawNormals;
  glPushMatrix();
  {				// Outer eyeball (Sclera)
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientSclera);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseSclera);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecularSclera);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
    
    if(leftEye)
      glTranslatef(xEyeLTran, yEyeLTran, zEyeLTran);
    else
      glTranslatef(xEyeRTran, yEyeRTran, zEyeRTran);
    drawSphere(xStart, yStart, zStart, zScale, rOuter, t.get(), t.get(), drawNormals, normScale);

    glPushMatrix();
    {			// Inner eyeball (pupil)
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientPupil);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffusePupil);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecularPupil);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
      
      if(leftEye)
      glTranslatef(xEyeLInnerTran, yEyeLInnerTran, zEyeLInnerTran);
      else
	glTranslatef(xEyeRInnerTran, yEyeRInnerTran, zEyeRInnerTran);
      drawSphere(xStart, yStart, zStart, zScale, rInner, t.get(), t.get(), drawNormals, normScale);
    }
    glPopMatrix();
  }
  glPopMatrix();
}


void Frogject::draw()
{
  glPushMatrix();		// Draw frog
  
  // Draw frog in correct position :)
  glScalef(frogScale, frogScale, frogScale); // Draw frog at correct size :)
  glTranslatef(torso::length / 3.2, 0, torso::width / 2);	     // Center of frog isn't the front of it's head :)
  glRotatef(angle, xRot, yRot, zRot);
  drawTorsoOrBraincase(torso::xTorsoScale, torso::yTorsoScale, torso::zTorsoScale, normScale);
  drawLegs();
  drawHead();

  glPopMatrix();
}


std::vector<vectorAxis::tCoords> Frogject::getPos() const
{
  return std::vector<vectorAxis::tCoords> {pos, pos + (sizeof(pos) / sizeof(vectorAxis::tCoords))};
};


vectorAxis::tCoords Frogject::getHitRadius() const
{
  return hitRadius;
}


const SphericalCoords & Frogject::getVec() const
{
  return vector;
}


bool Frogject::isDead() const
{
  return dead;
}


bool Frogject::hasWon() const
{
  return won;
}


bool Frogject::isJumping() const
{
  return jumping;
}


bool Frogject::isFalling() const
{
  return falling;
}


bool Frogject::isOnLog() const
{
  return onLog;
}


void Frogject::setPos(const vectorAxis::tCoords x, const vectorAxis::tCoords y, const vectorAxis::tCoords z)
{
  using namespace vectorAxis;
  pos[xIndex] = x;
  pos[yIndex] = y;
  pos[zIndex] = z;
}


void Frogject::setXPos(const vectorAxis::tCoords x)
{
  pos[vectorAxis::xIndex] = x;
}


void Frogject::setYPos(const vectorAxis::tCoords y)
{
  pos[vectorAxis::yIndex] = y;
}


void Frogject::setZPos(const vectorAxis::tCoords z)
{
  pos[vectorAxis::zIndex] = z;
}


void Frogject::resetPos()
{
  using namespace vectorAxis;
  pos[xIndex] = posInit[xIndex];
  pos[yIndex] = posInit[yIndex];
  pos[zIndex] = posInit[zIndex];
}


void Frogject::setVec(const SphericalCoords & vector)
{
  this->vector = vector;
}


void Frogject::setVecAbs(const vectorAxis::tCoords xV, const vectorAxis::tCoords yV, const vectorAxis::tCoords zV)
{
  vector.setAbs(xV, yV, zV);
}


void Frogject::setVecInclinationRel(const vectorAxis::tCoords inclination)
{
  vector.setInclinationRel(inclination);
}


void Frogject::setVecAzimuthRel(const vectorAxis::tCoords azimuth)
{
  vector.setAzimuthRel(azimuth);
}


void Frogject::setVecMagRel(const vectorAxis::tCoords m)
{
  vector.setMagRel(m);
}


void Frogject::setVecMagAbs(const vectorAxis::tCoords m)
{
  vector.setMagAbs(m);
}


void Frogject::setDead(const bool dead)
{
  this->dead = dead;
}


void Frogject::setWon(const bool won)
{
  this->won = won;
}


void Frogject::setJumping(const bool jumping)
{
  this->jumping = jumping;
  if(jumping == true)
    onLog = false;		// Can't be on log if we are jumping!
  else
    falling = false;		// Can't be falling if we are not jumping (in our world.)
}


void Frogject::setFalling(const bool falling)
{
  this->falling = falling;
}


void Frogject::setOnLog(const bool onLog)
{
  this->onLog = onLog;
}
#include "include/Tesselation.hpp"

int Tesselation::t {Tesselation::tMin};
#include <iostream>
#include <sstream>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "include/world.hpp"
#include "include/utility.hpp"
#include "include/normals.hpp"


void init();
void display();
void drawSkyBox();
void draw3D();
void drawFrog();
void drawCartesianAxis();
void drawGround();
void drawRoad();
/* This function is a bit of a hack since we were having trouble generalizing the drawGrid()
function enough to be able to draw the road properly :'(. */
void drawTopRoadTex();
void drawCars();
void drawRiverBed();
void drawRiver();
// Calculate's normal based on average of m's and calls glNormal3f (can't figure out how to do it the other way, therefore this way is better :) .)
void registerVectorNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                      const float yEnd, const float z);
void drawWaveNormals();
// Calculate's normal based on average of m's and draws it.
void drawVectorNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                      const float yEnd, const float z);
//void updateRiverVertices();
void drawLogs();
void drawHud();
//void reshape(const int width, const int height);
void mouseMotion(const int x, const int y);
void mouseButt(const int button, const int state, const int x, const int y);
void keyboardDown(const unsigned char key, const int x, const int y);
void keyboardUp(const unsigned char key, const int x, const int y);
void specialKeyDown(const int key, const int x, const int y);
void specialKeyUp(const int key, const int x, const int y);
void update();
void updateRiverVertices();
void addAdditionalWaves(const float a, const float k, const float w);
float waveY(const float A, const float k, const float w, const float x);
void updateLogs();
/* Randomely adds new cars to world::dynamicState::cars::cars and update's cars
   already in the vector (removing those that have gone off of the road) */
void updateCars();
void updateFrogVec();		// Update frog vector based on key presses
/* would probably be good to change this so that it uses argument's but were already basically abusing global state haha :'( */
void resetVectorIfOutOfBounds();	// Resets frog::vector so that it's magnitude is within frog::restrictedModeMaxVectorMagnitude
/* Checks for object's colliding or interacting with a frog that is potentially not jumping. */
bool staticFrogCollisions();
void updateJumpingFrog();	// Update frog vector and coordinates when frog is jumping
void updateFrogOnLog();	// We need to update world::dynamicstate::frog::y when on a log :)
// Called either directly or indirectly by updateJumpingFrog() -------------------------------------------------- START:
bool groundSection1Collision(const float xFuture, const float yFuture, const float zFuture, const float radius, float & yNew);
void finishedJumping(const float yNew); // Does the clean up work after a jump
void finishRoadCollision(const float xNew, const float yFuture, const float zFuture, const float radius, const float yGround);
bool carsCollision(const float xFuture, const float yFuture, const float radius);
bool handleLogCollision(const float xFuture, const float yFuture, const float zFuture);
/* Returns true if (world::dynamicState::frog::x & y +/- world::dynamicState::frog::radius) have hit
   within world::dynamicState::river::logs::innerRadius of one of the logs */
bool logCollision(const float xFuture, const float yFuture, bool & sideOfLog, const float fRadius, const float lRadius);
/* Has (world::dynamicState::frog::x +/- world::dynamicState::frog::radius) hit within
   world::dynamicState::river::logs::innerRadius of a log? Returns true if so. */
bool logCollisionCenter(const float xFuture, const float yFuture, const float zFuture, float & yNew, const bool updateFrogOnThisLog);
bool riverCollision(const float xFuture, const float yFuture, const float radius, float & yNew);
bool groundSection3Collision(const float xFuture, const float yFuture, const float radius, float & yNew);
/* X and y are set to xFuture & yFuture if they are within the ranges (minX, maxX) & (minY, maxY) respectively. X and or y are
   not updated if xFuture and or yFuture are out of range. */
void handleBoarderCollision(const float xFuture, const float yFuture, const float zFuture,
					    const float radius, const float minX, const float maxX,
					    const float minY, const float maxY, const float minZ, const float maxZ);
// Called either directly or indirectly by updateJumpingFrog() ---------------------------------------------------- END:
void updateDeadFrogPos();	// Update the coordinates of the dead frog if it is in the lsd river
void updateFrogTrajectory();
// Check whether current computed trajectory line coordinate touches or is in an object
bool trajectoryLineIntersect(const float x, const float y, const float z, float & xTouch, float & yTouch, float & zTouch,
			     bool & trajectoryLineTouchingLog, bool & potentialLogHit);
// After we have detected that the trajectory line has intersected with an object we many need to do some extra generic work
void trajectoryLineIntersectExtraWork(bool & trajectoryLineTouchingLog, bool & potentialLogHit, bool & ret);
void updateHud();		// Update dynamicHudContent::fr and dynamicHudContent::ft


int main(int argc, char * argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(1000, 1000); // Sick of the tiny window :(
  glutCreateWindow("SuperSamGraphics :^)");
  init();

  glutReshapeFunc(world::dynamicState::camera.reshape);
  glutDisplayFunc(display);
  glutMotionFunc(mouseMotion);
  glutMouseFunc(mouseButt);
  glutKeyboardFunc(keyboardDown);
  glutKeyboardUpFunc(keyboardUp);
  glutSpecialFunc(specialKeyDown);		// For arrow keys
  glutSpecialUpFunc(specialKeyUp);
  glutIdleFunc(update);
  glutMainLoop();

  return EXIT_SUCCESS;
}


void init()
{
  using namespace world::dynamicState::river;
  using namespace world::dynamicState::skybox;
  using namespace logs;

  glEnable(GL_NORMALIZE);
  char errorMsgPt1 [] = "Error: (in init()) loadTexture() failed to load texture \"",
    errorMsgPt2 [] = "\"\n";

  // Should add error handling code at some point.
  // Load textures :)
  if(!(world::dynamicState::ground::textureHandle = loadTexture(world::landscape::ground::textureName)))
    {
      std::cerr<<errorMsgPt1<<world::landscape::ground::textureName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(world::dynamicState::road::textureHandle = loadTexture(world::landscape::road::textureName)))
    {
      std::cerr<<errorMsgPt1<<world::landscape::road::textureName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(logs::textureHandle = loadTexture(logs::textureName)))
    {
      std::cerr<<errorMsgPt1<<logs::textureName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(world::dynamicState::riverbed::textureHandle = loadTexture(world::landscape::riverbed::textureName)))
    {
      std::cerr<<errorMsgPt1<<world::landscape::riverbed::textureName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandleNegX = loadTexture(negXName)))
    {
      std::cerr<<errorMsgPt1<<negXName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandleNegY = loadTexture(negYName)))
    {
      std::cerr<<errorMsgPt1<<negYName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandleNegZ = loadTexture(negZName)))
    {
      std::cerr<<errorMsgPt1<<negZName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandlePosX = loadTexture(posXName)))
    {
      std::cerr<<errorMsgPt1<<posXName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandlePosY = loadTexture(posYName)))
    {
      std::cerr<<errorMsgPt1<<posYName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
  if(!(textureHandlePosZ = loadTexture(posZName)))
    {
      std::cerr<<errorMsgPt1<<posZName<<errorMsgPt2;
      exit(TEXTURE_ERROR);
    }
}


void display()
{
  using world::dynamicState::camera;

  camera.perspectiveOn();
  camera.applyCameraTransformations();

  drawSkyBox();
  
  draw3D();

  camera.orthoOn();
  if(hud::on)
    drawHud();
      
  ++hud::frames;
  
  for(GLenum e {glGetError()}; e != GL_NO_ERROR; e = glGetError()) // Handle errors
    std::cerr<<"display: "<<gluErrorString(e)<<'\n';
  glutSwapBuffers();
}


void drawSkyBox()
{
  using namespace world::dynamicState::skybox;
  using namespace vectorAxis;
  using world::dynamicState::camera;
  
  camera.disableDepthTestAndLighting();

  glColor3f(1, 1, 1);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, textureHandleNegZ);  
  glBegin(GL_QUADS);		// First face (front face.)
  glTexCoord2f(1, 0);
  glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex]);
  glTexCoord2f(0, 0);
  glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex]);
  glTexCoord2f(0, 1);
  glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex]);
  glTexCoord2f(1, 1);
  glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex]);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureHandlePosY);
  glBegin(GL_QUADS);		// Second face (top face.)
  glTexCoord2f(1, 1);  
  glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex]);
  glTexCoord2f(0, 1);
  glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex]);
  glTexCoord2f(0, 0);
  glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex]);
  glTexCoord2f(1, 0);

  glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex]);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureHandlePosZ);
  glBegin(GL_QUADS);		// Third face (back face.)
  glTexCoord2f(0, 1);
  glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex]);
  glTexCoord2f(1, 1);
  glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex]);
  glTexCoord2f(1, 0);
  glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex]);
  glTexCoord2f(0, 0);
  glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex]);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureHandleNegY);
  glBegin(GL_QUADS);		// Fouth face (bottom face.)
  glTexCoord2f(0, 1);
  glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex]);
  glTexCoord2f(1, 1);
  glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex]);
  glTexCoord2f(1, 0);
  glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex]);
  glTexCoord2f(0, 0);
  glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex]);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, textureHandlePosX);
  glBegin(GL_QUADS);
  glTexCoord2f(1, 1);
  glVertex3f(tBL[xIndex], tBL[yIndex], tBL[zIndex]);
  glTexCoord2f(0, 1);
  glVertex3f(tFL[xIndex], tFL[yIndex], tFL[zIndex]); // Fifth face (left face.)
  glTexCoord2f(0, 0);
  glVertex3f(bFL[xIndex], bFL[yIndex], bFL[zIndex]);
  glTexCoord2f(1, 0);
  glVertex3f(bBL[xIndex], bBL[yIndex], bBL[zIndex]);
  glEnd();

    glBindTexture(GL_TEXTURE_2D, textureHandleNegX);
  glBegin(GL_QUADS);
  glTexCoord2f(1, 1);
  glVertex3f(tFR[xIndex], tFR[yIndex], tFR[zIndex]); // Sixth face (right face.)
  glTexCoord2f(1, 0);
  glVertex3f(bFR[xIndex], bFR[yIndex], bFR[zIndex]);
  glTexCoord2f(0, 0);
  glVertex3f(bBR[xIndex], bBR[yIndex], bBR[zIndex]);
    glTexCoord2f(0, 1);
  glVertex3f(tBR[xIndex], tBR[yIndex], tBR[zIndex]);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  camera.enableDepthTestAndLighting();
}


void draw3D()
{
  using world::dynamicState::frog::superFrog;
  using namespace vectorAxis;

  // Translate camera to frog position.
  world::dynamicState::camera.applyWorldTranslations(-superFrog.getPos()[xIndex], -superFrog.getPos()[yIndex], -superFrog.getPos()[zIndex]);
  drawFrog();
  if(world::dynamicState::drawAxis)
    drawCartesianAxis();
  drawGround();
  drawRoad();
  drawCars();
  drawRiverBed();
  drawLogs();
  drawRiver();
}


void drawFrog()
{
  using namespace world::dynamicState;
  using namespace world::axis;
  using namespace frog;


  glPushMatrix();
  glTranslatef(superFrog.getPos()[xIndex], superFrog.getPos()[yIndex] - frogDrawYOffset, superFrog.getPos()[zIndex]);
  //    drawSphere(0, 0, 0, 1, superFrog.getHitRadius(), 16, 16, false);
  superFrog.draw();
  glPopMatrix();
  
  
  if(!superFrog.isDead())
    {
      using namespace world::dynamicState::river::logs;
      using namespace world::dynamicState::frog;
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, world::lights::materialSpecular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, world::lights::materialShininess);
      // Set potential ball hit color.
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientTrajectoryBallHit); // Set color
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseTrajectoryBallHit);
      // X, y & z coords are stored in trajectory consecutively
      constexpr float trajectoryEndXOffset {3}, trajectoryEndYOffset {2}, trajectoryEndZOffset {1};
      if(world::dynamicState::river::logs::trajectoryLineTouchingLog || world::dynamicState::river::trajectoryLineTouchingWater)
	{
	  if(!potentialLogHit)
	    {				// Set ball miss color.
	      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientTrajectoryBallMiss);
	      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseTrajectoryBallMiss);
	    }
	  if(trajectory.size() != 0)
	    drawDotPotAtEndOfTrajectoryRainbow(trajectory[trajectory.size() - trajectoryEndXOffset],
					       trajectory[trajectory.size() - trajectoryEndYOffset],
					       trajectory[trajectory.size() - trajectoryEndZOffset],
					       world::landscape::zDepthOffset,
					       world::dynamicState::river::logs::hitLogIndicatorRadius,
					       world::dynamicState::river::lastWaveTesselation.get(),
					       world::dynamicState::wireframe);
	}
      else
	{
	  if(trajectory.size() != 0) // Trajectory may be empty if the game has just started.
	    drawDotPotAtEndOfTrajectoryRainbow(trajectory[trajectory.size() - trajectoryEndXOffset],
					       trajectory[trajectory.size() - trajectoryEndYOffset],
					       trajectory[trajectory.size() - trajectoryEndZOffset],
					       world::landscape::zDepthOffset, // 0.0 -> don't want to translate in the z axis!
					       world::dynamicState::river::logs::hitLogIndicatorRadius,
					       world::dynamicState::river::lastWaveTesselation.get(),
					       world::dynamicState::wireframe);
	}
      // Set trajectory color
      camera.disableLighting();
      glColor3f(trajectoryLineAndTrajectoryVectorColor[rIndex], trajectoryLineAndTrajectoryVectorColor[gIndex],
		trajectoryLineAndTrajectoryVectorColor[bIndex]);
      // Draw trajectory and trajectory vector line.
      drawTrajectory(trajectory);
      drawVector(superFrog.getPos()[xIndex], superFrog.getPos()[yIndex], superFrog.getPos()[zIndex],
		 superFrog.getVec().getXComp(), superFrog.getVec().getYComp(), superFrog.getVec().getZComp(), vecScale,
		 normalize);
      camera.enableLighting();
    }
}


void drawCartesianAxis()
{
  using namespace world::axis;
  using world::dynamicState::camera;
  camera.disableLighting();
  glBegin(GL_LINES);
  glColor3f(xColor[xIndex], xColor[yIndex], xColor[zIndex]);
  glVertex3f(xXOrigin, xYOrigin, xZOrigin);
  glVertex3f(xX, xY, xZ);
  glColor3f(yColor[xIndex], yColor[yIndex], yColor[zIndex]);
  glVertex3f(yXOrigin, yYOrigin, yZOrigin);
  glVertex3f(yX, yY, yZ);
  glColor3f(zColor[xIndex], zColor[yIndex], zColor[zIndex]);
  glVertex3f(zXOrigin, zYOrigin, zZOrigin);
  glVertex3f(zX, zY, zZ);
  glEnd();
  camera.enableLighting();
}


void drawGround()
{
  using namespace world::landscape::ground;
  using namespace normals;
  using world::dynamicState::tesselation;
  using world::dynamicState::ground::textureHandle;
  using world::dynamicState::drawTexture;
  // Set material color.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, world::lights::materialShininess);
  // Draw first section.
  drawGrid(sec1StX, sec1StY, sec1StZ, sec1LnX, sec1LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, textureScale);
  // Draw second section.
  drawGrid(sec2StX, sec2StY, sec2StZ, sec2LnX, sec2LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, textureScale);
  // Draw third section.
  drawGrid(sec3StX, sec3StY, sec3StZ, sec3LnX, sec3LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, textureScale);
}


void drawRoad()
{				// Draw first section.
  using namespace world::landscape;
  using namespace road;
  using namespace normals;
  using world::dynamicState::tesselation;
  using world::dynamicState::road::textureHandle;
  using world::dynamicState::drawTexture;
  // Set material color.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  // Draw first section.
  glPushMatrix();
  glRotatef(-angle1, noXRot, noYRot, zRot);
  // The side's of our road are grassy :)
  drawGrid(sec1StY, -sec1StX, sec1StZ, sec1LnY, sec1LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, world::dynamicState::ground::textureHandle,
	   bankTextureScale);
  glPopMatrix();
  // Draw second section.
  drawGrid(sec2StX, sec2StY, sec2StZ, width, sec2LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale);//, drawTexture, textureHandle, textureScale);
  drawTopRoadTex();
  // Draw third section.
  glPushMatrix();
  glRotatef(angle2, noXRot, yRot, noZRot);
  glRotatef(-angle1, noXRot, noYRot, zRot);
  drawGrid(sec3StY, sec3StX, sec3StZ, sec3LnY, sec3LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, world::dynamicState::ground::textureHandle,
	   bankTextureScale);
  glPopMatrix();
}


void drawTopRoadTex()
{
  using namespace world::landscape;
  using namespace road;
  using world::dynamicState::road::textureHandle;
  glEnable(GL_BLEND);
  //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                                                   
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);
  glVertex3f(sec2StX, 		sec2StY + textureYOffset, sec2StZ);
  glTexCoord2f(1, 1);
  glVertex3f(sec2StX + width,	sec2StY + textureYOffset, sec2StZ);
  glTexCoord2f(1, 0);
  glVertex3f(sec2StX + width,	sec2StY + textureYOffset, sec2StZ + sec2LnZ);
  glTexCoord2f(0, 0);
  glVertex3f(sec2StX,		sec2StY + textureYOffset, sec2StZ + sec2LnZ);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}


void drawCars()
{
  using namespace world::landscape::cars;
  using namespace world::dynamicState::cars;
  using world::dynamicState::tesselation;
  //  using world::dynamicState::cars::cars;
  
  glPushMatrix();
  // Top left corner of road
  glTranslatef(world::landscape::road::sec1EnX,
	       world::landscape::road::sec1EnY, world::landscape::zDepthOffset);
  for(car c: cars)
    {
      glPushMatrix();			// Move to lane and position along serial experiments.
      glTranslatef(c.xOffset, c.yOffset, c.zOffset);
      // Set body color.
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(c.materialAmbientBody[0]));
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(c.materialDiffuseBody[0]));
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c.materialSpecular[0]));
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininessBody);
      //      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c.materialSpecular[0])); // Both have the same specular.
      drawBox(length, height, width, normals::drawNormals, normals::normScale);
      // Set dome color.
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(materialAmbientDome[0]));
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(materialDiffuseDome[0]));
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, world::lights::materialSpecular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininessDome);
      drawSphere(domeXStart, domeYStart, domeZStart, domeZScale, domeRadius, tesselation.get(),
		 tesselation.get(), normals::drawNormals, normals::normScale);
      
      glPopMatrix();
    }
  glPopMatrix();
}


void drawRiverBed()
{
  using namespace world::lights;
  using namespace world::landscape;
  using namespace riverbed;
  using namespace normals;
  using world::dynamicState::tesselation;
  using world::dynamicState::riverbed::textureHandle;
  using world::dynamicState::drawTexture;

  // Set material color.
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
  
  // Draw first section.
  glPushMatrix();
  glRotatef(angle1, noXRot, noYRot, zRot);
  //  glColor3f(1j, 0, 0);
  drawGrid(sec1StY, -sec1StX, sec1StZ, sec1LnY, sec1LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, bankTextureScale);
  glPopMatrix();
  // Draw second section.
  drawGrid(sec2StX, sec2StY, sec2StZ, sec2LnX, sec2LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, textureScale);
  // Draw third section.
  glPushMatrix();
  glRotatef(-angle1, noXRot, noYRot, zRot);
  drawGrid(sec3StY, -sec3StX, sec3StZ, sec3LnY, sec3LnZ, tesselation.get(), tesselation.get(),
	   drawNormals, normScale, drawTexture, textureHandle, bankTextureScale);
  glPopMatrix();
}


void drawRiver()
{
  using namespace world::dynamicState;
  using namespace world::axis;
  using namespace river;
  lastWaveTesselation = tesselation;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, world::lights::materialSpecular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

  for(int zIter {}, zIterNext {1}; // Draw wave
      zIterNext < (lastWaveTesselation.get() + additionalVerticesZ);
      ++zIter, zIterNext = (zIter +1))
    {
      constexpr int xStart {-1}, xCenter {0}, xEnd {1};
      int xIter {1};
      xIter = 1;
      registerVectorNormal(waveVertices[zIter][xIter + xStart][xIndex],
			   waveVertices[zIter][xIter + xStart][yIndex],
			   waveVertices[zIter][xIter + xCenter][xIndex],
			   waveVertices[zIter][xIter + xCenter][yIndex],
			   waveVertices[zIter][xIter + xEnd][xIndex],
			   waveVertices[zIter][xIter + xEnd][yIndex],
			   waveVertices[zIter][xIter + xCenter][zIndex]);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(waveVertices[zIter][xIter + xCenter][xIndex],
                 waveVertices[zIter][xIter + xCenter][yIndex],
                 waveVertices[zIter][xIter + xCenter][zIndex]);
      registerVectorNormal(waveVertices[zIterNext][xIter + xStart][xIndex],
			   waveVertices[zIterNext][xIter + xStart][yIndex],
			   waveVertices[zIterNext][xIter + xCenter][xIndex],
			   waveVertices[zIterNext][xIter + xCenter][yIndex],
			   waveVertices[zIterNext][xIter + xEnd][xIndex],
			   waveVertices[zIterNext][xIter + xEnd][yIndex],
			   waveVertices[zIterNext][xIter + xCenter][zIndex]);
      glVertex3f(waveVertices[zIterNext][xIter + xCenter][xIndex],
                 waveVertices[zIterNext][xIter + xCenter][yIndex],
                 waveVertices[zIterNext][xIter + xCenter][zIndex]);
      ++xIter;
      for( ; xIter < (lastWaveTesselation.get() + additionalVerticesX -1); ++xIter)
        {                       // -1 because we are looking 1 ahead
          registerVectorNormal(waveVertices[zIter][xIter + xStart][xIndex],
			       waveVertices[zIter][xIter + xStart][yIndex],
			       waveVertices[zIter][xIter + xCenter][xIndex],
			       waveVertices[zIter][xIter + xCenter][yIndex],
			       waveVertices[zIter][xIter + xEnd][xIndex],
			       waveVertices[zIter][xIter + xEnd][yIndex],
			       waveVertices[zIter][xIter + xCenter][zIndex]);
          glVertex3f(waveVertices[zIter][xIter + xCenter][xIndex],
                     waveVertices[zIter][xIter + xCenter][yIndex],
                     waveVertices[zIter][xIter + xCenter][zIndex]);
          registerVectorNormal(waveVertices[zIterNext][xIter + xStart][xIndex],
			       waveVertices[zIterNext][xIter + xStart][yIndex],
			       waveVertices[zIterNext][xIter + xCenter][xIndex],
			       waveVertices[zIterNext][xIter + xCenter][yIndex],
			       waveVertices[zIterNext][xIter + xEnd][xIndex],
			       waveVertices[zIterNext][xIter + xEnd][yIndex],
			       waveVertices[zIterNext][xIter + xCenter][zIndex]);
          glVertex3f(waveVertices[zIterNext][xIter + xCenter][xIndex],
                     waveVertices[zIterNext][xIter + xCenter][yIndex],
                     waveVertices[zIterNext][xIter + xCenter][zIndex]);
        }
      glEnd();
    }
  drawWaveNormals();
}


void registerVectorNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                          const float yEnd, const float z)
{
  float run {}, y {};
  calcNormal(xStart, yStart, xCenter, yCenter, xEnd, yEnd, z, run, y, normals::normScale);
  glNormal3f(xCenter + run, yCenter + y, z);
}


void drawWaveNormals()
{
  using namespace world::dynamicState;
  using namespace world::axis;
  using namespace river;
  using normals::drawNormals;
  if(drawNormals)
    for(int zIter {}, zIterNext {1}; // Draw Normals :)
	zIterNext < (lastWaveTesselation.get() + additionalVerticesZ);
	++zIter, zIterNext = (zIter +1))
      {
	constexpr int xStart {-1}, xCenter {0}, xEnd {1};
	int xIter {1};
	xIter = 1;
	drawVectorNormal(waveVertices[zIter][xIter + xStart][xIndex],     waveVertices[zIter][xIter + xStart][yIndex],
			 waveVertices[zIter][xIter + xCenter][xIndex],    waveVertices[zIter][xIter + xCenter][yIndex],
			 waveVertices[zIter][xIter + xEnd][xIndex],       waveVertices[zIter][xIter + xEnd][yIndex],
			 waveVertices[zIter][xIter + xCenter][zIndex]);
	drawVectorNormal(waveVertices[zIterNext][xIter + xStart][xIndex],
			 waveVertices[zIterNext][xIter + xStart][yIndex],
			 waveVertices[zIterNext][xIter + xCenter][xIndex],
			 waveVertices[zIterNext][xIter + xCenter][yIndex],
			 waveVertices[zIterNext][xIter + xEnd][xIndex],
			 waveVertices[zIterNext][xIter + xEnd][yIndex],
			 waveVertices[zIterNext][xIter + xCenter][zIndex]);
	++xIter;
	for( ; xIter < (lastWaveTesselation.get() + additionalVerticesX -1); ++xIter)
	  {                       // -1 because we are looking 1 ahead
	    drawVectorNormal(waveVertices[zIter][xIter + xStart][xIndex],
			     waveVertices[zIter][xIter + xStart][yIndex],
			     waveVertices[zIter][xIter + xCenter][xIndex],
			     waveVertices[zIter][xIter + xCenter][yIndex],
			     waveVertices[zIter][xIter + xEnd][xIndex],
			     waveVertices[zIter][xIter + xEnd][yIndex],
			     waveVertices[zIter][xIter + xCenter][zIndex]);
	    drawVectorNormal(waveVertices[zIterNext][xIter + xStart][xIndex],
			     waveVertices[zIterNext][xIter + xStart][yIndex],
			     waveVertices[zIterNext][xIter + xCenter][xIndex],
			     waveVertices[zIterNext][xIter + xCenter][yIndex],
			     waveVertices[zIterNext][xIter + xEnd][xIndex],
			     waveVertices[zIterNext][xIter + xEnd][yIndex],
			     waveVertices[zIterNext][xIter + xCenter][zIndex]);
	  }
      }
}


void drawVectorNormal(const float xStart, const float yStart, float xCenter, float yCenter, const float xEnd,
                      const float yEnd, const float z)
{
  using normals::normDrawScale;
  float run {}, y {};
  calcNormal(xStart, yStart, xCenter, yCenter, xEnd, yEnd, z, run, y, normals::normScale);
  glBegin(GL_LINES);
  glVertex3f(xCenter, yCenter, z);
  glVertex3f(xCenter + (run * normDrawScale), yCenter + (y * normDrawScale), z);
  glEnd();
}


void drawLogs()
{
  using namespace world::dynamicState::river::logs;
  using namespace world::axis;
  using world::dynamicState::tesselation;

  glPushMatrix();			// Move to lane and position along serial experiments.
  glTranslatef(world::landscape::riverbed::sec1EnX,
	       world::landscape::riverbed::sec1StY, world::landscape::zDepthOffset);
  
  for(mobileLogs l: logs)
    {
      glPushMatrix();
      glTranslatef(l.xOffset, l.yOffset, l.zOffset);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, world::lights::materialShininess);
      drawCylinder(0.0, 0.0, 0.0, radius, logLen, world::dynamicState::river::lastWaveTesselation.get(),
		   world::dynamicState::river::lastWaveTesselation.get(), normals::drawNormals, normals::normScale,
		   world::dynamicState::drawTexture, textureHandle, textureScale);
      // Ends must be a slightly different color as they are not textured
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientEnd);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuseEnd);
      drawCircle(world::landscape::zDepthOffset + logLen, radius, world::dynamicState::river::lastWaveTesselation.get(),
		 world::dynamicState::wireframe, normals::drawNormals, normals::normScale);
      glRotatef(angleEndCircle, noXRot, yRot, noZRot);
      drawCircle(world::landscape::zDepthOffset, radius, world::dynamicState::river::lastWaveTesselation.get(),
		 world::dynamicState::wireframe, normals::drawNormals, normals::normScale);
      glPopMatrix();
    }
  glPopMatrix();
}


void drawHud()
{ // Had trouble compiling with string listerals that are "char * const" in utility.hpp. Maybe I'm just stupid :'(
  using namespace hud;
  using namespace world::dynamicState::gameScore;
  using world::dynamicState::camera;

  std::stringstream fr {};
  std::stringstream ft {};
  std::stringstream t {};
  std::stringstream score {};
  std::stringstream lives {};

  // frame rate
  fr<<"fr (f/s): ";
  fr.precision(precision);
  fr.width(width);
  fr<<hud::fr;
  // frame time
  ft<<"ft (ms/f): ";
  ft.precision(precision);
  ft.width(width);
  ft<<hud::ft;
  // tesselation
  t<<"tess:      ";  
  t.precision(precision);
  t.width(width);
  t<<(float)world::dynamicState::tesselation.get();
  score<<"score:     ";
  score<<world::dynamicState::gameScore::score;
  lives<<"lives:     ";
  lives<<world::dynamicState::gameScore::lives;

  glColor3f(r, g, b);  
  drawString(((camera.width - x) / camera.width), (camera.height - hud::y1) / camera.height,
    z, fr.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24); // y1 is ambiguous, I am not sure why :'(
  drawString(((camera.width - x) / camera.width), (camera.height - y2) / camera.height,
     z, ft.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
  drawString(((camera.width - x) / camera.width), (camera.height - y3) / camera.height,
     z, t.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
  
  if(world::dynamicState::gameScore::score == maxScore)
    {
      drawString(((camera.width - x) / camera.width), (camera.height - y4) / camera.height,
		 z, winningString, GLUT_BITMAP_TIMES_ROMAN_24);
    }
  else
    {
      if(world::dynamicState::gameScore::lives != minLives)
	{
	  drawString(((camera.width - x) / camera.width), (camera.height - y4) / camera.height,
		     z, lives.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
	  drawString(((camera.width - x) / camera.width), (camera.height - y5) / camera.height,
		     z, score.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
	}
      else
	{
	  drawString(((camera.width - x) / camera.width), (camera.height - y4) / camera.height,
		     z, losingString, GLUT_BITMAP_TIMES_ROMAN_24);
	}
    }
  
  if(world::dynamicState::restrictedMode)
    {				// Restriced mode!
      std::stringstream restricted {};
      restricted<<"IN RESTRICTED MODE!";
      fr.width(width);
      drawString(x, y4, z, restricted.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24);
    }
}


void mouseMotion(const int x, const int y)
{
  using namespace world::dynamicState;
  camera.applyCameraTransformations();
  camera.cameraUpdate(x, y);
}


void mouseButt(const int button, const int state, const int x, const int y)
{
  world::dynamicState::camera.buttonEventUpdate(button, state, x, y);
}


void keyboardDown(const unsigned char key, const int x, const int y)
{
  using namespace world::dynamicState;
  using normals::drawNormals;

  switch(key)
    {
    case 'g':			// Stop updating all state.
      pause = !pause;
      break;
    case ESC_CHAR:		// Quit
    case 'q':			// Q is crossed out in the speck but we DECIDED to leave it in.
      exit(EXIT_SUCCESS);
    }

  if(world::dynamicState::pause) // We don't wan't to update any state while the animation is paused.
    switch(key)
      {
      case 'x':			// Toggle annying axis lines >:(
	drawAxis = !drawAxis;
	break;
      case 'n':
	drawNormals = !drawNormals;
	break;
      case 'e':			// Enter restricted mode (The real game :) )
	resetVectorIfOutOfBounds(); // Cant enter restricted mode with illegal state.
	restrictedMode = !restrictedMode;
	  break;
      case 'a':			// Rotate left.
	keyMask |= leftMask;
	break;
      case 'd':			// Rotate right.
	keyMask |= rightMask;
	break;
      case 'f':			// Toggle solid / wireframe.
	wireframe = !wireframe;
	if(wireframe)
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case 't':
	world::dynamicState::drawTexture = !world::dynamicState::drawTexture;
	break;
      case 'o':			// Turn hud on / off.
	hud::on = !hud::on;
	break;
      case 'r':
	{				// Reset player.
	  if(!frog::superFrog.isJumping())
	    {			// Resetting while jumping is cheating!
	      using namespace frog;
	      superFrog.setDead(false);
	      superFrog.setWon(false);
	      superFrog.setJumping(false);
	      superFrog.setOnLog(false);
	      superFrog.resetPos();
	      superFrog.setVec(SphericalCoords(initVecR, initVecAzimuth, initVecInclination));
	      camera.resetAngleAndZoom();
	    }
	  break;
	}
      case '+':		  // Change tesselation.
      case '=':			// Use "+" character when not pressing shift.
	tesselation.increase();
	break;
      case '-':
	tesselation.decrease();
	break;
      case ' ':			// Start frog jumping.
	if(world::dynamicState::restrictedMode)
	  resetVectorIfOutOfBounds();
	if(!frog::superFrog.isDead())
	  {
	    frog::superFrog.setJumping(true);
	  }
	break;
      }
}


void keyboardUp(unsigned char key, int x, int y)
{
  using namespace world::dynamicState;
  switch(key)
    {
    case 'a':			// Left
      keyMask &= ~leftMask;
      break;
    case 'd':
      keyMask &= ~rightMask;
      break;
    }
}


void specialKeyDown(const int key, const int x, const int y)
{
  using namespace world::dynamicState;
  switch(key)
    {
    case GLUT_KEY_LEFT:                   // Increase rotation (anti clock wise.)
      keyMask |= backwardMask;
      break;
    case GLUT_KEY_RIGHT:                   // Decrease rotation.
      keyMask |= forwardMask;
      break;
    case GLUT_KEY_DOWN:                   // Decrease magnitude.
      keyMask |= decMask;
      break;
    case GLUT_KEY_UP:                   // Increase magnitude.
      keyMask |= incMask;
      break;
    }
}


void specialKeyUp(const int key, const int x, const int y)
{
  using namespace world::dynamicState;
  switch (key)
    {
    case GLUT_KEY_LEFT:                   // Increase rotation off
      keyMask &= ~backwardMask;
      break;
    case GLUT_KEY_RIGHT:                   // Decrease rotation off
      keyMask &= ~forwardMask;
      break;
    case GLUT_KEY_DOWN:                   // Increase magnitude off
      keyMask &= ~decMask;
      break;
    case GLUT_KEY_UP:                   // Decrease magnitude off
      keyMask &= ~incMask;
      break;
    }
}


void update()
{
  using namespace world::dynamicState::frog;
  using namespace world::axis;
  using namespace world::dynamicState::gameScore;
  using world::dynamicState::camera;
  // Since collision detection will rely on the time values internal to t, t should be updated here.
  world::dynamicState::t.update();
  if(world::dynamicState::pause)
    {
      // We calculate the river and log vertices here beacuse they are needed for collision detection as well a drawing
      updateRiverVertices();
      updateLogs();
      updateCars();
           updateFrogTrajectory();

      if((lives > minLives) && (score < maxScore))
	{
	  updateFrogVec();
	  if(!superFrog.isDead())
	    {// dead should not be set when jumping is set!
	      if(!staticFrogCollisions())
		{
		  if(superFrog.isJumping())
		    {
		      updateJumpingFrog();
		    }
		  else
		    {
		      if(superFrog.isOnLog())
			updateFrogOnLog();		      // :)
		    }
		}
	      handleBoarderCollision(superFrog.getPos()[xIndex], superFrog.getPos()[yIndex], superFrog.getPos()[zIndex],
				     radius, minX, maxX, minY, maxY,
				     minZ, maxZ);
	 
	      if(superFrog.isDead() && superFrog.hasWon())
		{
		  superFrog.setDead(false);
		  superFrog.setWon(false);
		  superFrog.setJumping(false);
		  superFrog.setOnLog(false);
		  superFrog.resetPos();
		  superFrog.setVec(SphericalCoords(initVecR, initVecAzimuth, initVecInclination));
		  camera.resetAngleAndZoom();
		  ++score;
		}
	      else
		{
		  if(superFrog.isDead())
		    {			// Move into memeber function?
		      superFrog.setDead(false);
		      superFrog.setWon(false);
		      superFrog.setJumping(false);
		      superFrog.setOnLog(false);
		      superFrog.resetPos();
		      superFrog.setVec(SphericalCoords(initVecR, initVecAzimuth, initVecInclination));
		      camera.resetAngleAndZoom();
		      --lives;
		    }
		}
	    }
	}
      else
	updateDeadFrogPos();

      updateHud();
      glutPostRedisplay();		// Ask glut to schedule a call to the display function :)
    }
}


void updateRiverVertices()
{
  using namespace world::dynamicState;
  using namespace river;
  /* We want to avoid race conditions where tesselation is changed before updateRiverVertices or drawRiver have
     run in full and also when we are part way through calculating collision detection. */
  lastWaveTesselation = tesselation;

  float z {stZ};

  /* "x <= (enX + (xLength / lastWaveTesselation.get()))" and not "x <= enX" because we need to draw one more
     vertice then there are segment. */
  for(int zIter {}; z <= enZ; z += (zLength / lastWaveTesselation.get()), ++zIter)
    {                           // Iterate over z plane of river.
      /* "xLength / lastWaveTesselation.get()" because we need an extra vertice on each end in x for normals
         (and yes I'm too stupid to figure out your equations >:'(      ) */
      float x {};
      x = stX - (xLength / lastWaveTesselation.get());

      for(int xIter {}; x <= (enX + (xLength / lastWaveTesselation.get())); // Again we need an extra vertice on each end in x.
          x += (xLength / lastWaveTesselation.get()), ++xIter)
        {                               // Iterate over x plane of river and store river verticies.
          waveVertices[zIter][xIter][world::axis::xIndex] = x;
          waveVertices[zIter][xIter][world::axis::yIndex] = yOffset; // Only "= yOffset" because we calculate y in addAdditionalWaves()!
          waveVertices[zIter][xIter][world::axis::zIndex] = z;
        }
    }
  // If we make changes here we must also make equivalent changes in updateLogVertices()!
  addAdditionalWaves(A1, k1, w1);
  addAdditionalWaves(A2, k2, w2);
  addAdditionalWaves(A3, k3, w3);
}


void addAdditionalWaves(const float a, const float k, const float w)
{
  using namespace world::dynamicState;
  using namespace river;

  lastWaveTesselation = tesselation;
  float z {stZ};

  for(int zIter {}; z <= enZ; z += (zLength / lastWaveTesselation.get()), ++zIter)
    {
      float x {};
      x = stX - (xLength / lastWaveTesselation.get());

      for(int xIter {}; x <= (enX + (xLength / lastWaveTesselation.get()));
          x += (xLength / lastWaveTesselation.get()), ++xIter)
	waveVertices[zIter][xIter][world::axis::yIndex] += waveY(a, k, w, x);
	//        waveVertices[zIter][xIter][world::axis::yIndex] += a * sin(k*x + w*t.getT()) + yOffset;
    }
}


float waveY(const float A, const float k, const float w, const float x)
{
  return  (A * sin(k*x + w*world::dynamicState::t.getT())); 
}


void updateLogs()
{
  using namespace world::dynamicState::river::logs;
  using world::dynamicState::frog::logOn;

  static int potentialLane {};		// New logs are added to this lane.
  static Time t {};
  t.updateT();
  
  if(t.getDt() > updateRate)
    {
      for(size_t iter {}; iter < logs.size(); ++iter)
	{				// Advance logs.
	  logs[iter].zOffset += (logs[iter].laneDirection ? updateDistance: -updateDistance);
	  if((logs[iter].zOffset + logLen) < world::landscape::riverbed::sec1StZ || logs[iter].zOffset > world::landscape::riverbed::sec1EnZ)
	    {
	      logs.erase(logs.begin() +iter);
	      --iter;
	    }
	}
      // Adjust randNum::uni to change rate at which logs randomly spawn.
      if(randNum::uni(randNum::randomNumberEngine) == 0)
	{
	  if(logs.size() > 0)
	    {				// We need to make sure we don't collide with other logs.
	      bool collision {false};
	      for(size_t iter {}; iter < logs.size(); ++iter)
		{
		  if(logs[iter].lane == potentialLane)
		    {			// There is a log in the lane that we are attempting to add to
		      if(laneDirections[potentialLane] == true)
			{
			  if(logs[iter].zOffset < (world::landscape::riverbed::sec1StZ + logLen - (logLen * collisionDist)))
			    {		// There is a collision
			      collision = true;
			      break;
			    }	  
			}
		      else
			{
			  if(logs[iter].zOffset > (world::landscape::riverbed::sec1EnZ - logLen - (logLen * collisionDist)))
			    {		// There is a collision
			      collision = true;
			      break;
			    }
			}
		    }
		}
	      if(!collision)
		{			// There was no collision
		  if(laneDirections[potentialLane] == true)
		    {
		      logs.push_back(mobileLogs {laneOffsets[potentialLane], 0,
			    world::landscape::riverbed::sec1StZ - logLen, potentialLane,
			    laneDirections[potentialLane], false});
		    }
		  else
		    {
		      logs.push_back(mobileLogs {laneOffsets[potentialLane], 0,
			    world::landscape::riverbed::sec1EnZ, potentialLane, laneDirections[potentialLane], false});
		    }
		}
	    }
	  else
	    {				// There are no other logs so add log.
	      if(laneDirections[potentialLane] == true)
		{
		  logs.push_back(mobileLogs {laneOffsets[potentialLane], 0,
			world::landscape::riverbed::sec1StZ - logLen, potentialLane, laneDirections[potentialLane], false});
		}
	      else
		{
		  logs.push_back(mobileLogs {laneOffsets[potentialLane], 0,
			world::landscape::riverbed::sec1EnZ, potentialLane, laneDirections[potentialLane], false});
		}
	    }
	}
      using namespace world::dynamicState::river;
      for(size_t iter {}; iter < logs::logs.size(); ++iter)
	{				// update logs y values.
	  logs::logs[iter].yOffset = (waveY(A1, k1, w1, logs::logs[iter].xOffset)
				      + waveY(A2, k2, w2, logs::logs[iter].xOffset) +
				      waveY(A3, k3, w3, logs::logs[iter].xOffset) + yOffset) + logs::floatHeight;
	}
      
      // Cycle through lanes
      ++potentialLane;
      if(potentialLane == numLanes)
	potentialLane = 0;
      
      t.update();
    }
}


/* Randomely adds new cars to world::dynamicState::cars::cars and update's cars
   already in the vector (removing those that have gone off of the road) */
void updateCars()
{
  using namespace world::landscape::cars;
  using namespace world::dynamicState::cars;
  using namespace::randNum;
  using namespace::world::lights;
  using world::dynamicState::cars::cars;

  static int potentialLane {};		// New cars are added to this lane.

					// Should be moved into seperet func, updateCarColors()?
  using namespace randNum;
  using namespace world::landscape::cars;
  using namespace world::lights;
  
  float downR {1}, downG {1}, downB {1};
  int lucky {};
  switch((uni(randomNumberEngine)) % 3)
    {					// Add color bias.
    case 0:
      lucky = 0;
      downR = toneUpColor;
      downG = toneDownColor;
      downB = toneDownColor;
      break;
    case 1:
      lucky = 1;
      downR = toneDownColor;
      downG = toneUpColor;
      downB = toneDownColor;
      break;
    case 2:
      lucky = 2;
      downR = toneDownColor;
      downG = toneDownColor;
      downB = toneUpColor;
      break;
    } // Use std::move for vectors and put in new function?
  std::vector<float> ambientBody {(float(uni(randomNumberEngine)) / randNum::maxRange) * downR,
      (float(uni((randomNumberEngine))) / randNum::maxRange) * downG,
      (float(uni((randomNumberEngine))) / randNum::maxRange) * downB,
      (alpha)};
  switch(lucky)
    {					// Add aditional color bias.
    case 0:
      ambientBody[m1Index] += colorAddition;
      break;
    case 1:
      ambientBody[m2Index] += colorAddition;
      break;
    case 2:
      ambientBody[m3Index] += colorAddition;
      break;
    }
  std::vector<float> diffuseBody {(float(uni(randomNumberEngine)) / randNum::maxRange) * downR,
      (float(uni((randomNumberEngine))) / randNum::maxRange) * downG,
      (float(uni((randomNumberEngine))) / randNum::maxRange) * downB,
      (alpha)};
  switch((uni(randomNumberEngine)) % 3)
    {					// Different bias for specular.
    case 0:
      downR = toneUpColor;
      downG = toneDownColor;
      downB = toneDownColor;
      break;
    case 1:
      downR = toneDownColor;
      downG = toneUpColor;
      downB = toneDownColor;
      break;
    case 2:
      downR = toneDownColor;
      downG = toneDownColor;
      downB = toneUpColor;
      break;
    }
  std::vector<float> specular {(float(uni(randomNumberEngine)) / randNum::maxRange) * downR,
      (float(uni(randomNumberEngine)) / randNum::maxRange) * downG,
      (float(uni(randomNumberEngine)) / randNum::maxRange) * downB,
      (specAlpha)};
  specular[m1Index] *= specMull;	// Reduce specular.
  specular[m2Index] *= specMull;
  specular[m3Index] *= specMull;
  std::vector<std::vector<float>> ret {ambientBody, diffuseBody, specular};

  static Time t {};
  t.updateT();
  if(t.getDt() > updateRate)
    {
      for(size_t iter {}; iter < cars.size(); ++iter)
	{				// Advance cars.
	  cars[iter].zOffset += (cars[iter].laneDirection ? updateDistance: -updateDistance);
	  if((cars[iter].zOffset + width) < world::landscape::road::sec1StZ || cars[iter].zOffset > world::landscape::road::sec1EnZ)
	    {
	      cars.erase(cars.begin() +iter);
	      --iter;
	    }
	}
      // Adjust randNum::uni to change rate at which cars randomly spawn.
      if(randNum::uni(randNum::randomNumberEngine) == 0)
	{
	  if(cars.size() > 0)
	    {				// We need to make sure we don't collide with other cars.
	      bool collision {false};
	      for(size_t iter {}; iter < cars.size(); ++iter)
		{
		  if(cars[iter].lane == potentialLane)
		    {			// There is a car in the lane that we are attempting to add to
		      if(laneDirections[potentialLane] == true)
			{
			  if(cars[iter].zOffset < (world::landscape::road::sec1StZ + width - (width * collisionDist)))
			    {		// There is a collision
			      collision = true;
			      break;
			    }	  
			}
		      else
			{
			  if(cars[iter].zOffset > (world::landscape::road::sec1EnZ - width - (width * collisionDist)))
			    {		// There is a collision
			      collision = true;
			      break;
			    }
			}
		    }
		}
	      if(!collision)
		{			// There was no collision
		  if(laneDirections[potentialLane] == true)
		    {
		      cars.push_back(car {laneOffsets[potentialLane], driveHeight,
			    world::landscape::road::sec1StZ - width, potentialLane, laneDirections[potentialLane],
			    ambientBody, diffuseBody, specular});
		    }
		  else
		    {
		      cars.push_back(car {laneOffsets[potentialLane], driveHeight,
			    world::landscape::road::sec1EnZ, potentialLane, laneDirections[potentialLane],
			    ambientBody, diffuseBody, specular});
		    }
		}
	    }
	  else
	    {				// There are no other cars so add car.
	      if(laneDirections[potentialLane] == true)
		{
		  cars.push_back(car {laneOffsets[potentialLane], driveHeight,
			world::landscape::road::sec1StZ - width, potentialLane, laneDirections[potentialLane],
			ambientBody, diffuseBody, specular});
		}
	      else
		{
		  cars.push_back(car {laneOffsets[potentialLane], driveHeight,
			world::landscape::road::sec1EnZ, potentialLane, laneDirections[potentialLane],
			ambientBody, diffuseBody, specular});
		}
	    }
	}
      // Cycle through lanes
      ++potentialLane;
      if(potentialLane == numLanes)
	potentialLane = 0;
      
      t.update();
    }
}


void  updateFrogVec()
{
  using namespace world::dynamicState::frog;
  using namespace world::dynamicState;
  
  static bool keyPressed {false};
  static float tLast {t.getT()}, dt {};

  // If there is some key pressed and we are not jumping (updating the frog vector while jumping would be cheating!)
  if(keyMask && !superFrog.isJumping() && !superFrog.isDead())
    {
      if(!keyPressed)
        {                       // We don't know how much time has passed
          keyPressed = true;
          tLast = t.getT();
        }

      dt = t.getT() - tLast;

      if(keyMask & backwardMask)
        {			// Move backwards (in x.)
          superFrog.setVecAzimuthRel(rotateScale * dt);
        }
      if(keyMask & forwardMask)
        {			// Move forwards.
          superFrog.setVecAzimuthRel(-rotateScale * dt);
        }
      if(keyMask & incMask)
        {			// Increase vec size.
	  if(restrictedMode)
	    {
	      if(world::dynamicState::restrictedMode)
		resetVectorIfOutOfBounds();
	      if((superFrog.getVec().getMag() + magScale * dt) <= superFrog.restrictedModeMaxVectorMagnitude)
		superFrog.setVecMagRel(magScale * dt);
	    }
	  else
	    {
	      superFrog.setVecMagRel(magScale * dt);
	    }
        }
      if(keyMask & decMask)
        {			// Decrease vec size.
	  if(restrictedMode)
	    {
	      if(world::dynamicState::restrictedMode)
		resetVectorIfOutOfBounds();
	      if((superFrog.getVec().getMag() - magScale * dt) <= superFrog.restrictedModeMaxVectorMagnitude)
		superFrog.setVecMagRel(-magScale * dt);
	    }
	  else
	    {
	      superFrog.setVecMagRel(-magScale * dt);
	    }
        }
      if(keyMask & leftMask)
	{
	  superFrog.setVecInclinationRel(rotateScale * dt);
	}
      if(keyMask & rightMask)
	{
	  superFrog.setVecInclinationRel(-rotateScale * dt);
	}

      tLast = t.getT();          // Reset to current time.
    }
  else
    keyPressed = false;
}


void resetVectorIfOutOfBounds()
{
  using world::dynamicState::frog::superFrog;
    
  if(superFrog.getVec().getMag() > superFrog.restrictedModeMaxVectorMagnitude || superFrog.getVec().getMag() < -superFrog.restrictedModeMaxVectorMagnitude)
    {
      if(superFrog.getVec().getMag() > 0)
	superFrog.setVecMagAbs(superFrog.restrictedModeMaxVectorMagnitude);
      else
	superFrog.setVecMagAbs(-superFrog.restrictedModeMaxVectorMagnitude);
    }
}


/*  */
bool staticFrogCollisions()
{
  using world::dynamicState::frog::superFrog;
  using world::axis::xIndex;
    using world::axis::yIndex;
  
  bool ret {false};
  // First check to see if we have been hit by a car :'(
  if(carsCollision(superFrog.getPos()[xIndex], superFrog.getPos()[yIndex], superFrog.getHitRadius()))
    {
      {}
    }
  else
    {
      {}
    }

  return ret;
}


void updateJumpingFrog()
{ /* This function isn't very general. Part of the reason for this is that we're not exactly sure how to go about it
     and another part of the reason is that we don't have a very uniform data structure or representation of the world
     coordinates to work with */
  //  using world::landscape::ground::sec1StX;
  using world::axis::minX;
  using world::axis::maxX;
  using world::axis::minY;
  using world::axis::maxY;
  using world::axis::minZ;
  using world::axis::maxZ;
  using namespace world::dynamicState;
  using namespace frog;
  using namespace vectorAxis;

  float xFuture {superFrog.getPos()[xIndex] + superFrog.getVec().getXComp() * world::dynamicState::t.getDt()},
    yFuture {superFrog.getPos()[yIndex] + superFrog.getVec().getYComp() * world::dynamicState::t.getDt()},
      zFuture {superFrog.getPos()[zIndex] + superFrog.getVec().getZComp() * world::dynamicState::t.getDt()};
      float xNew {}, yNew {};//, zNew {};		// What is our new position?
      if(superFrog.isFalling())
	{
	  xFuture = superFrog.getPos()[xIndex];
	  yFuture = {superFrog.getPos()[yIndex] - g * world::dynamicState::t.getDt()};
	  zFuture = superFrog.getPos()[zIndex];
	}

    // The ordering here is important!
    if(groundSection1Collision(xFuture, yFuture, zFuture, radius, yNew))
      {
	if(world::dynamicState::restrictedMode)
	  resetVectorIfOutOfBounds();
	finishedJumping(yNew);
      }
    else
      {
	if(horizontalLineCollision(xFuture, yFuture, radius, world::landscape::ground::sec2StX, world::landscape::ground::sec2EnX,
				   world::landscape::ground::sec2StY, yNew))
	  {			// Check for collision with 2nd ground section
	    if(world::dynamicState::restrictedMode)
	      resetVectorIfOutOfBounds();
	    finishedJumping(yNew);
	  }
	else
	  {
	    if(horizontalLineCollision(xFuture, yFuture, radius, world::landscape::road::sec2StX, world::landscape::road::sec2EnX,
				       world::landscape::road::sec2StY, yNew))
	      {			// Check for collision with top of road
		if(world::dynamicState::restrictedMode)
		  resetVectorIfOutOfBounds();
		finishedJumping(yNew);
	      }
	    else
	      {
		if(vertLineLeftCollision(xFuture, yFuture, radius, world::landscape::road::sec1StX,
					 world::landscape::road::sec1EnY, world::landscape::road::sec1StY, xNew))
		  {			// Check for collision with left side of road
		    if(world::dynamicState::restrictedMode)
		      resetVectorIfOutOfBounds();
		    finishRoadCollision(xNew, yFuture, zFuture, radius, world::landscape::ground::sec1StY);
		  }
		else
		  {
		    if(vertLineRightCollision(xFuture, yFuture, radius, world::landscape::road::sec3StX,
					      world::landscape::road::sec3EnY, world::landscape::road::sec3StY, xNew))
		      {			// Check for collision with right side of road
			if(world::dynamicState::restrictedMode)
			  resetVectorIfOutOfBounds();
			finishRoadCollision(xNew, yFuture, zFuture, radius, world::landscape::ground::sec3StY);
		      }
		    else
		      {
			if(!carsCollision(xFuture, yFuture, radius))
			  {	 // If we didn't collide with any buildings :)
			    if(!handleLogCollision(xFuture, yFuture, zFuture))
			      {	// We didn't need to handle a log collision because we didn't hit one
				if(riverCollision(xFuture, yFuture, world::dynamicState::frog::radius, yNew))
				  {
				    superFrog.setDead(true);
				    finishedJumping(yNew);
				  }
				else
				  {
				    if(vertLineRightCollision(xFuture, yFuture, radius, world::landscape::riverbed::sec1StX,
							      world::landscape::riverbed::sec1StY,
							      world::landscape::riverbed::sec1EnY, xNew))
				      {		// Check for collision with left side of river (frog comming from right)
					superFrog.setPos(xNew, yFuture, zFuture);
				      }
				    else
				      {
					if(vertLineLeftCollision(xFuture, yFuture, radius, world::landscape::riverbed::sec3StX,
								 world::landscape::riverbed::sec3EnY,
								 world::landscape::riverbed::sec3StY, xNew))
					  {	// Check for collision with right side of river (frog comming from left)
					    superFrog.setPos(xNew, yFuture, zFuture);
					  }
					else
					  {
					    if(groundSection3Collision(xFuture, yFuture, radius, yNew))
					      {
						std::cout<<"hello\n";
						superFrog.setDead(true);
						superFrog.setWon(true);
						finishedJumping(yNew);
					      }
					    else
					      {
						superFrog.setXPos(xFuture);
						superFrog.setYPos(yFuture);
						superFrog.setZPos(zFuture);
					      }
					  }
				      }
				  }
			      }
			  }
		      }
		  }
	      }
	  }
      }	// :'(

    // Gravity will bring the frog down :(
    superFrog.setVecAbs(superFrog.getVec().getXComp(),
			superFrog.getVec().getYComp() - g * world::dynamicState::t.getDt(),
			superFrog.getVec().getZComp());
}


void updateFrogOnLog()
{
  using namespace world::dynamicState;
  using frog::superFrog;
  bool foundLogOn {false};	// Have we found a log the frog is on?

  /* This is not ideal but we can't use a pointer or reference to the log
     because it's address might change and since there are only a small
     amount of logs the overheader from doing this should be pretty low. */
  for(river::logs::mobileLogs log: river::logs::logs)
    {
      if(log.frogOnThisLog)
	{
	  superFrog.setYPos(log.yOffset + frog::radius + river::logs::radius);
	  superFrog.setZPos(log.zOffset + log.frogZOffset);
	  foundLogOn = true;
	}
    }
  if(!foundLogOn)		// Our log has dissapeared :'(
    {
      superFrog.setOnLog(false);
      superFrog.setJumping(true); // We are falling off the log (jumping.)
      superFrog.setFalling(true);
    }
}


bool groundSection1Collision(const float xFuture, const float yFuture, const float zFuture, const float radius, float & yNew)
{
  using world::landscape::ground::sec1EnX;
  using world::landscape::ground::sec1StZ;
  using world::landscape::ground::sec1EnZ;
  using world::landscape::ground::sec1StY;
  
  bool ret {false};

  if((xFuture + radius) <= sec1EnX)
    {
      //      if((zFuture + radius) >= sec1StZ && (zFuture - radius) <= sec1EnZ)
      if((yFuture - radius) < sec1StY)
	{
	  yNew = sec1StY + radius;	  
	  ret = true;
	}
    }

  return ret;
}


void finishedJumping(const float yNew)
{
  using namespace world::dynamicState;
  using namespace frog;
  
  superFrog.setJumping(false);
  superFrog.setYPos(yNew);
  superFrog.setVec(SphericalCoords(superFrog.getVec().getMag(), -SphericalCoords::radToDeg(superFrog.getVec().getAzimuth()),
			    SphericalCoords::radToDeg(superFrog.getVec().getInclination())));
}


void finishRoadCollision(const float xNew, const float yFuture, const float zFuture, const float radius, const float yGround)
{
  world::dynamicState::frog::superFrog.setPos(xNew, yFuture, zFuture);
  if(yFuture - radius <= yGround)
    {
      finishedJumping(yGround + radius);
    }
}


bool carsCollision(const float xFuture, const float yFuture, const float radius)
{
  using world::dynamicState::frog::superFrog;
  using namespace world::dynamicState::cars;
  using namespace vectorAxis;
  using world::landscape::road::sec1EnX;
  using world::landscape::road::sec1EnY;
  using world::landscape::cars::length;
  using world::landscape::cars::height;
  using world::landscape::cars::width;
  
  bool ret {false};

  for(car c: cars)
    {
      float yNew {superFrog.getPos()[vectorAxis::yIndex]};
      float a {};
      
      if(superFrog.getPos()[zIndex] >= (c.zOffset) && superFrog.getPos()[zIndex] <= (c.zOffset + width))
	{			// We've intersected with a car in the z axis.
	  if(vertLineLeftCollision(xFuture, yFuture, superFrog.getHitRadius(), sec1EnX + abs(c.xOffset), sec1EnY + abs(c.yOffset) + height,
				   sec1EnY + abs(c.yOffset), a))
	    {
	      ret = true;
	      superFrog.setDead(true);
	      finishedJumping(yNew);
	    }
	  else
	    {
	      if(vertLineRightCollision(xFuture, yFuture, superFrog.getHitRadius(), sec1EnX + abs(c.xOffset) + length,
					sec1EnY + abs(c.yOffset) + height, sec1EnY + abs(c.yOffset), a))
		{
		  ret = true;
		  superFrog.setDead(true);
		  finishedJumping(yNew);
		}
	      else
		{
		  if(horizontalLineCollision(xFuture, yFuture, superFrog.getHitRadius(), sec1EnX + abs(c.xOffset),
					     sec1EnX + abs(c.xOffset) + width, sec1EnY + abs(c.yOffset) + height, a))
		    {
		      ret = true;
		      superFrog.setDead(true);
		      finishedJumping(yNew);
		    }
		}
	    }
	}      
    }
  
  return ret;
}


bool handleLogCollision(const float xFuture, const float yFuture, const float zFuture)
{
  using namespace vectorAxis;
  using world::dynamicState::frog::superFrog;
  
  bool ret {false}, sideOfLog {false}; // False = left
  float yNew {};
  
  if(logCollision(xFuture, yFuture, sideOfLog, world::dynamicState::frog::radius, world::dynamicState::river::logs::radius))
    {		// We've collided with a log
      ret = true;
      
      if(logCollisionCenter(xFuture, yFuture, zFuture, yNew, true))
	{ // And we've hit it in a good enough spot to stay on it!
	  world::dynamicState::frog::superFrog.setOnLog(true);
	  finishedJumping(yNew);
	}
      else
	{			// Move away from the log (bounce off it if you will)
	  using world::dynamicState::river::logs::bounceBackXFactor;
	  
	  if(!sideOfLog)
	    {	// On left side of log
	      xFuture > superFrog.getPos()[xIndex] ?
		superFrog.setXPos((superFrog.getPos()[xIndex] -= bounceBackXFactor * (xFuture - superFrog.getPos()[xIndex]))):
		superFrog.setXPos((superFrog.getPos()[xIndex] += bounceBackXFactor * (xFuture - superFrog.getPos()[xIndex])));
	    }
	  else
	    {	// On right side
	      xFuture > superFrog.getPos()[xIndex] ?
		superFrog.setXPos(superFrog.getPos()[xIndex] += bounceBackXFactor*(xFuture - superFrog.getPos()[xIndex])):
		superFrog.setXPos(superFrog.getPos()[xIndex] -= bounceBackXFactor*(xFuture - superFrog.getPos()[xIndex]));
	    }
	  superFrog.setYPos(superFrog.getPos()[yIndex] += yFuture / world::dynamicState::river::logs::bounceBackYFactor);
	}
    }
  return ret;
}


bool logCollision(const float xFuture, const float yFuture, bool & sideOfLog, const float fRadius, const float lRadius)
{
  using world::axis::xIndex;
  using world::axis::yIndex;
  using world::axis::zIndex;
  using world::dynamicState::frog::radius;
  using world::dynamicState::river::logs::logs;
  using world::dynamicState::frog::superFrog;
  using world::dynamicState::river::logs::logLen;
  
  bool ret {false};

  for(world::dynamicState::river::logs::mobileLogs l: logs)
    {
      if(superFrog.getPos()[zIndex] > l.zOffset && superFrog.getPos()[zIndex] < (l.zOffset + logLen))
	{
	  if(pythagorean(xFuture, yFuture, l.xOffset, l.yOffset)
	     <= (fRadius + lRadius))
	    {
	      sideOfLog = l.xOffset > world::dynamicState::frog::superFrog.getPos()[xIndex] ? false : true;
	      ret = true;
	    }
	}
    }
  
  return ret;
}


bool logCollisionCenter(const float xFuture, const float yFuture, const float zFuture, float & yNew, const bool updateFrogOnThisLog)
{/* Has (world::dynamicState::frog::x +/- world::dynamicState::frog::radius) hit within
    world::dynamicState::river::logs::innerRadius of a log? */
  using namespace world::axis;
  using world::dynamicState::frog::superFrog;
  using world::dynamicState::frog::radius;
  using world::dynamicState::river::logs::innerRadius;
  using world::dynamicState::river::logs::logs;

  bool ret {false};

  for(long unsigned iter {}; iter < logs.size(); ++iter)
    {
      if(zFuture > logs[iter].zOffset &&
	 zFuture < (logs[iter].zOffset + world::dynamicState::river::logs::logLen))
	{
	  if((xFuture - radius) >= (logs[iter].xOffset -innerRadius) && (xFuture + radius) <= (logs[iter].xOffset) +innerRadius)
	    {
	      if(pythagorean(xFuture, yFuture, logs[iter].xOffset, logs[iter].yOffset)
		 <= (radius + world::dynamicState::river::logs::radius))
		{			// We've landed safely on a log!
		  ret = true;
		  if(updateFrogOnThisLog)
		    {
		      logs[iter].frogOnThisLog = true;
		      logs[iter].frogZOffset = (superFrog.getPos()[zIndex] - logs[iter].zOffset);
		    }
		  yNew = world::dynamicState::frog::superFrog.getPos()[vectorAxis::yIndex];
		  break;
		}
	    }
	}
    }

  return ret;
}


bool riverCollision(const float xFuture, const float yFuture, const float radius, float & yNew)
{
  using world::axis::xIndex;
  using world::axis::yIndex;
  using world::axis::zIndex;
  using world::dynamicState::river::waveVertices;

  bool ret {false};

  /* Would be greatly improved by using binary search (waveVerticies is computer linearly and thus the x values should be increasing) */
  for(int iter {}, iterNext {1}; iter < world::dynamicState::river::lastWaveTesselation.get() -1; ++iter, ++iterNext)
    {
      float yVertice {};
      if(xFuture == waveVertices[zIndex][iter][xIndex])
	{			// We have potentially gone directly below a waveVertice
	  yVertice = waveVertices[zIndex][iter][yIndex];
	  if((yFuture - radius) < yVertice)
	    {			// We are directly below a waveVertice
	      yNew = yVertice + radius;
	      ret = true;
	      break;
	    }
	}
      else
	{
	  if(xFuture == waveVertices[zIndex][iterNext][xIndex])
	    { // We have potentially gone directly below a waveVertice
	      yVertice = waveVertices[zIndex][iterNext][yIndex];
	      if((yFuture - radius) < yVertice)
		{		// We are directly below a waveVertice
		  yNew = yVertice + radius;
		  ret = true;
		  break;
		}
	    }
	  else
	    {
	      if(xFuture > waveVertices[zIndex][iter][xIndex] && xFuture < waveVertices[zIndex][iterNext][xIndex])
		{		// We have potentially gone below the line between two waveVertices
		  yVertice = {(waveVertices[zIndex][iter][yIndex] + waveVertices[zIndex][iterNext][yIndex]) /2};
		  if((yFuture - radius) < yVertice)
		    {		// We are below the line between two waveVertices
		      yNew = (yVertice);
		      ret = true;
		      break;
		    }
		}
	    }
	}
    }
  return ret;
}


bool groundSection3Collision(const float xFuture, const float yFuture, const float radius, float & yNew)
{
  using world::landscape::ground::sec3StY;

  bool ret {false};
  
  if((xFuture - radius) >= world::landscape::ground::sec3StX)
    {
      if((yFuture - radius) < sec3StY)
	{
	  yNew = (sec3StY + radius);
	  ret = true;
	}
    }
  
  return ret;
}


// Should abstract out the essense of this function into another function!
void handleBoarderCollision(const float xFuture, const float yFuture, const float zFuture,
					    const float radius, const float minX, const float maxX,
					    const float minY, const float maxY, const float minZ, const float maxZ)
{				
  using world::dynamicState::frog::superFrog;
  
  if((xFuture - radius) < minX)
    superFrog.setXPos(minX + radius);
  else
    if((xFuture + radius) > maxX)
      superFrog.setXPos(maxX - radius);

  if((yFuture - radius) < minY)
    superFrog.setYPos(minY + radius);
  else
    if((yFuture + radius) > maxY)
      superFrog.setYPos(maxY - radius);

  if((zFuture - radius) < minZ)
    superFrog.setZPos(minZ + radius);
  else
    if((zFuture + radius) > maxZ)
      superFrog.setZPos(maxZ - radius);
}


void updateDeadFrogPos()
{
  using namespace world::dynamicState::frog;
  using namespace vectorAxis;
  using world::dynamicState::frog::superFrog;
  using world::dynamicState::river::waveVertices;
  using world::dynamicState::river::lastWaveTesselation;
  // Are we in the river?
  if(superFrog.getPos()[xIndex] >= waveVertices[0][0][xIndex] &&
     superFrog.getPos()[xIndex] <=
     waveVertices[lastWaveTesselation.get() -1][lastWaveTesselation.get() -1][xIndex])
    for(int iter {}, iterNext {1}; iter < lastWaveTesselation.get() -1; ++iter, ++iterNext)
      {
	constexpr int zIndex {};		// All vertices allong z have the same y height.
	if(superFrog.getPos()[xIndex] == waveVertices[zIndex][iter][xIndex])
	  {
	    superFrog.setYPos(waveVertices[zIndex][iter][yIndex] + radius);
	  }
	else
	  {
	    if(superFrog.getPos()[xIndex] == waveVertices[zIndex][iterNext][xIndex])
	      {
		superFrog.setYPos(waveVertices[zIndex][iterNext][yIndex] + radius);
	      }
	    else
	      {
		if(superFrog.getPos()[xIndex] > waveVertices[zIndex][iter][xIndex] &&
		   superFrog.getPos()[xIndex] < waveVertices[zIndex][iterNext][xIndex])
		  {
		    superFrog.setYPos((waveVertices[zIndex][iter][yIndex] +
				       waveVertices[zIndex][iterNext][yIndex]) /2 + radius);
		  }
	      }
	  }
      }
}


void  updateFrogTrajectory()
{      /* We compute line segments for a trajectory at one level of granularity and then when we are close we start
	  computing at a finer level of granularity. When we finally hit yTouch we set y to yTouch */
  /* All of these using statements are unfortunate but we don't want to give x and y
     differnt names */
  using namespace vectorAxis;
  using world::dynamicState::frog::superFrog;
  using world::dynamicState::frog::trajectoryScaleFactor;
  using world::dynamicState::frog::_2ndTrajectoryScaleFactor;
  using world::dynamicState::frog::g;
  using world::dynamicState::frog::trajectory;
  
  float x {superFrog.getPos()[xIndex]},
    y {superFrog.getPos()[yIndex]},
    z {superFrog.getPos()[zIndex]},      
	xV {superFrog.getVec().getXComp()},
	  yV {superFrog.getVec().getYComp()},
	    zV {superFrog.getVec().getZComp()};

  int iter {};
  float yTouch {world::axis::minY}, xTouch {}, zTouch {};
  
  trajectory.clear();		// Clear the trajectory vector as we are only using .push_back()
  
  do
    {
      trajectory.push_back(x);
      trajectory.push_back(y);
      trajectory.push_back(z);
      x += (xV/trajectoryScaleFactor);
      y += (yV/trajectoryScaleFactor);
      z += (zV/trajectoryScaleFactor);
      yV -= (g/trajectoryScaleFactor);
      ++iter;

      if(trajectoryLineIntersect(x, y, z, xTouch, yTouch, zTouch, world::dynamicState::river::logs::trajectoryLineTouchingLog,
				 world::dynamicState::river::logs::potentialLogHit))
	break;
    }while(y > yTouch);// add " && x > xTouch" ?

  yV += (g/trajectoryScaleFactor); // Roll back a bit :)
  // MUST BE DONE IN REVERSE OPERATION ORDER (MAYBE SOMETHING TO DO WITH IEEE 754 ACCURACY)
  x -= (xV/trajectoryScaleFactor);
  y -= (yV/trajectoryScaleFactor);
  z -= (zV/trajectoryScaleFactor);

  do
    {                           // Get closer :)      
      trajectory.push_back(x);
      trajectory.push_back(y);
      trajectory.push_back(z);
      x += (xV/_2ndTrajectoryScaleFactor);
      y += (yV/_2ndTrajectoryScaleFactor);
      z += (zV/_2ndTrajectoryScaleFactor);
      yV -= (g/_2ndTrajectoryScaleFactor);
      ++iter;
    }while(y > yTouch);// add " && x > xTouch" ?
}


bool trajectoryLineIntersect(const float x, const float y, const float z, float & xTouch, float & yTouch, float & zTouch,
			     bool & trajectoryLineTouchingLog, bool & potentialLogHit)
{ // Yes this is a long function and no I don't really care :). I would probably split it up a bit but I don't really have time for it.
  using namespace::world::landscape;

  bool ret {false};

  if(groundSection1Collision(x, y, z, 0, yTouch))
    {
      trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
    }
  else
    {
      if(horizontalLineCollision(x, y, 0, world::landscape::ground::sec2StX, world::landscape::ground::sec2EnX,
				 world::landscape::ground::sec2StY, yTouch))
	{                     // Check for collision with 2nd ground section
	  trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
	}
      else
	{
	  if(horizontalLineCollision(x, y, 0, world::landscape::road::sec2StX, world::landscape::road::sec2EnX,
				     world::landscape::road::sec2StY, yTouch))
	    {                 // Check for collision with top of road
	      trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
	    }
	  else
	    {
	      if(vertLineLeftCollision(x, y, 0, world::landscape::road::sec1StX,
				       world::landscape::road::sec1EnY, world::landscape::road::sec1StY, xTouch))
		{                     // Check for collision with left side of road
		  trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
		  yTouch = y;
		}
	      else
		{
		  if(vertLineRightCollision(x, y, 0, world::landscape::road::sec3StX,
					    world::landscape::road::sec3EnY, world::landscape::road::sec3StY, xTouch))
		    {                 // Check for collision with right side of road
		      trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
		      yTouch = y;
		    }
		  else
		    {	
		      bool placeHolder {};
		      if(logCollision(x, y, placeHolder, 0,  world::dynamicState::river::logs::radius))
			{
			  trajectoryLineTouchingLog = true;
			  potentialLogHit = false;
			  {
			    float yPlaceholder {};
			    if(logCollisionCenter(x, y, z, yPlaceholder, false))
			      {
				trajectoryLineTouchingLog = true;
				potentialLogHit = true;
			      }
			  }
			  yTouch = y;
			  ret = true;
			}
		      else
			{
			  if(riverCollision(x, y, 0, yTouch))
			    {
			      trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit, ret);
			      world::dynamicState::river::trajectoryLineTouchingWater = true;
			    }
			  else
			    {
			      if(vertLineRightCollision(x, y, 0, world::landscape::riverbed::sec1StX,
							world::landscape::riverbed::sec1StY,
							world::landscape::riverbed::sec1EnY, xTouch))
				{               // Intersection with left side of river
				  trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit,
								   ret);
				  yTouch = y;
				}
			      else
				{
				  if(vertLineLeftCollision(x, y, 0, world::landscape::riverbed::sec3StX,
							   world::landscape::riverbed::sec3EnY,
							   world::landscape::riverbed::sec3StY, xTouch))
				    {   // Intersection with right side of river
				      trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit,
								       ret);
				      yTouch = y;
				    }
				  else
				    {
				      if(groundSection3Collision(x, y, 0, yTouch))
					{
					  trajectoryLineIntersectExtraWork(trajectoryLineTouchingLog, potentialLogHit,
									   ret);
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
  return ret;
}


void trajectoryLineIntersectExtraWork(bool & trajectoryLineTouchingLog, bool & potentialLogHit, bool & ret)
{
  trajectoryLineTouchingLog = false;
  world::dynamicState::river::trajectoryLineTouchingWater = false;
  potentialLogHit = false;
  ret = true;
}


void updateHud()
{
  using namespace hud; // There appears to be some initial inaccuracy in fr when the program is first run

  statTime.update();
    
  if((statTime.getT() - tLast) > updateInterval)
    {
      fr = frames / (statTime.getT() - tLast);
      ft = (statTime.getT() - tLast) / frames;
      tLast = statTime.getT();
      frames = 0;
    }
}
#include <GL/gl.h>
#include "include/normals.hpp"


namespace normals
{
  bool drawNormals {false};
}
#include <cmath>
#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "include/utility.hpp"
#include "include/normals.hpp"


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
#include "include/world.hpp"
#include "include/normals.hpp"


namespace world
{
  namespace dynamicState
  {
    Camera camera {frog::initXRot, frog::initYRot, frog::initZoom, frog::camHeight};
    Time t;
    Time tWaveTime;
    Tesselation tesselation {};
    bool drawAxis {true};
    bool wireframe {false};
    bool drawTexture {true};
    bool pause {true};
    bool restrictedMode {false};
    //    bool drawNormals {false};
    char keyMask {};

    namespace gameScore
    {
      int lives {5};
      int score {};
    }

    namespace skybox
    {				// :)
      GLuint textureHandleNegX;
      GLuint textureHandleNegY;
      GLuint textureHandleNegZ;
      GLuint textureHandlePosX;
      GLuint textureHandlePosY;
      GLuint textureHandlePosZ;
    }

    namespace ground
    {
      GLuint textureHandle {};
    }

    namespace road
    {
      GLuint textureHandle {};
    }

    namespace cars
    {
      std::vector<car> cars {};
    }

    namespace riverbed
    {
      GLuint textureHandle {};
    }

    namespace river
    {
      float waveVertices[Tesselation::tMax + additionalVerticesZ][Tesselation::tMax + additionalVerticesX]
      [axis::dimensions] {};
      Tesselation lastWaveTesselation {dynamicState::tesselation};
      bool trajectoryLineTouchingWater {false};

      namespace logs
      {
	//	float logXVertices[numLogsX][axis::dimensions] {};
	std::vector<float> logZVertices {};
	std::vector<mobileLogs> logs {};
	bool trajectoryLineTouchingLog {false};
	bool potentialLogHit {false};
	GLuint textureHandle {};
      }
    }
    
    namespace frog
    {
      Frogject superFrog {initX, initY, initZ, radius, initVecR, initVecAzimuth, initVecInclination, restrictedVecMax,
	  scaleFactor,
	  normals::normScale};
      long logOn {};
      std::vector<float> trajectory {};
    }
  }
}
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
#ifndef FROGJECT_HPP_
#define FROGJECT_HPP_


#include <iostream>
#include "SphericalCoords.hpp"
#include "Tesselation.hpp"
#include "utility.hpp"		// For abs(), <cmath> version and other version causing compiler to giver error's :(


class Frogject
{
  /* -----------------------------------------------------------------------------------------------------------------
     Member veriables  -----------------------------------------------------------------------------------------------
     -------------------------------------------------------------------------------------------------------------- */
  private:
  /* We only want to allow one instantiation of Frogject because of the use of the anynomous namespace with
     non-const veriables in Frogject.cpp */
  static bool instantiated;
  const vectorAxis::tCoords hitRadius;		// The frog's hit box is modeled as a sphere.
  vectorAxis::tCoords pos [vectorAxis::dimensions];
  vectorAxis::tCoords posInit [vectorAxis::dimensions];
  SphericalCoords vector;		// Jump vector
  Tesselation t {};
  vectorAxis::tCoords frogScale;
  vectorAxis::tCoords normScale;
  bool dead {false};
  bool won {false};
  bool jumping {false};
  bool falling {false};
  bool onLog {false};

  class box
  { /* We are effectively using these inner classes as namespaces as we made this frog before we decided to make it an
       object and as another consequence of this the features of this class will probably be fairly limited, that is
       however also largely because of time constraint's. */
  public:
    static constexpr vectorAxis::tCoords tFL [] {0.0, 1.0, 0.0};
    static constexpr vectorAxis::tCoords tFR [] {0.0, 1.0, 1.0};
    static constexpr vectorAxis::tCoords tBL [] {1.0, 1.0, 0.0};
    static constexpr vectorAxis::tCoords tBR [] {1.0, 1.0, 1.0};
    static constexpr vectorAxis::tCoords bFL [] {0.0, 0.0, 0.0};
    static constexpr vectorAxis::tCoords bFR [] {0.0, 0.0, 1.0};
    static constexpr vectorAxis::tCoords bBL [] {1.0, 0.0, 0.0};
    static constexpr vectorAxis::tCoords bBR [] {1.0, 0.0, 1.0};
  };

public:
  const vectorAxis::tCoords restrictedModeMaxVectorMagnitude;

  /* -----------------------------------------------------------------------------------------------------------------
     Member functions ------------------------------------------------------------------------------------------------
     -------------------------------------------------------------------------------------------------------------- */
private:
  void checkInstance()
  {
    if(instantiated)
      {
	std::cerr<<"Error: there can be only one instance of Frogject at any one time!\n";
	exit(INSTANCE_ERROR);
      }
    instantiated = true;
  }
  void drawTorsoOrBraincase(const vectorAxis::tCoords xNormScale, const vectorAxis::tCoords yNormScale,
			    const vectorAxis::tCoords zNormScale, const vectorAxis::tCoords normDrawScaleScale);
  void drawTorsoOrBraincaseNormals(const vectorAxis::tCoords xNormScale,
					     const vectorAxis::tCoords yNormScale,
					     const vectorAxis::tCoords zNormScale,
					     const vectorAxis::tCoords normDrawScaleScale);
  void drawLegs();
  void drawFrontLeg(const bool leftLeg); // Draw left leg if true right leg if false.
  void drawFrontShoulder();
  void drawFrontLowerLeg();
  void drawFrontFeet();
  void drawFrontToes();
  void drawBackLeg(const bool leftLeg);
  void drawBackShoulder();
  void drawBackLowerLeg();
  void drawBackLowerLowerLeg();
  void drawBackFeet();
  void drawBackToes();
  void drawHead();
  void drawEye(const bool leftEye);
  
public:	// initVecR & initVecTheta are though to be sufficiently closely related to coordinates to use tCoords.
  Frogject(const vectorAxis::tCoords x, const vectorAxis::tCoords y, const vectorAxis::tCoords z,
	   const vectorAxis::tCoords hitBoxRadius, const vectorAxis::tCoords initVecR,
	   const vectorAxis::tCoords initVecAzimuth, const vectorAxis::tCoords initVecInclination,
	   const vectorAxis::tCoords restriced, const vectorAxis::tCoords frogScale,
	   const vectorAxis::tCoords normScale) :
    hitRadius(hitBoxRadius), vector(initVecR, initVecAzimuth, initVecInclination), restrictedModeMaxVectorMagnitude(restriced)
  {
    using namespace vectorAxis;
    checkInstance();
    posInit[xIndex] = x;
    posInit[yIndex] = y;
    posInit[zIndex] = z;
    pos[xIndex] = x;
    pos[yIndex] = y;
    pos[zIndex] = z;
    this->frogScale = frogScale;
    this->normScale = normScale * (1 / frogScale);
    //    this->trajectory = trajectory;
  }

  ~Frogject()
  {
    instantiated = false;
  }

  void draw();
  std::vector<vectorAxis::tCoords> getPos() const;
  const SphericalCoords & getVec() const;
  vectorAxis::tCoords getHitRadius() const;
  bool isDead() const;
  bool hasWon() const;
  bool isJumping() const;
  bool isFalling() const;
  bool isOnLog() const;
  void setPos(const vectorAxis::tCoords x, const vectorAxis::tCoords y, const vectorAxis::tCoords z);
  void setXPos(const vectorAxis::tCoords x);
  void setYPos(const vectorAxis::tCoords y);
  void setZPos(const vectorAxis::tCoords z);
  void resetPos();
  void setVec(const SphericalCoords & vector);
  void setVecAbs(const vectorAxis::tCoords xV, const vectorAxis::tCoords yV, const vectorAxis::tCoords zV);
  void setVecInclinationRel(const vectorAxis::tCoords inclination);
  void setVecAzimuthRel(const vectorAxis::tCoords azimuth);
  void setVecMagRel(const vectorAxis::tCoords m);
  void setVecMagAbs(const vectorAxis::tCoords m);
  void setDead(const bool dead);
  void setWon(const bool won);
  void setJumping(const bool jumping);
  void setFalling(const bool falling);
  void setOnLog(const bool onLog);
};


#endif
#ifndef RADDEG_HPP_
#define RADDEG_HPP_

#include <cmath>


class SphericalCoords
{
private:

  float r {}, azimuth {}, inclination {};

public:

  SphericalCoords(const float r, const float azimuth, const float inclination)
  {
    this->r = r;
    this->azimuth = degToRad(azimuth);
    this->inclination = degToRad(inclination);
  }

  static float degToRad(const float x)
  {
    return x * (M_PI / 180);
  }

  static float radToDeg(const float x)
  {
    return x * (180 / M_PI);
  }

  float getXComp() const
  {
    return r*sin(inclination)*cosf(azimuth);
  }

  float getYComp() const
  {
    return r*sinf(inclination)*sinf(azimuth);
  }

  float getZComp() const
  {
    return r*cosf(inclination);
  }

  float getInclination() const
  {
    return inclination;
  }

  float getAzimuth() const
  {
    return azimuth;
  }

  float getMag() const
  {
    return r;
  }

  //  void setAngleRel(const float inclination)
  void setInclinationRel(const float inclination)
  {
    this->inclination += inclination;
  }

  //  void setAngleAbs(const float inclination)
  void setInclinationAbs(const float inclination)
  {
    this->inclination = inclination;
  }

  void setAzimuthRel(const float azimuth)
  {
    this->azimuth += azimuth;
  }

  void setAzimuthAbs(const float azimuth)
  {
    this->inclination = azimuth;
    }

  void setMagRel(const float r)
  {
    this->r += r;
  }

  void setMagAbs(const float r)
  {
    this->r = r;
  }

  void setAbs(const float xV, const float yV, const float zV)
  {
    r = sqrt(powf(xV, 2) + powf(yV, 2) + powf(zV, 2));
    inclination = acos(zV / r);
    azimuth = atan2(yV, xV);
  }
};


#endif
#ifndef TESSELATION_HPP_
#define TESSELATION_HPP_


class Tesselation
{
private:
  /* t is static so that we can have multiple instances of Tesselation with a shared t value. For example Frogject
     has it's own Tesselation instance and this way it does not have to be explicitly told what t is at any given
     time. */
  static int t;
  static constexpr int step {2};
public:
  static constexpr int tMin {8};
  // I made it 256 and not 128 so that my water "reflection" would look nicer haha :)                              
  static constexpr int tMax {128};

  Tesselation()
  {
    t = tMin;
  }

  Tesselation(const Tesselation & t)
  {
    this->t = t.get();
  }

  Tesselation & operator=(const Tesselation & t)
  {
    this->t = t.get();
    return *this;
  }

  void increase()
  {
    if(t < tMax)
      t *= step;
  }

  void decrease()
  {
    if(t > tMin)
      t /= step;
  }

  int get() const
  {
    return t;
  }
};


#endif
#ifndef TIME_HPP_
#define TIME_HPP_


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


class Time
{
private:
  float t, tLast, scaleFactor {1000}; // Offset is used when pausing the game
public:
  Time()
  {
    t = glutGet(GLUT_ELAPSED_TIME) / scaleFactor;
    tLast = t;
  }

  Time(const Time & t)
  {
    this->t = t.t;
    this->tLast = t.tLast;
  }

  Time operator=(const Time & t)
  {
    this->t = t.t;
    this->tLast = t.tLast;
    return t;
  }

  void update()
  {
    tLast = t;
    t = (glutGet(GLUT_ELAPSED_TIME) / scaleFactor);
  }

  void updateT()
  {
    t = (glutGet(GLUT_ELAPSED_TIME) / scaleFactor);
  }

  float getT()
  {
    return t;
  }

  float getTLast()
  {
    return tLast;
  }

  float getDt()
  {
    return t - tLast;
  }
};


#endif
#ifndef NORMALS_HPP_
#define NORMALS_HPP_

namespace normals
{
  constexpr int light0 {GL_LIGHT0};
  constexpr float normScale {1.0};
  constexpr float normalLen {1.0}; // Un-scailed length of all normals
  constexpr float normDrawScale {normScale * 0.005};
  extern bool drawNormals;
  constexpr int rIndex {0}, gIndex {1}, bIndex {2};
  constexpr float color [] {1, 1, 0};
}

#endif
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
#ifndef WORLD_HPP_
#define WORLD_HPP_


#include <vector>
#include <array>
#include <cmath>
#include "Camera.hpp"
#include "Frogject.hpp"
#include "Tesselation.hpp"
#include "Time.hpp"
#include "SphericalCoords.hpp"


namespace world
{ 
  namespace axis
  {
    constexpr float xColor [] {1, 0, 0}, yColor [] {0, 1, 0}, zColor [] {0, 0, 1};
    constexpr int dimensions {3};
    constexpr float minX {-1}, maxX {-minX}, minY {minX}, maxY {maxX}, minZ {minX}, maxZ {maxX};
    //    static constexpr float zDepth {};
    constexpr float xR {1}, xG {0}, xB {0};
    constexpr float yR {0}, yG {1}, yB {0};
    constexpr float zR {0}, zG {0}, zB {1};
    constexpr float xXOrigin {0}, xYOrigin {0}, xZOrigin {0};
    constexpr float xX {maxX}, xY {0}, xZ {0};
    constexpr float yXOrigin {0}, yYOrigin {0}, yZOrigin {0};
    constexpr float yX {0}, yY {maxY}, yZ {0};
    constexpr float zXOrigin {0}, zYOrigin {0}, zZOrigin {0};
    constexpr float zX {0}, zY {0}, zZ {maxZ};
    // These veriables are duplicated in utility.hpp to get around mutural inclusion of header files.
    constexpr int xIndex {0}, yIndex {1}, zIndex {2};// Ordering convention of x, y & z when used in an array.
    constexpr int rIndex {0}, gIndex {1}, bIndex {2};
  }

  namespace lights
  {		      // The worlds lighting is defined in Camera.hpp.
    // Default material colours.
    constexpr float materialAmbient [] {1, 1, 1, 1};
    constexpr float materialDiffuse [] {1, 1, 1, 1};
    constexpr float materialSpecular [] {0.55, 0.1, 0.1, 0.5}; // Nice red specular, lava sky reflection.
    constexpr float materialShininess {30};
    constexpr int numColorDimensions {4}; // RGBA.
    constexpr int m1Index {0}, m2Index {1}, m3Index {2}, m4Index {3};
  }
  
  namespace landscape
  { // The way some of the coordinates are calculated here should be changed to abs(x2 - x1)!
    constexpr float zDepthOffset {0.0}; // Used for translations
    //      constexpr float skyReflectColorR {0.3}, skyReflectColorG {0.6}, skyReflectColorB {1.0};

    constexpr float angle1 {-90};
    constexpr float angle2 {180};
    constexpr float noXRot {}, noYRot {}, yRot {1}, noZRot {}, zRot {1};
    
    namespace ground
    {
      constexpr float materialAmbient [] {lights::materialAmbient[lights::m1Index],
	  lights::materialAmbient[lights::m2Index], lights::materialAmbient[lights::m3Index],
	  lights::materialAmbient[lights::m4Index]};
      constexpr float materialDiffuse [] {lights::materialDiffuse[lights::m1Index],
	  lights::materialDiffuse[lights::m2Index], lights::materialDiffuse[lights::m3Index],
	  lights::materialDiffuse[lights::m4Index]};
      constexpr float materialSpecular [] {lights::materialSpecular[lights::m1Index],
	  lights::materialSpecular[lights::m1Index], lights::materialSpecular[lights::m1Index],
	  lights::materialSpecular[lights::m1Index]};
      // st = start, ed = end, ln = length (got sick of typeing. Maybe not the best names idk :) )
      constexpr float sec1StX {-1}, sec1StY {0}, sec1StZ {sec1StX}; // First section (before road)
      constexpr float sec1EnX {-0.75}, sec1EnY {0}, sec1EnZ {1};
      // Needed for drawGrid() :)
      constexpr float sec1LnX {abs(sec1StX) - abs(sec1EnX)}, sec1LnZ {abs(sec1StZ) + abs(sec1EnZ)};
      constexpr float sec2StX {-0.25}, sec2StY {0}, sec2StZ {sec1StZ}; // Second section (after road)
      constexpr float sec2EnX {0}, sec2EnY {0}, sec2EnZ {sec1EnZ};
      constexpr float sec2LnX {abs(sec2StX) - abs(sec2EnX)}, sec2LnZ {abs(sec2StZ) + abs(sec2EnZ)};
      constexpr float sec3StX {0.75}, sec3StY {0}, sec3StZ {sec2StZ}; // Third section (after river)
      constexpr float sec3EnX {1}, sec3EnY {0}, sec3EnZ {sec2EnZ};
      constexpr float sec3LnX {abs(sec3EnX - sec3StX)}, sec3LnZ {abs(sec3StZ) + abs(sec3EnZ)};
      //      constexpr bool drawTexture {true};
      constexpr char textureName[] {"./textures/grass.jpg"};
      constexpr float textureScale {0.35}; // Texture coordinates should be multiplied by this number.
    }
    
    namespace road
    {
      constexpr float materialAmbient [] {lights::materialAmbient[lights::m1Index],
	  lights::materialAmbient[lights::m2Index], lights::materialAmbient[lights::m3Index],
	  lights::materialAmbient[lights::m4Index]};
      constexpr float materialDiffuse [] {lights::materialDiffuse[lights::m1Index],
	  lights::materialDiffuse[lights::m2Index], lights::materialDiffuse[lights::m3Index],
	  lights::materialDiffuse[lights::m4Index]};
      constexpr float materialSpecular [] {0.15, 0.15, 0.15, 0.9};
      constexpr float elevation {0.0416666};	   // Elevation of the road
      // Section 1 (left bank of the road)
      constexpr float sec1StX {ground::sec1EnX}, sec1StY {ground::sec1EnY}, sec1StZ {ground::sec1StZ};
      constexpr float sec1EnX {ground::sec1EnX}, sec1EnY {elevation}, sec1EnZ {ground::sec1EnZ};
      // Needed for drawGrid()
      constexpr float sec1LnY {abs(sec1EnY) - abs(sec1StY)}, sec1LnZ {abs(sec1StZ) + abs(sec1EnZ)};
      // Section 2 (main surface of the road)
      constexpr float sec2StX {ground::sec1EnX}, sec2StY {elevation}, sec2StZ {sec1StZ};
      constexpr float sec2EnX {ground::sec2StX}, sec2EnY {elevation}, sec2EnZ {sec1EnZ};
      // No need to calculate sec2LnX here because we do that latter with width.
      constexpr float sec2LnZ {abs(sec2StZ) + abs(sec2EnZ)};
      // Section 3 (right bank of the road)
      constexpr float sec3StX {ground::sec2StX}, sec3StY {ground::sec2EnY}, sec3StZ {sec2StZ};
      constexpr float sec3EnX {ground::sec2StX}, sec3EnY {elevation}, sec3EnZ {abs(sec2EnZ)};
      constexpr float sec3LnY {abs(sec3EnY) - abs(sec3StY)}, sec3LnZ {abs(sec3StZ) + abs(sec3EnZ)};
      // Road width. May have to be ajusted if the road is moved to a coordinate that intersects the y line.
      // (although most of this may have to be)
      constexpr float width {axis::xX - abs(abs(sec2StX) - abs(sec2EnX))};
      // Needed for handeling collisions with the banks of the road.
      constexpr float stepBack {0.007};
      constexpr char textureName[] {"./textures/road.png"};
      constexpr float bankTextureScale {0.15};	// Used to scale the texture's on the side of the road.
      constexpr float textureYOffset {0.00007}; // :'(
    };

    namespace cars
    {				// Colors.
      constexpr float alpha {1}, specAlpha {0.50};;
      const std::vector<float> materialAmbientDome {0.1, 0.1, 0.1, 0.85};
      const std::vector<float> materialDiffuseDome {0.1, 0.1, 0.1, 0.85};
      constexpr float materialShininessBody {0.0005}, materialShininessDome {6};;
      /* The two of the possible colour componements will be multiplied by toneDownColor and one by toneUpColor, the
	 one multiplied by tone up colour will also have colorAddition added to it. The specular highlight colors are
	 multiplied by specMull. */
      constexpr float toneDownColor {0.7}, toneUpColor {1.25}, colorAddition {0.15}, specMull {0.50};
      constexpr float length {0.09}, height {0.032}, width {0.17};
      constexpr float domeXStart {length / 2}, domeYStart {height / 2}, domeZStart {width / 2};
      constexpr float domeZScale {1.5}, domeRadius {length / (1 * 2)};
      constexpr float driveHeight {0.004};
      constexpr time_t updateRate {1 / 128}; // Update car position's 128 time's a second :)
      constexpr float updateDistance {0.0025}; // How much the car will move each update period.
      constexpr int numLanes {4}, lane1Index {0}, lane2Index {1}, lane3Index {2}, lane4Index {3};
      constexpr float laneGap {0.028};
      constexpr float laneOffsets [] {(length * 0) + (laneGap * 1), (length * 1) + (laneGap * 2),
	  (length * 2) + (laneGap * 3), (length * 3) + (laneGap * 4)};
      constexpr float collisionDist {0.5};
      constexpr bool laneDirections [] {false, false, true, true};
    };
    
    namespace riverbed
    {
      constexpr float elevation {-0.175};
      // Section 1 (left bank of river)
      constexpr float sec1StX {ground::sec2EnX}, sec1StY {ground::sec2EnY}, sec1StZ {ground::sec2StZ};
      constexpr float sec1EnX {ground::sec2EnX}, sec1EnY {elevation}, sec1EnZ {ground::sec2EnZ};
      constexpr float sec1LnY {abs(sec1EnY) - abs(sec1StY)}, sec1LnZ {abs(sec1StZ) + abs(sec1EnZ)};
      // Section 2 (river bed)
      constexpr float sec2StX {ground::sec2EnX}, sec2StY {elevation}, sec2StZ {sec1StZ};
      constexpr float sec2EnX {ground::sec3StX}, sec2EnY {elevation}, sec2EnZ {sec1EnZ};
      constexpr float sec2LnX {abs(sec2EnX) - abs(sec2StX)}, sec2LnZ {abs(sec2StZ) + abs(sec2EnZ)};
      // Section 3 (right bank of river)
      constexpr float sec3StX {ground::sec3StX}, sec3StY {elevation}, sec3StZ {sec2StZ};
      constexpr float sec3EnX {ground::sec3StX}, sec3EnY {ground::sec3StY}, sec3EnZ {sec2EnZ};
      constexpr float sec3LnY {abs(sec3StY) - abs(sec3EnY)}, sec3LnZ {abs(sec3StZ) + abs(sec3EnZ)};
      //      constexpr bool drawTexture {true};
      constexpr char textureName[] {"./textures/sand.jpg"};
      constexpr float textureScale {0.35}; // Texture coordinates should be multiplied by this number.
      constexpr float bankTextureScale {0.20};
    };
  };
  
  namespace dynamicState
  {/* daynamicState is reserved for those game objects which have some daynamic aspect to them and not just single
      variables that are part of some larger object */
    extern Camera camera;
    extern Time t;
    extern Time tWaveTime;
    extern Tesselation tesselation;
    extern bool drawAxis;
    extern bool wireframe;
    extern bool drawTexture;
    extern bool pause; // Control animation of things that change with time such as the frog and sine wave
    /* This is the real game mode where you can't just use hacked values for the frog's vector :)
       We've put this here even though it specifically relates to the frog because I consider it a more global
       property and if the game were extened in the future it might affect things other then just the frog! */
    extern bool restrictedMode;
    //    extern bool drawNormals;
    extern char keyMask;		// Store which key's are currently depressed :'(
    // Currently depressed keys :'(
    constexpr char incMask {0x01}, decMask {0x02}, forwardMask {0x08}, backwardMask {0x04},
      leftMask {16}, rightMask {32};

    namespace gameScore
    {
      constexpr char winningString [] {"o~<[! WINNER !]>~o"};
      constexpr char losingString [] {">:^( [! LOSER !] >:^("};
      constexpr int minLives {};
      extern int lives;
      constexpr int maxScore {5};
      extern int score;
    }
    
    namespace skybox
    {				// :)
      /* t = top, F = front, L = left, R = right. NOTE: change drawSkyBox() function to use coordinates from
	 namespace box in utility.cpp and remove coordinates defined here! */
      constexpr float tFL [] {-10, 10, -10}, tFR [] {10, 10, -10}, bFL [] {-10, -10, -10}, bFR [] {10, -10, -10},
	tBL [] {-10, 10, 10}, tBR [] {10, 10, 10}, bBL [] {-10, -10, 10}, bBR [] {10, -10, 10};
      constexpr char negXName [] {"./textures/skybox/negX.bmp"};
      constexpr char negYName [] {"./textures/skybox/negY.png"};
      constexpr char negZName [] {"./textures/skybox/negZ.bmp"};
      constexpr char posXName [] {"./textures/skybox/posX.bmp"};
      constexpr char posYName [] {"./textures/skybox/posY.bmp"};
      constexpr char posZName [] {"./textures/skybox/posZ.bmp"};
      extern GLuint textureHandleNegX;
      extern GLuint textureHandleNegY;
      extern GLuint textureHandleNegZ;
      extern GLuint textureHandlePosX;
      extern GLuint textureHandlePosY;
      extern GLuint textureHandlePosZ;
    }

    namespace ground
    {
      extern GLuint textureHandle;
    }

    namespace road
    {
      extern GLuint textureHandle;
    }

    namespace cars
    {
      struct car
      {
	float xOffset, yOffset, zOffset;
	int lane;
	bool laneDirection;
	// Colors.
	std::vector<float> materialAmbientBody;
	std::vector<float> materialDiffuseBody;
	std::vector<float> materialSpecular;
      };
      extern std::vector<car> cars;
    }

    namespace riverbed
    {
      extern GLuint textureHandle;
    }

    namespace river
    {
      constexpr float materialAmbient [] {0, 1, 1, 0.35};
      constexpr float materialDiffuse [] {0, 1, 1, 0.45};
      constexpr float materialShininess {18};
      // We assume that landscape::riverbed::sec1EnY < landscape::riverbed::sec1StY
      constexpr float stX {landscape::riverbed::sec2StX}, enX {landscape::riverbed::sec2EnX};
      constexpr float stZ {landscape::riverbed::sec2StZ}, enZ {landscape::riverbed::sec2EnZ};
      constexpr float xLength {abs(enX) - abs(stX)}, zLength {abs(stZ) + abs(enZ)};
      constexpr float yRiverbed {landscape::riverbed::sec2StY}; // Height of the riverbed
      // Wave is draw both above and below this line hence the /2
      constexpr float yOffset {yRiverbed / 2};
      constexpr float zDepth {0};
      static constexpr float LAMBDA {0.5};
      constexpr float k1 {(3.5 * M_PI) / LAMBDA}, k2 {(2 * M_PI) / LAMBDA}, k3 {(5.5 * M_PI) / LAMBDA};
      // velocity
      constexpr float w1 {M_PI / 6.7}, w2 {M_PI / 10}, w3 {M_PI / 3.1};
      // The multipliers here should add up to 1 :).
      constexpr float A1 = {(abs(landscape::riverbed::sec1StY - landscape::riverbed::sec1EnY) / 3) * 0.65},
	A2 {(abs(landscape::riverbed::sec1StY - landscape::riverbed::sec1EnY) / 2) * 0.1},
	A3 {(abs(landscape::riverbed::sec1StY - landscape::riverbed::sec1EnY) / 2) * 0.25};
      /* We need n+1 verticies in the x and z direction where n is the number of segments and another aditional
	 one on each end in x for calculating the normals at the ends. */
      constexpr int additionalVerticesX {3};
      constexpr int additionalVerticesZ {1};
      extern float waveVertices[Tesselation::tMax + additionalVerticesZ][Tesselation::tMax + additionalVerticesX]
            [axis::dimensions];
      extern Tesselation lastWaveTesselation;
      extern bool trajectoryLineTouchingWater;

      namespace logs		// I refuse to believe that I've gone overboard with namespaces :^)
      {
	// General perameters. ---------------------------------------------------------------------------------------
	constexpr float materialAmbient [] {lights::materialAmbient[lights::m1Index],
	  lights::materialAmbient[lights::m2Index], lights::materialAmbient[lights::m3Index],
	  lights::materialAmbient[lights::m4Index]};
	constexpr float materialDiffuse [] {lights::materialDiffuse[lights::m1Index],
	  lights::materialDiffuse[lights::m2Index], lights::materialDiffuse[lights::m3Index],
	  lights::materialDiffuse[lights::m4Index]};
	// End of logs must have a different colour as they are not textured
	constexpr float materialAmbientEnd [] {0.22, 0.11, 0.035, 1.0};
	constexpr float materialDiffuseEnd [] {0.22, 0.11, 0.035, 1.0};
	constexpr float materialSpecular [] {lights::materialSpecular[lights::m1Index],
	    lights::materialSpecular[lights::m1Index], lights::materialSpecular[lights::m1Index],
	    lights::materialSpecular[lights::m1Index]};
	
	constexpr int numLogsX {5};
	constexpr float logLen {0.59};
	constexpr float offset {xLength / numLogsX}; // Offset between logs
	constexpr float initialOffset {offset /2};  // We don't want the first log to start at stX
	constexpr float radius {0.0465};
	constexpr float circumference {radius * 2};
	constexpr float angleEndCircle {180};
	constexpr float noXRot {}, yRot {1}, noZRot {};
	extern std::vector<float> logZVertices; // There can potentially be a veriable number of logs in Z
	// Movement stuff. -------------------------------------------------------------------------------------------
	constexpr float floatHeight {radius * 0.16}; // 0 = halfway 50% above river.
	constexpr time_t updateRate {1 / 128}; // Update logs 128 time's a second :)
	constexpr float updateDistance {0.0010};
	constexpr int numLanes {numLogsX}, lane1Index {0}, lane2Index {1}, lane3Index {2}, lane4Index {3},
	  lane5Index {4};
	constexpr float laneGap {((abs(landscape::riverbed::sec3StX) - abs(landscape::riverbed::sec1StX)) -
				  (circumference * numLanes)) / (numLanes + 1)};
	constexpr float laneOffsets [] {((circumference * lane1Index) + (laneGap * (lane1Index + 1)) + radius),
	    ((circumference * lane2Index) + (laneGap * (lane2Index + 1)) + radius),
	    ((circumference * lane3Index) + (laneGap * (lane3Index + 1)) + radius),
	    ((circumference * lane4Index) + (laneGap * (lane4Index + 1)) + radius),
	    ((circumference * lane5Index) + (laneGap * (lane5Index + 1)) + radius)};
	constexpr float collisionDist {0.7};
	constexpr bool laneDirections [] {false, true, false, true, false};
	struct mobileLogs
	{
	  float xOffset, yOffset, zOffset;
	  int lane;
	  bool laneDirection;
	  bool frogOnThisLog;	// :)
	  float frogZOffset;	// Store offset of frog on log if frog is on log :)
	};
	extern std::vector<mobileLogs> logs;	
	// Interaction stuff. ----------------------------------------------------------------------------------------
	// Radius of sphere drawn at end of trajectory line when it hit's a safe spot on the log to jump onto
	constexpr float bounceBackXFactor {0.9};
	constexpr float bounceBackYFactor {12880000000};
	extern bool trajectoryLineTouchingLog;
	extern  bool potentialLogHit; // Is the trajectory line touching a potential hit position
	constexpr float hitLogIndicatorRadius {0.007};
	// Must frog::x +/- frog::radius must be within innerRadius of center of log to land on log!
	constexpr float innerRadius {0.040};
	// Texture stuff. --------------------------------------------------------------------------------------------
	//	constexpr bool drawTexture {true};
	constexpr char textureName[] {"./textures/wood.jpg"};
	extern GLuint textureHandle;
	constexpr float textureScale {0.35}; // Texture coordinates should be multiplied by this number.
      }
    }

    namespace frog
    {
      // Frog's colour main information is specified in Frogject.cpp :(
      //      constexpr float trajectoryBallMissColor {1.0, 0.0, 0.1},
      //	trajectoryBallHitColor {0.0, 1.0, 0.1};
      constexpr float trajectoryLineAndTrajectoryVectorColor [] {1.0, 0.82, 0.58};
      constexpr float materialAmbientTrajectoryBallMiss [] {0.5, 0.0, 0.05, 1},
	materialAmbientTrajectoryBallHit [] {0.0, 0.5, 0.05, 1},
	materialAmbientTrajectory [] {0.5, 0.41, 0.29, 1};
      constexpr float materialDiffuseTrajectoryBallMiss [] {1.0, 0.0, 0.1, 1},
	materialDiffuseTrajectoryBallHit [] {0.0, 1.0, 0.1, 1},
	materialDiffuseTrajectory [] {1.0, 0.82, 0.58, 1};
      constexpr float restrictedVecMax {0.225};	 // Maximum frog vector magnitude in restricted mode :)
      extern long logOn;	
      extern Frogject superFrog;
      constexpr float scaleFactor {0.00325};			  // Scale the frog by this much before drawing :)
      constexpr float initXRot {90}, initYRot {0}, initZoom {0.12}; // Initial camera zoom
      constexpr float camHeight {0.058};
      constexpr float frogDrawYOffset {0.0035};
      /* We've decided to put g here in spite of it being a global property because it is only used for the frog
	 (and by the principal of least authority it should go here idk haha) */
      constexpr float g {0.25};
      constexpr float ratio {2.0}; // ellipse ratio. b = (radius/ratio)
      constexpr float radius {0.0165};
      constexpr float initX {landscape::ground::sec1StX
	  + ((abs(landscape::ground::sec1StX) - abs(landscape::ground::sec1EnX)) / 2)},
	initY {landscape::ground::sec1StY + radius}, initZ {0}; // Initial position
      constexpr float initVecR {restrictedVecMax}, initVecAzimuth {90}, initVecInclination {90}; // Initial values
      //      extern VecPolar vector;
      constexpr float vecScale {0.7};
      constexpr float magScale {0.1};
      constexpr float rotateScale {0.3};
      constexpr bool normalize {false};
      /* These variables affect how many line segments are computed for a trajectory. We first compute more coarsely.
	 and then move to smaller lines segments */
      constexpr int trajectoryScaleFactor {384};
      constexpr int _2ndTrajectoryScaleFactor {175}; // We don't think this work's as intended.
      extern std::vector<float> trajectory;
    }
  }
}


#endif
/**
	@mainpage SOIL

	Jonathan Dummer
	2007-07-26-10.36

	Simple OpenGL Image Library

	A tiny c library for uploading images as
	textures into OpenGL.  Also saving and
	loading of images is supported.

	I'm using Sean's Tool Box image loader as a base:
	http://www.nothings.org/

	I'm upgrading it to load TGA and DDS files, and a direct
	path for loading DDS files straight into OpenGL textures,
	when applicable.

	Image Formats:
	- BMP		load & save
	- TGA		load & save
	- DDS		load & save
	- PNG		load
	- JPG		load

	OpenGL Texture Features:
	- resample to power-of-two sizes
	- MIPmap generation
	- compressed texture S3TC formats (if supported)
	- can pre-multiply alpha for you, for better compositing
	- can flip image about the y-axis (except pre-compressed DDS files)

	Thanks to:
	* Sean Barret - for the awesome stb_image
	* Dan Venkitachalam - for finding some non-compliant DDS files, and patching some explicit casts
	* everybody at gamedev.net
**/

#ifndef HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY
#define HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY

#ifdef __cplusplus
extern "C" {
#endif

/**
	The format of images that may be loaded (force_channels).
	SOIL_LOAD_AUTO leaves the image in whatever format it was found.
	SOIL_LOAD_L forces the image to load as Luminous (greyscale)
	SOIL_LOAD_LA forces the image to load as Luminous with Alpha
	SOIL_LOAD_RGB forces the image to load as Red Green Blue
	SOIL_LOAD_RGBA forces the image to load as Red Green Blue Alpha
**/
enum
{
	SOIL_LOAD_AUTO = 0,
	SOIL_LOAD_L = 1,
	SOIL_LOAD_LA = 2,
	SOIL_LOAD_RGB = 3,
	SOIL_LOAD_RGBA = 4
};

/**
	Passed in as reuse_texture_ID, will cause SOIL to
	register a new texture ID using glGenTextures().
	If the value passed into reuse_texture_ID > 0 then
	SOIL will just re-use that texture ID (great for
	reloading image assets in-game!)
**/
enum
{
	SOIL_CREATE_NEW_ID = 0
};

/**
	flags you can pass into SOIL_load_OGL_texture()
	and SOIL_create_OGL_texture().
	(note that if SOIL_FLAG_DDS_LOAD_DIRECT is used
	the rest of the flags with the exception of
	SOIL_FLAG_TEXTURE_REPEATS will be ignored while
	loading already-compressed DDS files.)

	SOIL_FLAG_POWER_OF_TWO: force the image to be POT
	SOIL_FLAG_MIPMAPS: generate mipmaps for the texture
	SOIL_FLAG_TEXTURE_REPEATS: otherwise will clamp
	SOIL_FLAG_MULTIPLY_ALPHA: for using (GL_ONE,GL_ONE_MINUS_SRC_ALPHA) blending
	SOIL_FLAG_INVERT_Y: flip the image vertically
	SOIL_FLAG_COMPRESS_TO_DXT: if the card can display them, will convert RGB to DXT1, RGBA to DXT5
	SOIL_FLAG_DDS_LOAD_DIRECT: will load DDS files directly without _ANY_ additional processing
	SOIL_FLAG_NTSC_SAFE_RGB: clamps RGB components to the range [16,235]
	SOIL_FLAG_CoCg_Y: Google YCoCg; RGB=>CoYCg, RGBA=>CoCgAY
	SOIL_FLAG_TEXTURE_RECTANGE: uses ARB_texture_rectangle ; pixel indexed & no repeat or MIPmaps or cubemaps
**/
enum
{
	SOIL_FLAG_POWER_OF_TWO = 1,
	SOIL_FLAG_MIPMAPS = 2,
	SOIL_FLAG_TEXTURE_REPEATS = 4,
	SOIL_FLAG_MULTIPLY_ALPHA = 8,
	SOIL_FLAG_INVERT_Y = 16,
	SOIL_FLAG_COMPRESS_TO_DXT = 32,
	SOIL_FLAG_DDS_LOAD_DIRECT = 64,
	SOIL_FLAG_NTSC_SAFE_RGB = 128,
	SOIL_FLAG_CoCg_Y = 256,
	SOIL_FLAG_TEXTURE_RECTANGLE = 512
};

/**
	The types of images that may be saved.
	(TGA supports uncompressed RGB / RGBA)
	(BMP supports uncompressed RGB)
	(DDS supports DXT1 and DXT5)
**/
enum
{
	SOIL_SAVE_TYPE_TGA = 0,
	SOIL_SAVE_TYPE_BMP = 1,
	SOIL_SAVE_TYPE_DDS = 2
};

/**
	Defines the order of faces in a DDS cubemap.
	I recommend that you use the same order in single
	image cubemap files, so they will be interchangeable
	with DDS cubemaps when using SOIL.
**/
#define SOIL_DDS_CUBEMAP_FACE_ORDER "EWUDNS"

/**
	The types of internal fake HDR representations

	SOIL_HDR_RGBE:		RGB * pow( 2.0, A - 128.0 )
	SOIL_HDR_RGBdivA:	RGB / A
	SOIL_HDR_RGBdivA2:	RGB / (A*A)
**/
enum
{
	SOIL_HDR_RGBE = 0,
	SOIL_HDR_RGBdivA = 1,
	SOIL_HDR_RGBdivA2 = 2
};

/**
	Loads an image from disk into an OpenGL texture.
	\param filename the name of the file to upload as a texture
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_texture
	(
		const char *filename,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 6 images from disk into an OpenGL cubemap texture.
	\param x_pos_file the name of the file to upload as the +x cube face
	\param x_neg_file the name of the file to upload as the -x cube face
	\param y_pos_file the name of the file to upload as the +y cube face
	\param y_neg_file the name of the file to upload as the -y cube face
	\param z_pos_file the name of the file to upload as the +z cube face
	\param z_neg_file the name of the file to upload as the -z cube face
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_cubemap
	(
		const char *x_pos_file,
		const char *x_neg_file,
		const char *y_pos_file,
		const char *y_neg_file,
		const char *z_pos_file,
		const char *z_neg_file,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 1 image from disk and splits it into an OpenGL cubemap texture.
	\param filename the name of the file to upload as a texture
	\param face_order the order of the faces in the file, any combination of NSWEUD, for North, South, Up, etc.
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_single_cubemap
	(
		const char *filename,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads an HDR image from disk into an OpenGL texture.
	\param filename the name of the file to upload as a texture
	\param fake_HDR_format SOIL_HDR_RGBE, SOIL_HDR_RGBdivA, SOIL_HDR_RGBdivA2
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_HDR_texture
	(
		const char *filename,
		int fake_HDR_format,
		int rescale_to_max,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads an image from RAM into an OpenGL texture.
	\param buffer the image data in RAM just as if it were still in a file
	\param buffer_length the size of the buffer in bytes
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_texture_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 6 images from memory into an OpenGL cubemap texture.
	\param x_pos_buffer the image data in RAM to upload as the +x cube face
	\param x_pos_buffer_length the size of the above buffer
	\param x_neg_buffer the image data in RAM to upload as the +x cube face
	\param x_neg_buffer_length the size of the above buffer
	\param y_pos_buffer the image data in RAM to upload as the +x cube face
	\param y_pos_buffer_length the size of the above buffer
	\param y_neg_buffer the image data in RAM to upload as the +x cube face
	\param y_neg_buffer_length the size of the above buffer
	\param z_pos_buffer the image data in RAM to upload as the +x cube face
	\param z_pos_buffer_length the size of the above buffer
	\param z_neg_buffer the image data in RAM to upload as the +x cube face
	\param z_neg_buffer_length the size of the above buffer
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_cubemap_from_memory
	(
		const unsigned char *const x_pos_buffer,
		int x_pos_buffer_length,
		const unsigned char *const x_neg_buffer,
		int x_neg_buffer_length,
		const unsigned char *const y_pos_buffer,
		int y_pos_buffer_length,
		const unsigned char *const y_neg_buffer,
		int y_neg_buffer_length,
		const unsigned char *const z_pos_buffer,
		int z_pos_buffer_length,
		const unsigned char *const z_neg_buffer,
		int z_neg_buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Loads 1 image from RAM and splits it into an OpenGL cubemap texture.
	\param buffer the image data in RAM just as if it were still in a file
	\param buffer_length the size of the buffer in bytes
	\param face_order the order of the faces in the file, any combination of NSWEUD, for North, South, Up, etc.
	\param force_channels 0-image format, 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_load_OGL_single_cubemap_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Creates a 2D OpenGL texture from raw image data.  Note that the raw data is
	_NOT_ freed after the upload (so the user can load various versions).
	\param data the raw data to be uploaded as an OpenGL texture
	\param width the width of the image in pixels
	\param height the height of the image in pixels
	\param channels the number of channels: 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_create_OGL_texture
	(
		const unsigned char *const data,
		int width, int height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Creates an OpenGL cubemap texture by splitting up 1 image into 6 parts.
	\param data the raw data to be uploaded as an OpenGL texture
	\param width the width of the image in pixels
	\param height the height of the image in pixels
	\param channels the number of channels: 1-luminous, 2-luminous/alpha, 3-RGB, 4-RGBA
	\param face_order the order of the faces in the file, and combination of NSWEUD, for North, South, Up, etc.
	\param reuse_texture_ID 0-generate a new texture ID, otherwise reuse the texture ID (overwriting the old texture)
	\param flags can be any of SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_DDS_LOAD_DIRECT
	\return 0-failed, otherwise returns the OpenGL texture handle
**/
unsigned int
	SOIL_create_OGL_single_cubemap
	(
		const unsigned char *const data,
		int width, int height, int channels,
		const char face_order[6],
		unsigned int reuse_texture_ID,
		unsigned int flags
	);

/**
	Captures the OpenGL window (RGB) and saves it to disk
	\return 0 if it failed, otherwise returns 1
**/
int
	SOIL_save_screenshot
	(
		const char *filename,
		int image_type,
		int x, int y,
		int width, int height
	);

/**
	Loads an image from disk into an array of unsigned chars.
	Note that *channels return the original channel count of the
	image.  If force_channels was other than SOIL_LOAD_AUTO,
	the resulting image has force_channels, but *channels may be
	different (if the original image had a different channel
	count).
	\return 0 if failed, otherwise returns 1
**/
unsigned char*
	SOIL_load_image
	(
		const char *filename,
		int *width, int *height, int *channels,
		int force_channels
	);

/**
	Loads an image from memory into an array of unsigned chars.
	Note that *channels return the original channel count of the
	image.  If force_channels was other than SOIL_LOAD_AUTO,
	the resulting image has force_channels, but *channels may be
	different (if the original image had a different channel
	count).
	\return 0 if failed, otherwise returns 1
**/
unsigned char*
	SOIL_load_image_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int *width, int *height, int *channels,
		int force_channels
	);

/**
	Saves an image from an array of unsigned chars (RGBA) to disk
	\return 0 if failed, otherwise returns 1
**/
int
	SOIL_save_image
	(
		const char *filename,
		int image_type,
		int width, int height, int channels,
		const unsigned char *const data
	);

/**
	Frees the image data (note, this is just C's "free()"...this function is
	present mostly so C++ programmers don't forget to use "free()" and call
	"delete []" instead [8^)
**/
void
	SOIL_free_image_data
	(
		unsigned char *img_data
	);

/**
	This function resturn a pointer to a string describing the last thing
	that happened inside SOIL.  It can be used to determine why an image
	failed to load.
**/
const char*
	SOIL_last_result
	(
		void
	);


#ifdef __cplusplus
}
#endif

#endif /* HEADER_SIMPLE_OPENGL_IMAGE_LIBRARY	*/
