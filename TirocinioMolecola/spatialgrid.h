#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include <vector>
#include <dynamicshape.h>
#include <atompair.h>


using namespace std;

class SpatialGridCell : public vector<int> {
public:
};

/*
typedef vector<int> SpatialGridCell;
*/

/*
class SpatialGridCell : public vector<int> {
public:
    vector<int> el;
};*/




class SpatialGrid{
public:

    vector< vector< vector<SpatialGridCell> > > indexesGrid;

    vector<qmol::Scalar> gridX;
    vector<qmol::Scalar> gridY;
    vector<qmol::Scalar> gridZ;

    qmol::Scalar cellDimX;
    qmol::Scalar cellDimY;
    qmol::Scalar cellDimZ;

    SpatialGrid();
    void reset();
    void set(vector<qmol::Scalar> bounds, int nEdgeCells);
    /*SpatialGrid(vector<float> bounds, vector<int> nEdgeCellsXYZ);
    SpatialGrid(vector<float> bounds, int nEdgeCells, float cellSize);
    SpatialGrid(vector<float> bounds, vector<int> nEdgeCellsXYZ, vector<float> cellSizeXYZ);*/


    void populateWithShape(const DynamicShape& shape);
    void populateWithShape(const DynamicShape& shape, qmol::Scalar radius);
    void initGrid(int x, int y, int z);

    //non mettere nel PairStatistic per evitare circular includes
    PairStatistics generatePairStatistcs(const DynamicShape& shape);
    PairStatistics generatePairStatistcs(const DynamicShape& ds, qmol::Scalar threshold);
    PairStatistics generatePairStatistcs(const DynamicShape& ds, qmol::Scalar threshold, int randomPairs);


    //test
    void populateTest(qmol::Pos p, qmol::Scalar radius);
    void printGridStats();

private:
    int findGridIndex(qmol::Scalar pos, vector<qmol::Scalar> gridAxis);
    bool checkCovalent(qmol::Pos c1, qmol::Scalar r1, qmol::Pos c2, qmol::Scalar r2);
    vector<int> matchAtoms(vector<int> indices);
};

#endif // SPATIALGRID_H
