#include "ofApp.h"

using namespace ofxCv;
using namespace cv;



////////////////////////////////////////
// setup
////////////////////////////////////////
void ofApp::setup()
{
    //---------------------------------------- basic variables
    w = ofGetWidth();
    h = ofGetHeight();
    
    halfWidth  = ( float )w / 2.0f;
    halfHeight = ( float )h / 2.0f;
    
    frameRate = 30;
    
    
    //---------------------------------------- kinect
    ofSetLogLevel( OF_LOG_VERBOSE );
    
    // calibration RGB and Depth
    kinect.setRegistration( true );
    
    // init kinect
    kinect.init();
    
    // open device
    kinect.open();
//    kinect.open( "A00364901334053A" );
  
    // turn off LED
    kinect.setLed( ( ofxKinect::LedMode )LED_OFF );
    
    kw = kinect.width;
    kh = kinect.height;
    
    nearThreshold = 500.0;
    farThreshold  = 2000.0;
    
    ofSetLogLevel( OF_LOG_SILENT );
    
    
    //---------------------------------------- openCv
    grayImg.allocate( kw, kh );
    
    imgW = 640;
    imgH = 360;
    
    unwarpedImg.allocate( imgW, imgH );
    
    selectCorner = -1;
    
    cvQuad.resize( 4 );
    cvQuad.at( 0 ) = toCv( ofVec2f(  0,  0 ) );
    cvQuad.at( 1 ) = toCv( ofVec2f( kw,  0 ) );
    cvQuad.at( 2 ) = toCv( ofVec2f( kw, kh ) );
    cvQuad.at( 3 ) = toCv( ofVec2f(  0, kh ) );
  
    cvQuad.at( 0 ) = toCv( ofVec2f( 74, 87 ) );
    cvQuad.at( 1 ) = toCv( ofVec2f( 519, 82 ) );
    cvQuad.at( 2 ) = toCv( ofVec2f( 519, 332 ) );
    cvQuad.at( 3 ) = toCv( ofVec2f( 78, 335 ) );
    
    minArea  = 100.0;
    maxArea  = kw * kh;
    
    persistence = 15;
    maxDistance = 60;
    
    finder.setMinArea( minArea );
    finder.setMaxArea( maxArea );
    finder.setThreshold( 10 );
    finder.getTracker().setPersistence( persistence );
    finder.getTracker().setMaximumDistance( maxDistance );
    
    step        = 5;
    bFullScreen = false;
    
    
    //---------------------------------------- osc
    oscSender.setup( OSC_HOST, OSC_PORT );
    
    
    //---------------------------------------- gui
    setupGUI();
    
    
    //---------------------------------------- application settings
    ofSetVerticalSync( false );
    ofDisableAntiAliasing();
    
    ofSetFrameRate( frameRate );
    ofBackground( 0 );
}


////////////////////////////////////////
// setupGUI
////////////////////////////////////////
void ofApp::setupGUI()
{
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = 255 - xInit;
    
    int dim = 16;
    
    gui = new ofxUICanvas( 10, 10, length, h );
    
    //------------------------------------------------------------ title
    gui->addLabel( "Boids_sender", OFX_UI_FONT_LARGE );
    
    //------------------------------------------------------------ fps
    gui->addSpacer( length - xInit, 2 );
    gui->addWidgetDown( new ofxUILabel( "FPS", OFX_UI_FONT_MEDIUM ) );
    
    gui->addFPSSlider( "FPS", length - xInit, dim );
    
    //------------------------------------------------------------ kinect
    gui->addSpacer( length - xInit, 2 );
    gui->addWidgetDown( new ofxUILabel( "Kinect", OFX_UI_FONT_MEDIUM ) );
    
    gui->addWidgetDown( new ofxUIRangeSlider( "Threshold", 500.0, 4000.0, &nearThreshold, &farThreshold, ( length - xInit ), dim ) );
    
    //------------------------------------------------------------ openCV
    gui->addSpacer( length - xInit, 2 );
    gui->addWidgetDown( new ofxUILabel( "OpenCV", OFX_UI_FONT_MEDIUM ) );
    
    gui->addWidgetDown( new ofxUIRangeSlider( "Area", 100.0, ( kw * kh ), &minArea, &maxArea, ( length - xInit ), dim ) );
    
    gui->addWidgetDown( new ofxUISlider( "Persistence", 15.0, 90.0, &persistence, ( length - xInit ), dim ) );
    gui->addWidgetDown( new ofxUISlider( "MaxDistance", 30.0, 120.0, &maxDistance, ( length - xInit ), dim ) );
    
    gui->addWidgetDown( new ofxUISlider( "Step", 0, 20, &step, ( length - xInit ), dim ) );
    
    
    gui->autoSizeToFitWidgets();
    
    gui->setDrawWidgetPadding( true );
    
    ofAddListener( gui->newGUIEvent, this, &ofApp::guiEvent );
    
    gui->loadSettings( "GUI/guiSettings.xml" );
}


