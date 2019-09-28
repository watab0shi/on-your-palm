#include "boid.h"


// EffectPoint
//--------------------------------------------------------------
EffectPoint::EffectPoint()
{
}

EffectPoint::EffectPoint( ofVec3f _pos, float _radius )
: pos( _pos )
, radius( _radius )
{
  
}


// Boid
//--------------------------------------------------------------
Boid::Boid( float _x, float _y, float _z )
: identifier( -1 )
, vel( ofVec3f( 1, 0, 0 ).getRotatedRad( 0, 0, ofRandom( TWO_PI ) ) )
, pos( _x, _y, _z )
, radius( 8. )
, maxForce( .03 )
, maxSpeed( 2. )
, separateRange( 25. )
, alignRange( 50. )
, cohesionRange( 50. )
, color( ofColor::fromHsb( ofRandom( 90, 160 ), 0, 255 ) )
{
  
}

// ~Boid
//--------------------------------------------------------------
Boid::~Boid()
{
  
}

// setMaxForce
//--------------------------------------------------------------
void Boid::setMaxForce( float _maxForce )
{
  maxForce = _maxForce;
}

// setMaxSpeed
//--------------------------------------------------------------
void Boid::setMaxSpeed( float _maxSpeed )
{
  maxSpeed = _maxSpeed;
}

// setSeparateRange
//--------------------------------------------------------------
void Boid::setSeparateRange( float _separate )
{
  separateRange = _separate;
}

// setAlignRange
//--------------------------------------------------------------
void Boid::setAlignRange( float _align )
{
  alignRange = _align;
}

// setCohesionRange
//--------------------------------------------------------------
void Boid::setCohesionRange( float _cohesion )
{
  cohesionRange = _cohesion;
}

// setVelocity
//--------------------------------------------------------------
void Boid::setVelocity( ofVec3f _vel )
{
  vel = _vel;
}

// setColor
//--------------------------------------------------------------
void Boid::setColor( ofColor _color )
{
  color = _color;
}

// setRadius
//--------------------------------------------------------------
void Boid::setRadius( float _radius )
{
  radius = _radius;
}

// getIdentifier
//--------------------------------------------------------------
int Boid::getIdentifier()
{
  return identifier;
}

// getPosition
//--------------------------------------------------------------
ofVec3f Boid::getPosition()
{
  return pos;
}

// getVelocity
//--------------------------------------------------------------
ofVec3f Boid::getVelocity()
{
  return vel;
}

// getRadius
//--------------------------------------------------------------
float Boid::getRadius()
{
  return radius;
}

// getColor
//--------------------------------------------------------------
ofColor Boid::getColor()
{
  return color;
}

// setIdentifier
//--------------------------------------------------------------
void Boid::setIdentifier( vector< int >* _labels, vector< ofPolyline >* _pls, vector< ofPolyline >* _scaledPls )
{
//  color.set( 255 );
  
  // apply attract force if position is inside of a contour
  int i = 0;
  for( auto& pl : ( *_pls ) )
  {
    if( pl.inside( pos ) )
    {
      if( identifier == -1 )
      {
//        color.setHsb( ( 255 / _pls->size() ) * i, 120, 255 );
      }
      
      identifier = _labels->at( i );
      
      // scale down velocity
      vel *= 0.85;
      
      if( identifier % 3 == 0 )
      {
        radius = MAX( radius - 0.03, 4 );
      }
      else if( identifier % 3 == 1 )
      {
        radius = MIN( radius + 0.06, 20 );
      }
      else
      {
        if( radius > 8 )
        {
          radius = MAX( radius - 0.03, 8 );
        }
        else
        {
          radius = MIN( radius + 0.03, 8 );
        }
      }
      
      separateRange = ofMap( radius, 4, 20, 100, 160 );
      alignRange    = ofMap( radius, 4, 20, 80, 40 );
      cohesionRange = ofMap( radius, 4, 20, 250, 450 );
      maxForce      = ofMap( radius, 4, 20, 0.3, 0.6 );
      maxSpeed      = ofMap( radius, 4, 20, 3, 36 );
      
      applyForce( attract( pl.getCentroid2D() ) * 0.65 );
      
      return;
    }
    ++i;
  }
  
  identifier = -1;
  
  // apply repel force if position is outside of all contours
  applyForce( repel( _scaledPls ) * 4 );
}

