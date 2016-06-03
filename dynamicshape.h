#ifndef DYNAMICSHAPE_H
#define DYNAMICSHAPE_H

#include <qutemolLib/baseTypes.h>
#include <qutemolLib/shape.h>
#include "atompair.h"
#include <preferences.h>



class DynamicBall{
public:
  qmol::Pos prevPos; // previous position
  qmol::Pos currPos; // current position
  qmol::Scalar rad; // radius
  qmol::Col col; // color
  int ballID;

  DynamicBall();
  DynamicBall(const qmol::Ball &ball, int id);
};

class DynamicShape
{
public:
    Preferences prefs;

    std::vector<DynamicBall> ball;
    std::vector<qmol::Tube> tube;

    qmol::Pos boundSphereCenter; // bounding  sphere center
    qmol::Scalar radius; // bounding sphere radius
    qmol::Pos barycenter;

    qmol::Scalar error = 0;

    DynamicShape();
    void copyFrom(const qmol::Shape& s);

    void clear();

    void doPhysicsStep(qmol::Scalar dt, const PairStatistics& ps);

    void applySprings(const PairStatistics& ps, qmol::Scalar dt);
    void applyFixedConstrains(const PairStatistics& ps);

    qmol::Vec calculateSpringForce(qmol::Scalar k, qmol::Scalar springLength, int atomID, int atomIDPaired);
    void resolveCompenetration(const PairStatistics& ps);

    void wiggle(qmol::Scalar entropy);
    void rotateOnYAxis(qmol::Scalar force);
    void rotateOnAxis(qmol::Scalar force, qmol::Vec axis);

    std::vector<qmol::Scalar> findBounds();
    std::vector<qmol::Scalar> findBounds(qmol::Scalar maxRadius);

    void updateBoundSphereCenter();
    void updateBarycenter();


    bool checkIntersect(int atomID1, int atomID2);

    void calculatePSBonds(PairStatistics& ps);
    PairStatistics generatePSThreshold(qmol::Scalar thresh);





};

#endif // DYNAMICSHAPE_H
