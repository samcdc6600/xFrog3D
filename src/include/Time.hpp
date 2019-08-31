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
