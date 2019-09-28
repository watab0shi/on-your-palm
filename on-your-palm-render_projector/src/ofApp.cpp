#include "ofApp.h"

namespace
{
  const std::string warpSettingFilePath = "warpConfig.xml";
}

//--------------------------------------------------------------
void ofApp::setup()
{
  mClient.setup();
  mClient.set( "", "ony-your-palm-render" );
  
  warper.setup( 0, 0, 3840, 2160 );
  
  ofFile file( warpSettingFilePath );
  if( file.exists() )
  {
    warper.load( warpSettingFilePath );
  }
  warper.deactivate();
  
  ofSetWindowPosition( 3840 + 100, 0 );
  ofToggleFullscreen();
  
  ofSetVerticalSync( true );
  ofSetFrameRate( 30 );
  ofBackground( 0 );
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
  warper.begin();
  {
    ofSetColor( 255 );
    mClient.draw( 0, 0 );
  }
  warper.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed( int key )
{
  if( key == ' ' )
  {
    warper.toggleActive();
  }
  
  if( key == 'f' )
  {
    ofToggleFullscreen();
  }
  
  if( key == 's' )
  {
    warper.save( warpSettingFilePath );
  }
  if( key == 'l' )
  {
    warper.load( warpSettingFilePath );
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::exit()
{
  warper.save( warpSettingFilePath );
}
