#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <qutemolLib/basetypes.h>


class Preferences{

public:


    bool hardBonds = true;
    bool softBonds = true;

    bool showIntersectBonds = true;
    bool showHardBonds = false;
    bool showSoftBonds = false;

    int numberExtraBonds = 10000;
    qmol::Scalar damp = 0.160;
    qmol::Scalar trailing = 1.0;
    qmol::Scalar trailThres = 2.0;
    qmol::Scalar minVarianceTrhesh = 0.99;//0.2;
    qmol::Scalar maxVarianceTrhesh = 1.0;//4.0;

    qmol::Scalar R = 20;
    qmol::Scalar applicationDistance = 0.5;

    int removedCnt = -1;
    int intersectCnt = -1;
    int hardCnt = -1;
    int softCnt = -1;




    bool showGrid = false;

};

#endif // PREFERENCES_H
