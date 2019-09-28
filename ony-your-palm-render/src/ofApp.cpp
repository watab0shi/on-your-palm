#include "ofApp.h"


// setup
//--------------------------------------------------------------
void ofApp::setup()
{
  // basic variables
  w = ofGetWidth();
  h = ofGetHeight();
  halfWidth  = ( float )w / 2.;
  halfHeight = ( float )h / 2.;
  frameRate = 60;
  
  // second window
  int subW = 1920 * 2;
  int subH = 1080 * 2;
//  window2.setup( "", 0, 0, 1920, 1080, true );
//  window2.setup( "", ofGetScreenWidth(), 0, subW, subH, true );
  
  // fbo
  fbo.allocate( subW, subH, GL_RGB );
  fbo.begin();
  {
    ofClear( 0, 0, 0, 255 );
  }
  fbo.end();
  
  // syphon
  individualTextureSyphonServer.setName( "Texture Output" );
  
  // osc
  oscReceiver.setup( OSC_PORT );
  
  // flock
  for( int i = 0; i < NUM_BOIDS; ++i )
  {
    flock.addBoid( new Boid( ofRandomWidth(), ofRandomHeight() ) );
  }
  
  separate = 50.;
  align    = 100.;
  cohesion = 100.;
  maxForce = .3;
  maxSpeed = 2.;
  
  flock.labels    = &labels;
  flock.pls       = &pls;
  flock.scaledPls = &scaledPls;
  
  // gui
  setupGUI();
  
  gui->toggleVisible();
  ofHideCursor();
  
  // application settings
  ofSetVerticalSync( true );
  ofEnableAntiAliasing();
  ofEnableSmoothing();
  ofSetFrameRate( frameRate );
  ofBackground( 0 );
  
  ofToggleFullscreen();
}

// update
//--------------------------------------------------------------
void ofApp::update()
{
  ofSetWindowTitle( ofToString( ofGetFrameRate(), 2 ) );
  
  // update contours
  while( oscReceiver.hasWaitingMessages() )
  {
    ofxOscMessage msg;
    oscReceiver.getNextMessage( msg );
    
    if( msg.getAddress() == "/boids/contours/size" )
    {
      int size = msg.getArgAsInt32( 0 );
      
      while( pls.size() > size )
      {
        pls.pop_back();
      }
      
      while( labels.size() > size )
      {
        labels.pop_back();
      }
    }
    else if( msg.getAddress() == "/boids/contours/points" )
    {
      int idx    = msg.getArgAsInt32( 0 );
      int label  = msg.getArgAsInt32( 1 );
      string str = msg.getArgAsString( 2 );
      
      vector< string > ptsStr = ofSplitString( str, "|" );
      
      ofPolyline pl;
      
      for( int i = ( ptsStr.size() - 1 ); i >= 0; --i )
      {
        vector< string > pointStr = ofSplitString( ptsStr.at( i ), "," );
        
        if( pointStr.size() == 2 )
        {
          float x = ofToFloat( pointStr.at( 0 ) );
          float y = ofToFloat( pointStr.at( 1 ) );
          
          x *= w;
          y *= h;
          
          pl.addVertex( x, y );
        }
      }
      pl.setClosed( true );
      
      if( idx < pls.size() )
      {
        pls.at( idx ) = pl;
      }
      else
      {
        pls.push_back( pl );
      }
      
      if( idx < labels.size() )
      {
        labels.at( idx ) = label;
      }
      else
      {
        labels.push_back( label );
      }
    }
  }
  
  // scaled contours
  scaledPls.clear();
  float scale = 1.5;
  for( int i = 0; i < pls.size(); ++i )
  {
    ofPolyline pl    = pls.at( i );
    ofPoint centroid = pl.getCentroid2D();
    
    scaledPls.push_back( ofPolyline() );
    for( int j = 0; j < pl.getVertices().size(); ++j )
    {
      ofVec3f n = pl.getNormalAtIndex( j );
      scaledPls.back().addVertex( pl.getVertices().at( j ) + n * 150 );
    }
    scaledPls.back().close();
  }
  
  // update boids
  flock.update();
  numBoids = flock.getNumBoids();
  
  // draw boids in fbo
  fbo.begin();
  {
    ofBackground( 0 );
    
    ofPushMatrix();
    {
//      ofScale( .5, .5 );
      
      for( auto& l : labels ) flock.draw( l );
      
      // draw contours
      if( gui->isVisible() )
      {
        ofSetColor( 255 );
        for( auto& pl : pls ) pl.draw();
      }
    }
    ofPopMatrix();
  }
  fbo.end();
}

