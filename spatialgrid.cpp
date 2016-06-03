#include "spatialgrid.h"
#include <set>

typedef unsigned int uint;

using namespace qmol;
SpatialGrid::SpatialGrid(){

}

//bounds in format minX, maxX, minY, maxY, minZ, maxZ
void SpatialGrid::set(vector<Scalar> bounds, int nEdgeCells){
    Scalar xgap = bounds[1] - bounds[0];
    Scalar ygap = bounds[3] - bounds[2];
    Scalar zgap = bounds[5] - bounds[4];

    cellDimX = fabsf(xgap)/nEdgeCells;
    cellDimY = fabsf(ygap)/nEdgeCells;
    cellDimZ = fabsf(zgap)/nEdgeCells;

    gridX.push_back(bounds[0]);
    gridY.push_back(bounds[2]);
    gridZ.push_back(bounds[4]);

    for (int i = 0; i < nEdgeCells; ++i) {
        gridX.push_back(bounds[0] + (i+1)*cellDimX);
        gridY.push_back(bounds[2] + (i+1)*cellDimY);
        gridZ.push_back(bounds[4] + (i+1)*cellDimZ);
    }

    initGrid(nEdgeCells,nEdgeCells,nEdgeCells);
}
/*
SpatialGrid::SpatialGrid(vector<float> bounds, vector<int> nEdgeCellsXYZ){
    float xgap = bounds[1] - bounds[0];
    float ygap = bounds[3] - bounds[2];
    float zgap = bounds[5] - bounds[4];


    cellDimX = fabsf(xgap)/nEdgeCellsXYZ[0];
    cellDimY = fabsf(ygap)/nEdgeCellsXYZ[1];
    cellDimZ = fabsf(zgap)/nEdgeCellsXYZ[2];


    for (int i = 0; i < nEdgeCellsXYZ[0]; ++i) {
        gridX.push_back(bounds[0] +(i+1)*cellDimX);
    }

    for (int i = 0; i < nEdgeCellsXYZ[1]; ++i) {
        gridY.push_back(bounds[2] + (i+1)*cellDimY);
    }

    for (int i = 0; i < nEdgeCellsXYZ[2]; ++i) {
        gridZ.push_back(bounds[4] +(i+1)*cellDimZ);
    }


    initGrid(nEdgeCellsXYZ[0],nEdgeCellsXYZ[1],nEdgeCellsXYZ[2]);

}


SpatialGrid::SpatialGrid(vector<float> bounds, int nEdgeCells, float cellSize){


    cellDimX = cellDimY = cellDimZ = cellSize;

    for (int i = 0; i < nEdgeCells; ++i) {
        gridX.push_back(bounds[0] + (i+1)*cellSize);
        gridY.push_back(bounds[2] + (i+1)*cellSize);
        gridZ.push_back(bounds[4] + (i+1)*cellSize);
    }



    initGrid(nEdgeCells,nEdgeCells,nEdgeCells);


}

SpatialGrid::SpatialGrid(vector<float> bounds, vector<int> nEdgeCellsXYZ, vector<float> cellSizeXYZ){

    cellDimX = cellSizeXYZ[0];
    cellDimY = cellSizeXYZ[1];
    cellDimZ = cellSizeXYZ[2];


    for (int i = 0; i < nEdgeCellsXYZ[0]; ++i) {
        gridX.push_back(bounds[0] +(i+1)*cellDimX);
    }

    for (int i = 0; i < nEdgeCellsXYZ[1]; ++i) {
        gridY.push_back(bounds[2] + (i+1)*cellDimY);
    }

    for (int i = 0; i < nEdgeCellsXYZ[2]; ++i) {
        gridZ.push_back(bounds[4] +(i+1)*cellDimZ);
    }


    initGrid(nEdgeCellsXYZ[0],nEdgeCellsXYZ[1],nEdgeCellsXYZ[2]);
}
*/

int SpatialGrid::findGridIndex(Scalar pos, vector<Scalar> gridAxis){

    for (uint i = 1; i < gridAxis.size(); ++i) {
        if(pos < gridAxis[i]){
            return i-1;
        }
    }
    return -2;
}




void SpatialGrid::populateWithShape(const DynamicShape &shape){

    qmol::Pos pos;
    int x,y,z;

    for (uint i = 0; i < shape.ball.size(); ++i) { //per ogni sfera
        pos = shape.ball[i].currPos;
        x= findGridIndex(pos.X(),gridX);
        y= findGridIndex(pos.Y(),gridY);
        z= findGridIndex(pos.Z(),gridZ);
        indexesGrid[x][y][z].push_back(shape.ball[i].ballID);
    }
}