////////////////////////////////////////
// update
////////////////////////////////////////
void ofApp::update()
{
    ofSetWindowTitle( ofToString( ofGetFrameRate(), 2 ) );
    
    // update kinect
    kinect.update();
    
    if( kinect.isFrameNew() )
    {
        grayImg.setFromPixels( getBinarizedPixels( nearThreshold, farThreshold ) );
        
        vector< Point2f > wpts;
        copy( cvQuad.begin(), cvQuad.end(), back_inserter( wpts ) );
        unwarpPerspective( grayImg, unwarpedImg, wpts );
        unwarpedImg.flagImageChanged();
        
        finder.findContours( unwarpedImg );
    }
    
    step = ( int )step;
    
    // send size of contours
    ofxOscMessage sMsg;
    
    sMsg.setAddress( "/boids/contours/size" );
    sMsg.addIntArg( finder.size() );
    
    oscSender.sendMessage( sMsg );
    
    // send points of contours
    for( int i = 0; i < finder.size(); ++i )
    {
        vector< cv::Point > pts = finder.getContours().at( i );
        
        // osc message
        ofxOscMessage msg;
        
        msg.setAddress( "/boids/contours/points" );
        
        // add index
        msg.addIntArg( i );
        
        // add label
        msg.addIntArg( finder.getLabel( i ) );
        
        // add points
        string ptsStr = "";
        for( int j = 0; j < pts.size(); j += step )
        {
            // scale to 0 ~ 1
            ofPoint p = toOf( pts.at( j ) ) / ofPoint( imgW, imgH );
            
            ptsStr += ofToString( p.x, 5 );
            ptsStr += ",";
            ptsStr += ofToString( p.y, 5 );
            ptsStr += "|";
        }
        
        msg.addStringArg( ptsStr );
        
        oscSender.sendMessage( msg );
    }
}


////////////////////////////////////////
// getBinarizedPixels
////////////////////////////////////////
ofPixels ofApp::getBinarizedPixels( float _near, float _far )
{
    ofPixels pixels;
    pixels.allocate( kw, kh, 1 );
    
    float* distPixels = kinect.getDistancePixels().getData();
    
    for( int i = 0; i < ( kw * kh ); ++i )
    {
        if( ( distPixels[ i ] >= _near ) && ( distPixels[ i ] <= _far ) )
        {
            pixels.setColor( i, ofColor( 255 ) );
        }
        else
        {
            pixels.setColor( i, ofColor( 0 ) );
        }
    }
    
    return pixels;
}


