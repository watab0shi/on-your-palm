#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxSyphon.h"
//#include "ofxSecondWindow.h"
#include "ofxOsc.h"
#include "flock.h"

#define OSC_PORT 8000
#define NUM_BOIDS 600


// ofApp
//--------------------------------------------------------------------------------
class ofApp : public ofBaseApp
{
public:
  void setup();
  void update();
  void draw();
  void exit();
  
  void keyPressed( int key );
  void keyReleased( int key );
  void mouseMoved( int x, int y );
  void mouseDragged( int x, int y, int button );
  void mousePressed( int x, int y, int button );
  void mouseReleased( int x, int y, int button );
  void windowResized( int w, int h );
  
  int   w, h;
  float halfWidth, halfHeight;
  int   frameRate;
  
//  ofxSecondWindow window2;
  
  // osc
  ofxOscReceiver oscReceiver;
  
  // contour
  vector< int >        labels;
  vector< ofPolyline > pls;
  vector< ofPolyline > scaledPls;
  
  // flock
  Flock flock;
  float numBoids;
  float maxForce;
  float maxSpeed;
  float separate;
  float align;
  float cohesion;
  
  // fbo
  ofFbo fbo;
  
  // syphon
  ofxSyphonServer individualTextureSyphonServer;
  
  // gui
  ofxUICanvas* gui;
  void         setupGUI();
  void         guiEvent( ofxUIEventArgs &e );
};