void SpatialGrid::populateWithShape(const DynamicShape &shape, Scalar radius){
    Scalar xl,xh, yl, yh, zl, zh;
    qmol::Pos pos;

    for (uint i = 0; i < shape.ball.size(); ++i) { //per ogni sfera
        pos = shape.ball[i].currPos;
        xh = findGridIndex(pos.X()+radius,gridX);
        yh = findGridIndex(pos.Y()+radius,gridY);
        zh = findGridIndex(pos.Z()+radius,gridZ);
        xl = findGridIndex(pos.X()-radius,gridX);
        yl = findGridIndex(pos.Y()-radius,gridY);
        zl = findGridIndex(pos.Z()-radius,gridZ);

        for (int x = xl; x < xh+1; ++x) {
            for (int y = yl; y < yh+1; ++y) {
                for (int z = zl; z < zh+1; ++z) {
                    indexesGrid[x][y][z].push_back(shape.ball[i].ballID);
                }
            }
        }

    }

}

//void SpatialGrid::populateTest(qmol::Pos p, float radius){
//    qmol::Pos pos;
//    float xl,xh, yl, yh, zl, zh;

//    pos = p;
//    xh = findGridIndex(pos.X()+radius,gridX);
//    yh = findGridIndex(pos.Y()+radius,gridY);
//    zh = findGridIndex(pos.Z()+radius,gridZ);
//    xl = findGridIndex(pos.X()-radius,gridX);
//    yl = findGridIndex(pos.Y()-radius,gridY);
//    zl = findGridIndex(pos.Z()-radius,gridZ);
//    cout << xh << " " << yh << " "<< zh <<"\n" << xl<< " "<< yl <<" " << zl <<endl;

//    for (int x = xl; x < xh+1; ++x) {
//        for (int y = yl; y < yh+1; ++y) {
//            for (int z = zl; z < zh+1; ++z) {
//                indexesGrid[x][y][z].push_back(11);
//                cout << "x: " << x << " y: "<< y << " z: "<< z << endl;
//            }
//        }
//    }
//    cout << endl;


//}





void SpatialGrid::initGrid(int dx, int dy, int dz){

    //create grid with xyz dimensions
    //vector<vector<vector<SpatialGridCell>>> grid(dx,vector<vector<SpatialGridCell>>(dy, vector<SpatialGridCell>(dz)));
    //vector<vector<vector<SpatialGridCell>>> grid(dx);

    indexesGrid.resize(dx);

    for (int x = 0; x < dx; ++x) {
        indexesGrid[x].resize(dy);
        for (int y = 0; y < dy; ++y) {
            indexesGrid[x][y].resize(dz);
            for (int z = 0; z < dz; ++z) {
                indexesGrid[x][y][z] = SpatialGridCell();
            }
        }
    }

}

void SpatialGrid::printGridStats(){


    cout << "\n|----| GRID STATS |----|"<< endl;

    cout << "matrice 3d: " << indexesGrid.size() << "x"
         << indexesGrid[0].size() << "x"
         <<  indexesGrid[0][0].size()<< endl;

    int nCells = indexesGrid.size()*indexesGrid[0].size()*indexesGrid[0][0].size();

    cout << "nCells: " << nCells << endl;

    int count = 0, fullCells = 0;
    uint max = 0;

    for (uint x = 0; x < indexesGrid.size(); ++x) {
        for (uint y = 0; y < indexesGrid[0].size(); ++y) {
            for (uint z = 0; z < indexesGrid[0][0].size(); ++z) {
                count += indexesGrid[x][y][z].size();
                if(indexesGrid[x][y][z].size() != 0){
                    fullCells++;
                    if(indexesGrid[x][y][z].size() > max){
                        max = indexesGrid[x][y][z].size();
                    }
                }

            }
        }
    }

    cout << "Total cells not empty: " << fullCells << endl;
    cout << "Total atoms*cell fulled: " << count << endl;

    cout << "Average atoms per non empty cell: " << fixed <<((float)count)/fullCells << endl;
    cout << "Average atoms per cell: " << fixed <<((float)count)/nCells << endl;
    cout << "Max per cells: " << max << endl;

    cout << "\n|--------------------------|\n"<< endl;






}


bool SpatialGrid::checkCovalent(qmol::Pos c1, Scalar r1, qmol::Pos c2, Scalar r2){
    Scalar rsum = r1 + r2;
    Scalar dist = (c1 - c2).Norm();
    if(dist <= rsum){
        return true;
    }
    else{
        return false;
    }

}



static void unique(vector<int> a){

    set<int> b;
    for (uint i = 0; i < a.size(); ++i) {
        b.insert(a[i]);
    }
    cout << "Atomi unici processati: " << b.size() << endl;
}


//crea un array con gli accoppiamenti presenti in una cella (2 consecutivi = 1 coppia)
//ogni atomo fa coppia con ogni altro atomo nella cella
vector<int> SpatialGrid::matchAtoms(vector<int> indices){
    vector<int> matchedIndex;
    int a=0;
    for (uint i = 0; i < indices.size(); ++i) {
        for (uint j = i+1; j <indices.size(); ++j) {
            matchedIndex.push_back(indices[i]);
            matchedIndex.push_back(indices[j]);
            a += 2;
        }
    }
    //unique(matchedIndex);
    return matchedIndex;
}



