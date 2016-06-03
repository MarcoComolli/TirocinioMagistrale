#ifndef SHAPINGOPTIONS_H
#define SHAPINGOPTIONS_H

#include "baseTypes.h"

namespace qmol {

struct ShapingOptions{

    enum { BALL_AND_STICK=0, LICORICE=1, SPACE_FILL=2 };
    int mode; // any of the above

    bool keepHeteros;
    bool keepHydrogens;
    float licoriceRadius;
    float bondRadius;

    /// colorize mode: in 0..2. If 0, then color per atom. 1: per chain. 2: per model
    float colorizeMode;

    /// if -1, then show all models
    int showOnlyModel;

    /// used to remove totally-culled atoms. If all 0, then no cutPlane
    Plane cutPlane;

    void setDefaults();

    bool selectedOnly;

};

}

#endif
