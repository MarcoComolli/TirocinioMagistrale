#include "atompair.h"

AtomPair::AtomPair()  {
    distSum = 0;
    varSumSqrt = 0;
    varSumPlain = 0;
}

AtomPair::AtomPair(int atomID1, int atomID2, qmol::Scalar variance, qmol::Scalar distance, BoundType c, qmol::Scalar _kSpring)
        : constr(c) , kSpring(_kSpring)

{
    this->atomID1 = atomID1;
    this->atomID2 = atomID2;
    this->variance = variance;
    this->distance = distance;

    distSum = 0;
    varSumPlain = 0;
    varSumSqrt = 0;


}