// draw
//--------------------------------------------------------------
void ofApp::draw()
{
  ofBackgroundGradient( ofColor::fromHsb( 100, 200, 120 ), ofColor::fromHsb( 160, 200, 45 ) );
  
  ofSetColor( 255 );
  
  // draw boids in first window
  if( gui->isVisible() )
  {
    flock.debugDraw();
  }
  flock.draw( -1 );
  
  // debug draw
  if( gui->isVisible() )
  {
    // draw contours
    ofSetColor( 255 );
    for( auto& pl : pls ) pl.draw();
    
    ofSetColor( 255 );
    for( auto& spl : scaledPls ) spl.draw();
    
    ofSetColor( 255 );
    fbo.draw( 0, h - fbo.getHeight() / 4, fbo.getWidth() / 4, fbo.getHeight() / 4 );
    
    ofNoFill();
    ofDrawRectangle( 0, h - fbo.getHeight() / 4, fbo.getWidth() / 4, fbo.getHeight() / 4 );
  }
  
  // draw boids in second window
//  window2.begin();
//  {
//    ofSetColor( 255 );
//    fbo.draw( 0, 0, window2.getWidth(), window2.getHeight() );
//  }
//  window2.end();
  
  // publish fbo as texture
  individualTextureSyphonServer.publishTexture( &fbo.getTexture() );
}

// keyPressed
//--------------------------------------------------------------
void ofApp::keyPressed( int key )
{
  // flock
  // remove a boid
  if( key == '-' )
  {
    flock.removeBoid();
  }
  
  // add a boid
  if( key == '=' )
  {
    Boid* b = new Boid( ofGetMouseX(), ofGetMouseY() );
    
    b->setMaxForce( maxForce );
    b->setMaxSpeed( maxSpeed );
    b->setSeparateRange( separate );
    b->setAlignRange( align );
    b->setCohesionRange( cohesion );
    
    flock.addBoid( b );
  }
  
  // add attract point
  if( key == 's' )
  {
    flock.addAttractPoint( EffectPoint( ofVec3f( ofGetMouseX(), ofGetMouseY(), 0 ), 500 ) );
  }
  
  // remove attract point
  if( key == 'a' )
  {
    flock.removeAttractPoint();
  }
  
  // add repel point
  if( key == 'w' )
  {
    flock.addRepelPoint( EffectPoint( ofVec3f( ofGetMouseX(), ofGetMouseY(), 0 ), 200 ) );
  }
  
  // remove repel point
  if( key == 'q' )
  {
    flock.removeRepelPoint();
  }
  
  // reset
  if( key == 'h' )
  {
    flock.clear();
    
    for( int i = 0; i < NUM_BOIDS; ++i )
    {
      flock.addBoid( new Boid( halfWidth, halfHeight ) );
    }
    
    flock.setSeparateRange( separate );
    flock.setAlignRange( align );
    flock.setCohesionRange( cohesion );
    flock.setMaxForce( maxForce );
    flock.setMaxSpeed( maxSpeed );
  }
  
  // reset
  if( key == 'r' )
  {
    flock.clear();
    
    for( int i = 0; i < NUM_BOIDS; ++i )
    {
      flock.addBoid( new Boid( ofRandomWidth(), ofRandomHeight() ) );
    }
    
    flock.setSeparateRange( separate );
    flock.setAlignRange( align );
    flock.setCohesionRange( cohesion );
    flock.setMaxForce( maxForce );
    flock.setMaxSpeed( maxSpeed );
  }
  
  // gui
  if( key == ' ' )
  {
    gui->toggleVisible();
    
    ( gui->isVisible() ) ? ofShowCursor(): ofHideCursor();
  }
  
  // fullscreen
  if( key == 'f' )
  {
    ofToggleFullscreen();
  }
}

