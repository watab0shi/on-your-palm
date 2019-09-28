#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxOsc.h"


#define OSC_HOST "loaclhost"
#define OSC_PORT 8000


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ofApp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    
    
    //---------------------------------------- basic variables
    int w, h;
    float halfWidth, halfHeight;
    
    int frameRate;
    
    
    //---------------------------------------- kinect
    ofxKinect kinect;
    int       kw, kh;
    float     nearThreshold, farThreshold;
    
    ofPixels getBinarizedPixels( float _near, float _far );
    
    
    //---------------------------------------- openCv
    ofxCvGrayscaleImage grayImg;
    ofxCvGrayscaleImage unwarpedImg;
    
    int imgW, imgH;
    
    vector< cv::Point > cvQuad;
    int                 selectCorner;
    
    ofxCv::ContourFinder finder;
    float minArea, maxArea;
    float persistence;
    float maxDistance;
    
    float step;
    bool bFullScreen;
    
    
    //---------------------------------------- osc
    ofxOscSender oscSender;
    
    
    //---------------------------------------- gui
    ofxUICanvas *gui;
    
    void setupGUI();
    
    void guiEvent( ofxUIEventArgs &e );
};