PairStatistics SpatialGrid::generatePairStatistcs(const DynamicShape& shape){

    /*
    *  //PSEUDOCODICE
    *  per ogni cella della griglia
    *    se ha almeno 2 elementi
    *        prendi il vettore con dentro gli indici e calcola gli accoppiamenti
    *        per ogni coppia trovata crea un atompair
    *        controlla se è dativo
    *            se si metti dativo
    */

    PairStatistics ps;
    vector<int> pairsIDs;
    //per ogni cella della griglia
    for (uint x = 0; x < indexesGrid.size(); ++x) {
        for (uint y = 0; y < indexesGrid[0].size(); ++y) {
            for (uint z = 0; z < indexesGrid[0][0].size(); ++z) {
                if(indexesGrid[x][y][z].size() > 1){ //se ha almeno 2 elem
                    pairsIDs = matchAtoms(indexesGrid[x][y][z]); // per ogni coppia trovata crea un atompair
                    for (uint i = 0; i < pairsIDs.size(); i += 2) {
                        AtomPair ap = AtomPair(pairsIDs[i],pairsIDs[i+1]);
                        if(checkCovalent(shape.ball.at(pairsIDs[i]).currPos,shape.ball.at(pairsIDs[i]).rad,       //controlla se
                                       shape.ball.at(pairsIDs[i+1]).currPos,shape.ball.at(pairsIDs[i+1]).rad)){ //è dativo
                            ap.constr = BoundType::INTERSECT;
                        }
                        ps.pairs.push_back(ap);
                    }
                    //unique(pairsIDs);
                }
            }
        }
    }
    return ps;

}




PairStatistics SpatialGrid::generatePairStatistcs(const DynamicShape& ds, Scalar threshold){

    /*
    *  //PSEUDOCODICE
    *  per ogni atomo nello shape
    *    calcola la sottogriglia con distanza con
    *    controlli per non sforare dalla griglia grande
    *        per ogni casella della sottogriglia
    *           calcola la distanza con gli atomi nelle celle
    *           se è minore della soglia aggiungi la coppia
    */

    PairStatistics ps;

    //per ogni atomo nello shape
    for (uint i = 0; i < ds.ball.size(); ++i) {
        qmol::Pos atomPos = ds.ball[i].currPos;

        int xl = findGridIndex((atomPos.X() - threshold),gridX); //x low
        int xh = findGridIndex((atomPos.X() + threshold),gridX)+1; //x high
        int yl = findGridIndex((atomPos.Y() - threshold),gridY); //y low
        int yh = findGridIndex((atomPos.Y() + threshold),gridY)+1; //y high
        int zl = findGridIndex((atomPos.Z() - threshold),gridZ); //z low
        int zh = findGridIndex((atomPos.Z() + threshold),gridZ)+1; //z high


        if(xh == -1){
            xh = gridX.size() - 1;
        }
        if(yh == -1){
            yh = gridY.size() - 1;
        }
        if(zh == -1){
            zh = gridZ.size() - 1;
        }

        //per ogni cella della sottogriglia
        for (int x = xl; x < xh; ++x) {
            for (int y = yl; y < yh; ++y) {
                for (int z = zl; z < zh; ++z) {
                    for (uint j = 0; j < indexesGrid[x][y][z].size(); ++j) {
                        qmol::Pos tempPos = ds.ball.at(indexesGrid[x][y][z].at(j)).currPos;
                        if(indexesGrid[x][y][z].at(j) > (int)i){
                            if(qmol::sqrDistance(atomPos, tempPos) < (threshold*threshold)){//Calcola la distanza con soglia
                                AtomPair ap(i,indexesGrid[x][y][z].at(j));
                                //Se ok fai l'atompair e mettila dentro
                                ps.pairs.push_back(ap);
                            }
                        }
                    }
                }
            }
        }
    }

    return ps;


}

PairStatistics SpatialGrid::generatePairStatistcs(const DynamicShape& ds, Scalar threshold, int randomPairs){


    PairStatistics ps = generatePairStatistcs(ds,threshold);
    int atomIdx1 = -1, atomIdx2 = -1;
    for (int i = 0; i < randomPairs; ++i) {
        atomIdx1 = rand()%ds.ball.size();
        atomIdx2 = rand()%ds.ball.size();

        while(atomIdx1 == atomIdx2){

            atomIdx1 = rand()%ds.ball.size();
            atomIdx2 = rand()%ds.ball.size();
        }

        ps.pairs.push_back(AtomPair(atomIdx1,atomIdx2));

    }

    return ps;

}


void SpatialGrid::reset(){
    indexesGrid.clear();

    gridX.clear();
    gridY.clear();
    gridZ.clear();

    cellDimX = 0;
    cellDimY = 0;
    cellDimZ = 0;
}
