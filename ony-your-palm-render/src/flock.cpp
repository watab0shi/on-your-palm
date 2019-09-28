#include "flock.h"


// Flock
//--------------------------------------------------------------
Flock::Flock()
{
  texture.load( "images/bloom.png" );
  texture.allocate( 250, 250, OF_IMAGE_COLOR_ALPHA );
}

// ~Flock
//--------------------------------------------------------------
Flock::~Flock()
{
}

// addBoid
//--------------------------------------------------------------
void Flock::addBoid( Boid* _b )
{
  boids.push_back( _b );
}

// removeBoid
//--------------------------------------------------------------
void Flock::removeBoid()
{
  if( boids.size() > 0 )
  {
    boids.erase( boids.end() - 1 );
  }
}

// addAttractPoint
//--------------------------------------------------------------
void Flock::addAttractPoint( EffectPoint _point )
{
  attractPoints.push_back( _point );
}

// removeAttractPoint
//--------------------------------------------------------------
void Flock::removeAttractPoint()
{
  if( attractPoints.size() > 0 )
  {
    attractPoints.erase( attractPoints.end() - 1 );
  }
}

// clearAttractPoints
//--------------------------------------------------------------
void Flock::clearAttractPoints()
{
  attractPoints.clear();
}

// addRepelPoint
//--------------------------------------------------------------
void Flock::addRepelPoint( EffectPoint _point )
{
  repelPoints.push_back( _point );
}

// removeRepelPoint
//--------------------------------------------------------------
void Flock::removeRepelPoint()
{
  if( repelPoints.size() > 0 )
  {
    repelPoints.erase( repelPoints.end() - 1 );
  }
}

// clearRepelPoints
//--------------------------------------------------------------
void Flock::clearRepelPoints()
{
  repelPoints.clear();
}

// clear
//--------------------------------------------------------------
void Flock::clear()
{
  boids.clear();
}

// setMaxSpeed
//--------------------------------------------------------------
void Flock::setMaxForce( float _maxForce )
{
  for( Boid* b : boids )
  {
    b->setMaxForce( _maxForce );
  }
}

// setMaxSpeed
//--------------------------------------------------------------
void Flock::setMaxSpeed( float _maxSpeed )
{
  for( Boid* b : boids )
  {
    b->setMaxSpeed( _maxSpeed );
  }
}

// setSeparateRange
//--------------------------------------------------------------
void Flock::setSeparateRange( float _separate )
{
  for( Boid* b : boids )
  {
    b->setSeparateRange( _separate );
  }
}

// setSeparateRange
//--------------------------------------------------------------
void Flock::setAlignRange( float _align )
{
  for( Boid* b : boids )
  {
    b->setAlignRange( _align );
  }
}

// setSeparateRange
//--------------------------------------------------------------
void Flock::setCohesionRange( float _cohesion )
{
  for( Boid* b : boids )
  {
    b->setCohesionRange( _cohesion );
  }
}

// setSeparateRange
//--------------------------------------------------------------
int Flock::getNumBoids()
{
  return boids.size();
}

