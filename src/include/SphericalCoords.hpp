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
