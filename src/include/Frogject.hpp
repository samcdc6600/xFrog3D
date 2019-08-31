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
