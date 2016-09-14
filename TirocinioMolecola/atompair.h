#ifndef ATOMPAIR_H
#define ATOMPAIR_H


#include <vector>
#include <qutemolLib/shape.h>


//#include <spatialgrid.h>

enum class BoundType{
    NONE = 0,
    INTERSECT = 1,
    HARD = 2,
    SOFT = 3,
    COMPENETRATION = 4
};

class AtomPair{
public:
    AtomPair();
    AtomPair(int atomID1, int atomID2,
             qmol::Scalar variance = -1, qmol::Scalar distance = -1, BoundType c = BoundType::NONE, qmol::Scalar kSpring = -1);


    int atomID1;
    int atomID2;

    qmol::Scalar distance;
    qmol::Scalar kSpring;
    BoundType constr;

    bool compareTo(AtomPair ap) const;

    bool operator<(const AtomPair &other) const {
        return atomID1 < other.atomID1;
    }

    //temp field for computation

    qmol::Scalar varSumSqrt;
    qmol::Scalar varSumPlain;
    qmol::Scalar distSum;
    qmol::Scalar variance;





};


class PairStatistics
{
public:
    PairStatistics();
    std::vector<AtomPair> pairs;
    int nShapes = 0;
    int intersectStartIdx = 0, intersectEndIdx = 0;
    int hardStartIdx = 0, hardEndIdx = 0;
    int softStartIdx = 0, softEndIdx = -1;
    int totalUniqueAtoms;

    void populateRandom(int nPairs, int totalAtoms, qmol::Scalar increase);
    void populateOne(int id1, int id2);

    //void populateWithGrid(const qmol::Shape& shape,const SpatialGrid& grid ); <- moved to spatialgrid
                                                                       //to avoid circular includes

    void reset(); // azzera i dati
    void add_shape(const qmol::Shape& shape);
    void calculateStats();
    void calculateBonds();
    void printStats();
    AtomPair getAtomPair(AtomPair &ap) const;

    void updateDistancesFromShape(const qmol::Shape &shape);

    void orderPairsByBound();



private:
    int calculateUniqueAtoms();
};



#endif // ATOMPAIR_H
