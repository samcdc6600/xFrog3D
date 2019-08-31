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