// update
//--------------------------------------------------------------
void Flock::update()
{
  // set identifier
  for( Boid* b : boids )
  {
    b->setIdentifier( labels, pls, scaledPls );
  }
  
  // clear repel points
  clearAttractPoints();
  // clear repel points
  clearRepelPoints();
  
  // add repel points
  for( int i = 0; i < boids.size(); ++i )
  {
    boids.at( i )->setColor( ofColor( 255 ) );
    
    // repel
    if( boids.at( i )->getRadius() == 20 && i % 60 == 0 )
    {
      addRepelPoint( EffectPoint( boids.at( i )->getPosition(), 400 ) );
      boids.at( i )->setColor( ofColor::fromHsb( 0, 110, 255 ) );
    }
    
    // attract
    else if( boids.at( i )->getRadius() == 4 && i % 61 == 0 )
    {
      addAttractPoint( EffectPoint( boids.at( i )->getPosition(), 600 ) );
      addRepelPoint( EffectPoint( boids.at( i )->getPosition(), 120 ) );
      boids.at( i )->setMaxSpeed( 5 );
      boids.at( i )->setColor( ofColor::fromHsb( 90, 110, 255 ) );
    }
    
    // weak
    else if( boids.at( i )->getRadius() < 8 && i % 62 == 0 )
    {
      boids.at( i )->setAlignRange( 10 );
      boids.at( i )->setCohesionRange( 20 );
      boids.at( i )->setMaxForce( 0.01 );
      boids.at( i )->setColor( ofColor::fromHsb( 160, 110, 255 ) );
    }
  }
  
  // set identifier of effect points
  for( int i = 0; i < attractPoints.size(); ++i )
  {
    attractPoints.at( i ).identifier = -1;
    
    for( int j = 0; j < pls->size(); ++j )
    {
      if( pls->at( j ).inside( attractPoints.at( i ).pos ) )
      {
        attractPoints.at( i ).identifier = labels->at( j );
      }
    }
  }
  
  for( int i = 0; i < repelPoints.size(); ++i )
  {
    repelPoints.at( i ).identifier = -1;
    
    for( int j = 0; j < pls->size(); ++j )
    {
      if( pls->at( j ).inside( repelPoints.at( i ).pos ) )
      {
        repelPoints.at( i ).identifier = labels->at( j );
      }
    }
  }
  
  // update
  for( Boid* b : boids )
  {
    b->update( &boids, &attractPoints, &repelPoints );
  }
}

// debugDraw
//--------------------------------------------------------------
void Flock::debugDraw()
{
  float r = 0.0;
  ofDisableDepthTest();
  ofSetLineWidth( 1 );
  
  // draw attract points
  for( int i = 0; i < attractPoints.size(); ++i )
  {
    r = attractPoints.at( i ).radius;
    
    ofEnableBlendMode( OF_BLENDMODE_ADD );
    ofSetColor( ofColor::fromHsb( 0, 200, 255, 90 ) );
    ofPushMatrix();
    {
      ofTranslate( attractPoints.at( i ).pos );
      texture.draw( -r, -r, r*2, r*2 );
    }
    ofPopMatrix();
    ofEnableAlphaBlending();
    
    ofSetColor( ofColor::fromHsb( 0, 200, 255 ) );
    
    ofFill();
    ofCircle( attractPoints.at( i ).pos, 8 );
    
    ofNoFill();
    ofCircle( attractPoints.at( i ).pos, 16 );
    
    // ofDrawBitmapStringHighlight( ofToString( attractPoints.at( i ).identifier ), attractPoints.at( i ).pos );
  }
  
  // draw attract points
  for( int i = 0; i < repelPoints.size(); ++i )
  {
    r = repelPoints.at( i ).radius;
    
    ofEnableBlendMode( OF_BLENDMODE_ADD );
    ofSetColor( ofColor::fromHsb( 130, 200, 255, 64 ) );
    ofPushMatrix();
    {
      ofTranslate( repelPoints.at( i ).pos );
      texture.draw( -r, -r, r*2, r*2 );
    }
    ofPopMatrix();
    ofEnableAlphaBlending();
    
    ofSetColor( ofColor::fromHsb( 130, 200, 255 ) );
    
    ofFill();
    ofCircle( repelPoints.at( i ).pos, 8 );
    
    ofNoFill();
    ofCircle( repelPoints.at( i ).pos, 16 );
    
    // ofDrawBitmapStringHighlight( ofToString( repelPoints.at( i ).identifier ), repelPoints.at( i ).pos );
  }
  
  // draw boids
  // draw();
}

// draw
//--------------------------------------------------------------
void Flock::draw( int _identifier )
{
  for( Boid* b : boids )
  {
    if( b->getIdentifier() == _identifier )
    {
      b->draw();
    }
  }
}

void Flock::draw()
{
  //        ofEnableDepthTest();
  //        {
  for( Boid* b : boids )
  {
    b->draw();
  }
  //        }
  //        ofDisableDepthTest();
}
