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
