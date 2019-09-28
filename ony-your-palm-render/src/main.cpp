#include "ofMain.h"
#include "ofApp.h"


// main
//--------------------------------------------------------------
int main()
{
  // setup the GL context
  ofSetupOpenGL( 1920, 1080, OF_WINDOW );
  
  // this kicks off the running of my app
  // can be OF_WINDOW or OF_FULLSCREEN
  // pass in width and height too:
  ofRunApp( new ofApp() );
}