// update
//--------------------------------------------------------------
void Boid::update( vector< Boid* >* _boids, vector< EffectPoint >* _attractPoints, vector< EffectPoint >* _repelPoints )
{
  // apply force : separate, align, cohesion, attract, repel
  flock2( _boids, _attractPoints, _repelPoints );
  
  // update position
  updatePosition();
  
  // wrap around
//  wrapAround();
  
  // bounce
  if( identifier == -1 )
  {
    separateFromWall();
    bounce();
  }
}

// flock2
//--------------------------------------------------------------
void Boid::flock2( vector< Boid* >* _boids, vector< EffectPoint >* _attractPoints, vector< EffectPoint >* _repelPoints )
{
  ofVec3f sSteer = ofVec3f( 0, 0, 0 );
  int     sCount = 0;
  
  ofVec3f aSteer = ofVec3f( 0, 0, 0 );
  ofVec3f aSum   = ofVec3f( 0, 0, 0 );
  int     aCount = 0;
  
  ofVec3f cSteer = ofVec3f( 0, 0, 0 );
  ofVec3f cSum   = ofVec3f( 0, 0, 0 );
  int     cCount = 0;
  
  for( auto& other : ( *_boids ) )
  {
    // skip process if other identifier is not the same
    if( other->getIdentifier() != identifier )
    {
      continue;
    }
    
    float dist = ( pos - other->getPosition() ).length();
    
    // 0 < dist < separateRange
    if( ( dist > 0 ) && ( dist < separateRange ) )
    {
      ofVec3f diff = pos - other->getPosition();
      diff.normalize();
      diff /= dist;
      
      // add
      sSteer += diff;
      ++sCount;
    }
    
    // 0 < dist < alignRange
    if( ( dist > 0 ) && ( dist < alignRange ) )
    {
      // add velocity
      aSum += other->getVelocity();
      ++aCount;
    }
    
    // 0 < dist < cohesionRange
    if( ( dist > 0 ) && ( dist < cohesionRange ) )
    {
      // add position
      cSum += other->getPosition();
      ++cCount;
    }
  }
  
  // ---------------------------------------- separate
  if( sCount > 0 )
  {
    // calculate average
    sSteer /= sCount;
    
    // steering = desired - velocity
    sSteer.normalize();
    sSteer *= maxSpeed;
    sSteer -= vel;
    
    // limit force
    sSteer.limit( maxForce );
  }
  
  // ---------------------------------------- align
  if( aCount > 0 )
  {
    // calculate average velocity
    aSum /= aCount;
    
    // scale to maximum speed
    aSum.normalize();
    aSum *= maxSpeed;
    
    // steering = desired - velocity
    aSteer = aSum - vel;
    
    // limit to maximum force
    aSteer.limit( maxForce );
  }
  
  // ---------------------------------------- cohesion
  if( cCount > 0 )
  {
    // calculate average position
    cSum /= cCount;
    
    // steer towards the target
    cSteer = seek( cSum );
  }
  
  ofVec3f atSteer = attract( _attractPoints );
  ofVec3f reSteer = repel( _repelPoints );
  
  // Arbitrarily weight these forces
  sSteer  *= 1.5;
  aSteer  *= 1.0;
  cSteer  *= 1.0;
  atSteer *= 1.5;
  reSteer *= 3.0;
  
  // Add the force vectors to acceleration
  applyForce( sSteer );
  applyForce( aSteer );
  applyForce( cSteer );
  
  if( _attractPoints->size() > 0 )
  {
    applyForce( atSteer );
  }
  
  if( _repelPoints->size() > 0 )
  {
    applyForce( reSteer );
  }
}