// keyReleased
//--------------------------------------------------------------
void ofApp::keyReleased( int key )
{
  
}

// mouseMoved
//--------------------------------------------------------------
void ofApp::mouseMoved( int x, int y )
{
  
}

// mouseDragged
//--------------------------------------------------------------
void ofApp::mouseDragged( int x, int y, int button )
{
  
}

// mousePressed
//--------------------------------------------------------------
void ofApp::mousePressed( int x, int y, int button )
{
  
}

// mouseReleased
//--------------------------------------------------------------
void ofApp::mouseReleased( int x, int y, int button )
{
  
}

// windowResized
//--------------------------------------------------------------
void ofApp::windowResized( int _w, int _h )
{
  w = _w;
  h = _h;
  
  halfWidth  = ( float )w / 2.;
  halfHeight = ( float )h / 2.;
}

// exit
//--------------------------------------------------------------
void ofApp::exit()
{
  gui->saveSettings( "GUI/guiSettings.xml" );
  
  delete gui;
}

// setupGUI
//--------------------------------------------------------------
void ofApp::setupGUI()
{
  float xInit  = OFX_UI_GLOBAL_WIDGET_SPACING;
  float length = 255 - xInit;
  int   dim    = 16;
  
  gui = new ofxUICanvas( 10, 10, length, h );
  
  // title
  gui->addLabel( "BoidSimulation", OFX_UI_FONT_LARGE );
  
  // fps
  gui->addSpacer( length - xInit, 2 );
  gui->addWidgetDown( new ofxUILabel( "FPS", OFX_UI_FONT_MEDIUM ) );
  
  gui->addFPSSlider( "FPS", length - xInit, dim );
  
  // flock
  gui->addSpacer( length - xInit, 2 );
  gui->addWidgetDown( new ofxUILabel( "Flock", OFX_UI_FONT_MEDIUM ) );
  
  gui->addWidgetDown( new ofxUISlider( "NumBoids", 0, 1000, &numBoids, ( length - xInit ), dim ) );
  
  gui->addWidgetDown( new ofxUISlider( "Separate", 0., 200., &separate, ( length - xInit ), dim ) );
  gui->addWidgetDown( new ofxUISlider( "Align", 0., 400., &align, ( length - xInit ), dim ) );
  gui->addWidgetDown( new ofxUISlider( "Cohesion", 0., 400., &cohesion, ( length - xInit ), dim ) );
  gui->addWidgetDown( new ofxUISlider( "MaxForce", 0.1, 0.6, &maxForce, ( length - xInit ), dim ) );
  gui->addWidgetDown( new ofxUISlider( "MaxSpeed", 1., 40., &maxSpeed, ( length - xInit ), dim ) );
  
  gui->autoSizeToFitWidgets();
  gui->setDrawWidgetPadding( true );
  ofAddListener( gui->newGUIEvent, this, &ofApp::guiEvent );
  gui->loadSettings( "GUI/guiSettings.xml" );
}

// guiEvent
//--------------------------------------------------------------
void ofApp::guiEvent( ofxUIEventArgs &e )
{
  string name = e.getName();
  
  if( name == "Separate" )
  {
    flock.setSeparateRange( separate );
  }
  else if( name == "Align" )
  {
    flock.setAlignRange( align );
  }
  else if( name == "Cohesion" )
  {
    flock.setCohesionRange( cohesion );
  }
  else if( name == "MaxForce" )
  {
    flock.setMaxForce( maxForce );
  }
  else if( name == "MaxSpeed" )
  {
    flock.setMaxSpeed( maxSpeed );
  }
}
