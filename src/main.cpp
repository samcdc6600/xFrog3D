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
  drawRiver();
  drawLogs();
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
