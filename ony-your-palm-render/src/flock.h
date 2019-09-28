#pragma once

#include "boid.h"


// Flock
//--------------------------------------------------------------
class Flock
{
  
public:
  Flock();
  ~Flock();
  
  void addBoid( Boid* _b );
  void removeBoid();
  void addAttractPoint( EffectPoint _point );
  void removeAttractPoint();
  void clearAttractPoints();
  void addRepelPoint( EffectPoint _point );
  void removeRepelPoint();
  void clearRepelPoints();
  void clear();
  
  void setMaxForce( float _maxForce );
  void setMaxSpeed( float _maxSpeed );
  void setSeparateRange( float _separate );
  void setAlignRange( float _align );
  void setCohesionRange( float _cohesion );
  
  int  getNumBoids();
  
  void update();
  
  void debugDraw();
  void draw( int _identifier );
  void draw();
  
  vector< int >*        labels;
  vector< ofPolyline >* pls;
  vector< ofPolyline >* scaledPls;
  
private:
  vector< Boid* >       boids;
  vector< EffectPoint > attractPoints;
  vector< EffectPoint > repelPoints;
  
  ofImage               texture;
};