// attract : steer towards attract point
//--------------------------------------------------------------
ofVec3f Boid::attract( ofVec3f _point )
{
  return seek( _point );
}

// attract : steer towards nearest attract point
//--------------------------------------------------------------
ofVec3f Boid::attract( vector< EffectPoint >* _points )
{
  float   closest = 99999;
  ofVec3f point   = ofVec3f( 0, 0, 0 );
  float   range   = 0.0;
  
  for( auto& p : ( *_points ) )
  {
    if( p.identifier == identifier )
    {
      float dist = ( p.pos - pos ).length();
      
      if( ( dist > 0 ) && ( dist < closest ) )
      {
        closest = dist;
        point   = p.pos;
        range   = p.radius;
      }
    }
  }
  
  float dist = ( point - pos ).length();
  if( ( dist > 0 ) && ( dist < range ) )
  {
    return seek( point );
  }
  
  return ofVec3f( 0, 0, 0 );
}

// repel : steer away from repel point
//--------------------------------------------------------------
ofVec3f Boid::repel( ofVec3f _point, float _range )
{
  float repelRange = _range;
  float dist       = ( _point - pos ).length();
  
  if( ( dist > 0 ) && ( dist < repelRange ) )
  {
    return -seek( _point );
  }
  else
  {
    return ofVec3f( 0, 0, 0 );
  }
}

// repel : steer away from nearest repel point
//--------------------------------------------------------------
ofVec3f Boid::repel( vector< EffectPoint >* _points )
{
  float closest = 99999;
  ofVec3f point = ofVec3f( 0, 0, 0 );
  float range   = 0.0;
  
  for( auto& p : ( *_points ) )
  {
    if( p.identifier == identifier )
    {
      float dist = ( p.pos - pos ).length();
      
      if( ( dist > 0 ) && ( dist < closest ) )
      {
        closest = dist;
        point   = p.pos;
        range   = p.radius;
      }
    }
  }
  
  float dist = ( point - pos ).length();
  if( ( dist > 0 ) && ( dist < range ) )
  {
    return -seek( point );
  }
  
  return ofVec3f( 0, 0, 0 );
}

// repel : steer away from repel point
//--------------------------------------------------------------
ofVec3f Boid::repel( vector< ofPolyline >* _pls )
{
  float   scale = 2.;
  ofVec3f steer = ofVec3f( 0, 0, 0 );
  int     count = 0;
  
  for( auto& pl : ( *_pls ) )
  {
    if( pl.inside( pos ) )
    {
      // get closest index
      int   ci = -1;
      float cd = 99999;
      
      int j = 0;
      for( auto& p : pl.getVertices() )
      {
        float d = ( p - pos ).length();
        if( d < cd )
        {
          cd = d;
          ci = j;
        }
        ++j;
      }
      
      // add closest normal
      steer += pl.getNormalAtIndex( ci );
      ++count;
    }
  }
  
  if( count > 0 )
  {
    steer /= count;
    steer.normalize();
    steer *= maxSpeed;
    steer -= vel;
    steer.limit( maxForce );
    
    return steer;
  }
  else
  {
    return ofVec3f( 0, 0, 0 );
  }
}

// seek
//--------------------------------------------------------------
ofVec3f Boid::seek( ofVec3f _target )
{
  // a vector pointing from position to target
  ofVec3f desired = _target - pos;
  
  // scale to maximum speed
  desired.normalize();
  desired *= maxSpeed;
  
  // steering = desired - velocity
  ofVec3f steer = desired - vel;
  
  // limit to maximum force
  steer.limit( maxForce );
  
  return steer;
}

// applyForce
//--------------------------------------------------------------
void Boid::applyForce( ofVec3f _force )
{
  accel += _force;
}

