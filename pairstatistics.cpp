#include <qutemolLib/baseTypes.h>
#include "atompair.h"
#include <set>

using namespace std;
using namespace qmol;
typedef unsigned int uint;

PairStatistics::PairStatistics(){
    nShapes = 0;
}

void PairStatistics::populateRandom(int nPairs, int totalAtoms, qmol::Scalar increase){
    qmol::Scalar scale = 1.0f;
    int aID1 = -1;
    int aID2 = -1;

    for (int i = 0; i < nPairs; ++i) {
        aID1 = (rand()%totalAtoms)+1;
        aID2 = aID1 + (rand()%(int) scale)+1;
        aID2 = (aID2%totalAtoms)+1;
        scale = scale*increase;
        AtomPair ap = AtomPair(aID1,aID2);
        pairs.push_back(ap);
    }

}

void PairStatistics::populateOne(int aID1, int aID2){

    AtomPair ap = AtomPair(aID1,aID2);
    pairs.push_back(ap);


}

void PairStatistics::reset(){
    nShapes = 0;
    for (uint i = 0; i < pairs.size(); ++i) {
        pairs[i].variance = 0;
        pairs[i].distance = 0;
        pairs[i].varSumSqrt = 0;
        pairs[i].varSumPlain = 0;
        pairs[i].distSum = 0;
    }
}

void PairStatistics::add_shape(const qmol::Shape& shape){

    nShapes++;
    Pos p1,p2;
    Scalar sqrDistance;
    for (uint i = 0; i < pairs.size(); ++i) {
        p1 = shape.ball[pairs[i].atomID1].pos;
        p2 = shape.ball[pairs[i].atomID2].pos;
        sqrDistance = qmol::sqrDistance(p1,p2);


        pairs[i].varSumSqrt += sqrDistance;
        pairs[i].varSumPlain += std::sqrt(sqrDistance);
        pairs[i].distSum += std::sqrt(sqrDistance);

    }

}

void PairStatistics::calculateStats(){
    for (uint i = 0; i < pairs.size(); ++i) {
        pairs[i].variance = (pairs[i].varSumSqrt/nShapes) - (pow((pairs[i].varSumPlain/nShapes),2));
        pairs[i].distance = pairs[i].distSum/nShapes;
    }
    totalUniqueAtoms = calculateUniqueAtoms();
}

int PairStatistics::calculateUniqueAtoms(){
    set<int> s;
    for (uint i = 0; i < pairs.size(); ++i) {
        s.insert(pairs[i].atomID1);
        s.insert(pairs[i].atomID2);
    }

    return s.size();
}

AtomPair PairStatistics::getAtomPair(AtomPair& ap) const{

    for (uint i = 0; i < pairs.size(); ++i) {
        if(pairs[i].atomID1 == ap.atomID1 && pairs[i].atomID2 == ap.atomID2){
            return pairs[i];
        }
    }
    return AtomPair(-1,-1);
}

void PairStatistics::printStats(){
    std::cout << "\n--< Pair Statistics Stats >--" << std::endl;
    std::cout << "Number of pairs: " << pairs.size()  << std::endl;
    std::cout << "Number of shapes: " << nShapes  << std::endl;
    std::cout << "Total unique atoms: " << totalUniqueAtoms  << std::endl;

    vector<Scalar> variances;
    vector<Scalar> distances;
    Scalar totV = 0;
    Scalar totD = 0;
    for (uint i = 0; i < pairs.size(); ++i) {
        variances.push_back(pairs[i].variance);
        distances.push_back(pairs[i].distance);
        totV += pairs[i].variance;
        totD += pairs[i].distance;
    }
    if(!variances.empty()){
        Scalar minV = *std::min_element(variances.begin(),variances.end());
        Scalar maxV = *std::max_element(variances.begin(),variances.end());
        Scalar minD = *std::min_element(distances.begin(),distances.end());
        Scalar maxD = *std::max_element(distances.begin(),distances.end());

        std::cout << "MIN variance: " << minV  << std::endl;
        std::cout << "MAX variance: " << maxV  << std::endl;
        std::cout << "AVG variance: " << totV/pairs.size()  << std::endl;

        std::cout << "MIN distance: " << minD  << std::endl;
        std::cout << "MAX distance: " << maxD  << std::endl;
        std::cout << "AVG distance: " << totD/pairs.size()  << std::endl;
    }

    std::cout << "----------------------------\n" << std::endl;

}

void PairStatistics::updateDistancesFromShape(const qmol::Shape &shape){
    for (uint i = 0; i < pairs.size(); ++i) {
        AtomPair& p = pairs[i];
        p.distance = qmol::distance( shape.ball[p.atomID1].pos , shape.ball[p.atomID2].pos );
    }
}

bool compareAPByBound(AtomPair a,AtomPair b) {
    return (a.constr < b.constr);
}

void PairStatistics::orderPairsByBound(){


    sort(pairs.begin(), pairs.end(), compareAPByBound);

    for (int i = 0; i < pairs.size(); ++i) {
        if(i != i < pairs.size() -1){
            if((int)pairs[i].constr < (int)pairs[i+1].constr){
                std::cout << i-1 << " -> " << (int)pairs[i-1].constr << std::endl;
                std::cout << i << " -> " << (int)pairs[i].constr << std::endl;
                std::cout << i+1 << " -> " << (int)pairs[i+1].constr << std::endl << std::endl;
                if(pairs[i].constr == BoundType::INTERSECT){
                    intersectEndIdx = i;
                    hardStartIdx = i+1;
                }
                if(pairs[i].constr == BoundType::HARD){
                    hardEndIdx = i;
                    softStartIdx = i+1;
                }
            }
        }
    }
    intersectStartIdx = 0;
    softEndIdx = pairs.size() -1;

    std::cout << "int start " << intersectStartIdx << std::endl;
    std::cout << "int end " << intersectEndIdx << std::endl;
    std::cout << "hard start " << hardStartIdx << std::endl;
    std::cout << "hard end " << hardEndIdx << std::endl;
    std::cout << "soft start " << softStartIdx << std::endl;
    std::cout << "soft end " << softEndIdx << std::endl;

}
