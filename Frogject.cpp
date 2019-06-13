#include "Frogject.hpp"
#include "normals.hpp"
#include "Time.hpp"



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
