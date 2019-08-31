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