// wrapAround
//--------------------------------------------------------------
void Boid::wrapAround()
{
  if( pos.x < -radius )
  {
    pos.x = ofGetWidth() + radius;
  }
  else if( pos.x > ( ofGetWidth() + radius ) )
  {
    pos.x = -radius;
  }
  
  if( pos.y < -radius )
  {
    pos.y = ofGetHeight() + radius;
  }
  else if( pos.y > ( ofGetHeight() + radius ) )
  {
    pos.y = -radius;
  }
}

// bounce
//--------------------------------------------------------------
void Boid::bounce()
{
  float damping = -1.0;
  
  // x
  if( pos.x < radius )
  {
    pos.x  = radius;
    vel.x *= damping;
  }
  else if( pos.x > ( ofGetWidth() - radius ) )
  {
    pos.x  = ofGetWidth() - radius;
    vel.x *= damping;
  }
  
  // y
  if( pos.y < radius )
  {
    pos.y  = radius;
    vel.y *= damping;
  }
  else if( pos.y > ( ofGetHeight() - radius ) )
  {
    pos.y  = ofGetHeight() - radius;
    vel.y *= damping;
  }
  
  // z
//  if( pos.z < radius )
//  {
//      pos.z  = radius;
//      vel.z *= damping;
//  }
//  else if( pos.z > ( ofGetHeight() - radius ) )
//  {
//      pos.z  = ofGetHeight() - radius;
//      vel.z *= damping;
//  }
}

// separateFromWall
//--------------------------------------------------------------
void Boid::separateFromWall()
{
  ofVec3f steer = ofVec3f( 0, 0, 0 );
  float   xEdge = ofGetWidth() * 0.1;
  float   yEdge = ofGetHeight() * 0.1;
  float   zEdge = ofGetHeight() * 0.1;
  
  // x
  if( pos.x < xEdge )
  {
    steer += seek( ofVec3f( xEdge, pos.y, pos.z ) );
  }
  else if( pos.x > ofGetWidth() - xEdge )
  {
    steer += seek( ofVec3f( ofGetWidth() - xEdge, pos.y, pos.z ) );
  }
  
  // y
  if( pos.y < yEdge )
  {
    steer += seek( ofVec3f( pos.x, yEdge, pos.z ) );
  }
  else if( pos.y > ofGetHeight() - yEdge )
  {
    steer += seek( ofVec3f( pos.x, ofGetHeight() - yEdge, pos.z ) );
  }
  
  // z
//  if( pos.z < zEdge )
//  {
//      steer += seek( ofVec3f( pos.x, pos.z, zEdge ) );
//  }
//  else if( pos.z > ofGetHeight() - zEdge )
//  {
//      steer += seek( ofVec3f( pos.x, pos.y, ofGetHeight() - zEdge ) );
//  }
  
  steer.limit( maxForce );
  
  applyForce( steer * 1.5 );
}

// updatePosition
//--------------------------------------------------------------
void Boid::updatePosition()
{
  // update velocity
  vel += accel;
  
  // limit velocity
  vel.limit( maxSpeed );
  
  // update position
  pos += vel;
  
  // reset acceleration to 0 in every frame
  accel *= 0.0;
}

// draw
//--------------------------------------------------------------
void Boid::draw()
{
  float theta = ofRadToDeg( atan2( vel.y, vel.x ) );
  
  ofFill();
  ofSetColor( color );
  
  ofPushMatrix();
  {
    ofTranslate( pos );
    ofRotate( theta );
    
    glBegin( GL_TRIANGLES );
    {
      glVertex2f( radius, 0 );
      glVertex2f( 0, radius * .75 );
      glVertex2f( -radius * 3, 0 );
      
      glVertex2f( -radius * 3, 0 );
      glVertex2f( 0, -radius * .75 );
      glVertex2f( radius, 0 );
    }
    glEnd();
//    ofDrawBitmapStringHighlight( ofToString( identifier ), 0, 0 );
//
//    ofTriangle( radius, 0, -radius, radius/2, -radius, -radius/2 );
//    ofDrawBox( radius, radius, radius );
  }
  ofPopMatrix();
}
