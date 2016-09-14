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

bool AtomPair::compareTo(AtomPair ap) const{
    int small1, small2, big1, big2;
    if(atomID1 < atomID2){
        small1 = atomID1;
        big1 = atomID2;
    }
    else{
        small1 = atomID2;
        big1 = atomID1;
    }
    if(ap.atomID1 < ap.atomID2){
        small2 = ap.atomID1;
        big2 = ap.atomID2;
    }
    else{
        small2 = ap.atomID2;
        big2 = ap.atomID1;
    }

    if (small1 == small2 && big1  == big2) {
        return true;
    }else{
        return false;
    }
}

