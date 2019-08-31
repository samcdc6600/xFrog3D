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
