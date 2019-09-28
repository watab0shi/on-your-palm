#pragma once

#include "ofMain.h"


// EffectPoint
//--------------------------------------------------------------
class EffectPoint
{
public:
  EffectPoint();
  EffectPoint( ofVec3f _pos, float _radius );
  
  int     identifier;
  ofVec3f pos;
  float   radius;
};


// Boid
//--------------------------------------------------------------
class Boid
{
public:
  Boid( float _x, float _y, float _z = 0. );
  ~Boid();
  
  void setMaxForce( float _maxForce );
  void setMaxSpeed( float _maxSpeed );
  void setSeparateRange( float _separate );
  void setAlignRange( float _align );
  void setCohesionRange( float _cohesion );
  void setVelocity( ofVec3f _vel );
  void setColor( ofColor _color );
  void setRadius( float _radius );
  void setIdentifier( vector< int >* _labels, vector< ofPolyline >* _pls, vector< ofPolyline >* _scaledPls );
  
  int     getIdentifier();
  ofVec3f getPosition();
  ofVec3f getVelocity();
  float   getRadius();
  ofColor getColor();
  
  void    update( vector< Boid* >* _boids, vector< EffectPoint >* _attractPoints, vector< EffectPoint >* _repelPoints );
  void    flock2( vector< Boid* >* _boids, vector< EffectPoint >* _attractPoints, vector< EffectPoint >* _repelPoints );
  ofVec3f attract( ofVec3f _point );
  ofVec3f attract( vector< EffectPoint >* _points );
  ofVec3f repel( ofVec3f _point, float _range = 150 );
  ofVec3f repel( vector< EffectPoint >* _points );
  ofVec3f repel( vector< ofPolyline >* _pls );
  ofVec3f seek( ofVec3f _target );
  void    applyForce( ofVec3f _force );
  void    wrapAround();
  void    bounce();
  void    separateFromWall();
  void    updatePosition();
  
  void    draw();
  
private:
  int     identifier;
  
  ofVec3f pos;
  ofVec3f vel;
  ofVec3f accel;
  
  float   maxForce;
  float   maxSpeed;
  float   separateRange;
  float   alignRange;
  float   cohesionRange;
  
  float   radius;
  ofColor color;
};