////////////////////////////////////////
// draw
////////////////////////////////////////
void ofApp::draw()
{
    ofSetColor( 255 );
    kinect.draw( 0, 0, kw, kh );
    ofSetColor( 255, 30 );
    kinect.drawDepth( 0, 0, kw, kh );
    
    ofSetColor( 255 );
    unwarpedImg.draw( kw, 0, imgW, imgH );
    
    ofPushMatrix();
    {
        ofTranslate( kw, 0 );
        finder.draw();
    }
    ofPopMatrix();
    
    
    // draw cvQuad
    ofSetColor( ofColor::fromHsb( 130, 200, 255 ) );
    ofNoFill();
    ofBeginShape();
    {
        for( int i = 0; i < cvQuad.size(); ++i )
        {
            ofVertex( toOf( cvQuad.at( i ) ).x, toOf( cvQuad.at( i ) ).y );
        }
    }
    ofEndShape( true );
    
    if( selectCorner > -1 )
    {
        ofDrawCircle( toOf( cvQuad.at( selectCorner ) ), 8 );
    }
    
    
    // draw vertex
    ofSetColor( 255 );
    ofPushMatrix();
    {
        ofTranslate( kw, imgH );
        
        ofFill();
        for( int i = 0; i < finder.size(); ++i )
        {
            vector< cv::Point > pts = finder.getContour( i );
            
            ofBeginShape();
            {
                for( int j = 0; j < pts.size(); j += step )
                {
                    ofPoint p = toOf( pts.at( j ) );
                    
                    // draw as default vertex
                    ofVertex( p );
                }
            }
            ofEndShape( true );
            
            ofDrawBitmapStringHighlight( ofToString( finder.getLabel( i ) ), toOf( finder.getCenter( i ) ) );
        }
    }
    ofPopMatrix();
    
    
    // draw rect
    ofSetColor( 255 );
    ofNoFill();
    ofDrawRectangle( 0, 0, kw, kh );
    ofDrawRectangle( kw, 0, imgW, imgH );
    ofDrawRectangle( kw, imgH, imgW, imgH );
    
    
    // draw selecting corner index
    ofDrawBitmapStringHighlight( "selecting : " + ofToString( selectCorner ), 20, h - 20 );
}


////////////////////////////////////////
// keyPressed
////////////////////////////////////////
void ofApp::keyPressed( int key )
{
    //---------------------------------------- openCv
    if( key == OF_KEY_LEFT )
    {
        selectCorner = CLAMP( selectCorner - 1, -1, 3 );
    }
    if( key == OF_KEY_RIGHT )
    {
        selectCorner = CLAMP( selectCorner + 1, -1, 3 );
    }
    if( key == '0' )
    {
        selectCorner = -1;
    }
    
    
    //---------------------------------------- gui
    if( key == ' ' )
    {
        gui->toggleVisible();
    }
    
    
    //---------------------------------------- fullscreen
    if( key == 'f' )
    {
        ofToggleFullscreen();
        bFullScreen = !bFullScreen;
    }
}


////////////////////////////////////////
// keyReleased
////////////////////////////////////////
void ofApp::keyReleased( int key )
{
    
}


////////////////////////////////////////
// mouseMoved
////////////////////////////////////////
void ofApp::mouseMoved( int x, int y )
{
    
}


////////////////////////////////////////
// mouseDragged
////////////////////////////////////////
void ofApp::mouseDragged( int x, int y, int button )
{
    
}


////////////////////////////////////////
// mousePressed
////////////////////////////////////////
void ofApp::mousePressed( int x, int y, int button )
{
    if( !gui->isVisible() && ( selectCorner > -1 ) )
    {
        cvQuad.at( selectCorner ) = toCv( ofVec2f( CLAMP( x, 0, kw - 1 ), CLAMP( y, 0, kh - 1 ) ) );
    }
}


////////////////////////////////////////
// mouseReleased
////////////////////////////////////////
void ofApp::mouseReleased( int x, int y, int button )
{
    
}


////////////////////////////////////////
// windowResized
////////////////////////////////////////
void ofApp::windowResized( int _w, int _h )
{
    w = _w;
    h = _h;
    
    halfWidth  = ( float )w / 2.0f;
    halfHeight = ( float )h / 2.0f;
}


////////////////////////////////////////
// exit
////////////////////////////////////////
void ofApp::exit()
{
    gui->saveSettings( "GUI/guiSettings.xml" );
    
    delete gui;
    
    for( int i = 0; i < cvQuad.size(); ++i )
    {
        cout << "cvQuad.at( "
             << ofToString( i )
             << " ) = toCv( ofVec2f( "
             << ofToString( toOf( cvQuad.at( i ) ).x )
             << ", "
             << ofToString( toOf( cvQuad.at( i ) ).y )
             << " ) );"
             << endl;
    }
    
}






////////////////////////////////////////
// guiEvent
////////////////////////////////////////
void ofApp::guiEvent( ofxUIEventArgs &e )
{
    string name = e.getName();
    
    if( name == "Area" )
    {
        finder.setMinArea( minArea );
        finder.setMaxArea( maxArea );
    }
    else if( name == "Persistence" )
    {
        finder.getTracker().setPersistence( persistence );
    }
    else if( name == "MaxDistance" )
    {
        finder.getTracker().setMaximumDistance( maxDistance );
    }
}
