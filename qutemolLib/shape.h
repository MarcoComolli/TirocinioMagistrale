#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <vcg/space/point3.h>
#include <vcg/space/box3.h>

#include "baseTypes.h"

class glProgram;
namespace glw{
  class surface;
}

namespace qmol {


/** Ball: a sphere */
class Ball{
public:
  Pos pos; // position
  float rad; // radius
  Col col; // color
};

/** Tube: a cylinder */
class Tube{
public:
  Pos pos; // position (center)
  float rad; // radius
  Vec len; // lenght vector (from center to either endpoint)
  Col col; // color

  // constructor (two endpoints)
  Tube(const Pos &a, const Pos &b);
};

/** Shape: a collections of balls'n'tubes which can be drawn on screen */
class Shape
{  
public:

  /* data */
  /* **** */
  std::vector<Ball> ball;
  std::vector<Tube> tube;

  Pos center; // bounding  sphere center
  Scalar radius; // bounding sphere radius

  Shape();
  void clear();
  void updateBoundingSphere();


  void shuffle();

};



} // namespace qmol

#endif // SHAPE_H
