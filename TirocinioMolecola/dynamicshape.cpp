#include "dynamicshape.h"
#include <random>
#include <sstream>

using namespace qmol;
using namespace std;
typedef unsigned int uint;

DynamicShape::DynamicShape(){
}

std::string printPos(qmol::Pos p){
    std::stringstream ss;
    ss << p.X() << "," << p.Y() << "," << p.Z();
    return  ss.str();
}

void DynamicShape::copyFrom(const qmol::Shape& s){

    clear();
    boundSphereCenter = s.center;
    radius = s.radius;
    for (uint i = 0; i < s.ball.size(); ++i) {
        ball.push_back(DynamicBall(s.ball.at(i),i));
    }

    for (uint i = 0; i < s.tube.size(); ++i) {
        tube.push_back(s.tube.at(i));
    }
    updateBarycenter(); //generate in this case

   std::cout << "THE BARYCENTER: " << printPos(barycenter) << std::endl;


}

void DynamicShape::clear(){
    ball.clear();
    tube.clear();
}

static bool almostEqual(Scalar a, Scalar b){
    const Scalar epsilon = 1e-4;
    return ((a-b) < epsilon  && (a-b) > -epsilon);

}

void DynamicShape::applySprings(const PairStatistics& ps, Scalar dt){


    //per ogni atompair che ha le molle
    //la forza risultante è data da:
    //1)k
    //2)lunghezza(PosAtomoAttuale-PosAtomoDellaCoppia) - distanza media
    //3)vettore(PosAtomoAttuale-PosAtomoDellaCoppia)/lunghezza(PosAtomoAttuale-PosAtomoDellaCoppia)


    for (int i = ps.softStartIdx; i <=  ps.softEndIdx ; ++i) {
            qmol::Vec force = calculateSpringForce(ps.pairs[i].kSpring,
                                                   ps.pairs.at(i).distance,
                                                   ps.pairs.at(i).atomID1,
                                                   ps.pairs.at(i).atomID2
                                                   );



            //std::cout << "Force: " << printPos(force) << std::endl;
            force *= dt*dt;



            ball.at(ps.pairs.at(i).atomID1).currPos += force; //current pos è ora a currentpos + forza (nel dt)
            ball.at(ps.pairs.at(i).atomID2).currPos -= force; //current pos è ora a currentpos - forza (nel dt)
    }

}

void DynamicShape::applyFixedConstrains(const PairStatistics& ps){
    error = 0;
    for (unsigned int i = ps.intersectStartIdx; i <= ps.hardEndIdx ; ++i) {
             qmol::Vec delta =
                     ball.at(ps.pairs.at(i).atomID2).currPos -
                     ball.at(ps.pairs.at(i).atomID1).currPos;

            Scalar currDist = delta.Norm();

            //for debug
            //float currDist = delta.Norm();
            if (almostEqual(currDist, ps.pairs.at(i).distance)){ continue;}
            if(almostEqual(currDist,0)) continue;

            error = std::max(error,qmol::abs(currDist - ps.pairs.at(i).distance));

            Scalar diff = (currDist - ps.pairs.at(i).distance)/currDist;

            ball.at(ps.pairs.at(i).atomID1).currPos += delta * (0.5 * diff);
            ball.at(ps.pairs.at(i).atomID2).currPos -= delta * (0.5 * diff);
    }
}

void DynamicShape::initCollisionDetection(){
    probableCollisions.clear();
    compenetrationIdx = 0;
    compenetrationIdxOffset = 0;

}


void DynamicShape::resolveCompenetration(int idx1, int idx2){

    DynamicBall b1 = ball[idx1];
    DynamicBall b2 = ball[idx2];

    Scalar module = (b2.rad + b1.rad) - (b2.currPos - b1.currPos).Norm();

    ball[idx1].currPos +=  (b1.currPos - b2.currPos).Normalize() * module/2;
    ball[idx2].currPos +=  (b2.currPos - b1.currPos).Normalize() * module/2;

    ball[idx1].prevPos =  ball[idx1].currPos;
    ball[idx2].prevPos =  ball[idx2].currPos;
    //ball[idx1].col = Col(0,1,0);
    //ball[idx2].col = Col(0,0,1);

}

void DynamicShape::searchForCompenetration(int start,const PairStatistics& ps){
    int pairsToBeTested = 1000;
    int interval = rand()%700+10;
    int atomIdx1 = start;
    int atomIdx2;
    bool areTheyIntersectBond = false;

    for (int i = 0; i < pairsToBeTested; ++i){
        atomIdx2 = (atomIdx1+interval+compenetrationIdxOffset)%ball.size();

        //for visualization
        //ball[atomIdx1].col = Col(1,0,0);
        //ball[atomIdx2].col = Col(0,1,1);

        if(testCompenetration(atomIdx1,atomIdx2)){
            for (int j = ps.intersectStartIdx; j < ps.intersectEndIdx; ++j) {
                if(ps.pairs[j].compareTo(AtomPair(atomIdx1,atomIdx2))){
                    areTheyIntersectBond = true;
                    break;
                }
            }
           if(atomIdx1 < atomIdx2 && !areTheyIntersectBond){
               probableCollisions.insert(AtomPair(atomIdx1, atomIdx2));
           }
           else if(atomIdx1 >= atomIdx2 && !areTheyIntersectBond){
               probableCollisions.insert(AtomPair(atomIdx2, atomIdx1));
           }
        }
        //std::cout << "tested: " << atomIdx1 << "-" << atomIdx2  << std::endl;
        atomIdx1 = (atomIdx1+1)%ball.size();
    }
}


void DynamicShape::checkProbableCollisions(){
    if(probableCollisions.empty()){
        return;
    }
    for (auto itr = probableCollisions.begin(); itr != probableCollisions.end(); ++itr){
        AtomPair ap = *itr;
        if(testCompenetration(ap.atomID1,ap.atomID2)){
            resolveCompenetration(ap.atomID1, ap.atomID2);
        }
    }

}


//negative number mean that they intersect
bool DynamicShape::testCompenetration(int idx1, int idx2){
    DynamicBall db1 = ball.at(idx1);
    DynamicBall db2 = ball.at(idx2);
    Scalar currentDist = (db1.currPos - db2.currPos).Norm();
    Scalar sumRadius = db1.rad + db2.rad;

    if(currentDist < sumRadius){ //they intersect
        return true;
    }
    else{                        //they don't intersect
        return false;
    }

}


void DynamicShape::doPhysicsStep(Scalar dt, const PairStatistics& ps){

    if (prefs.softBonds) {
        applySprings(ps,dt);
    }

    //forza = f(posNow)
    //acc = f/m
    Scalar damp = 1 - prefs.damp;
    Scalar speed = 0;
    qmol::Pos pNext;
    for (int i = 0; i < ps.totalUniqueAtoms; ++i) {
        //position
        pNext = ball.at(i).currPos + ((ball.at(i).currPos - ball.at(i).prevPos) * damp); //+ acc*dt*dt;
        ball.at(i).prevPos = ball.at(i).currPos;
        ball.at(i).currPos = pNext;
        speed = (ball.at(0).currPos - ball.at(0).prevPos).Norm();
        if(speed < 0.0001){ //some precision errors increase infinitesimal speed. Set to 0 if so
            ball.at(i).prevPos = pNext;
            ball.at(i).currPos = pNext;
        }
    }

    if (prefs.hardBonds) {
        Scalar prevErr;
        Scalar errorDiff = 1;
        int i = 0;
        while (qmol::abs(errorDiff) > 0.0001 && i < 5){
            prevErr = error;
            searchForCompenetration(compenetrationIdx, ps);
            checkProbableCollisions();
            applyFixedConstrains(ps);
            errorDiff  = prevErr-error;
            i++;
        }
    }
    updateBarycenter();
    compenetrationIdx = (compenetrationIdx+1)%ball.size();
    if(compenetrationIdx == 0) {
        compenetrationIdxOffset = (compenetrationIdxOffset+1)%ball.size();
        //std::cout << "compenetrationIdxOffset +1! -> " << compenetrationIdxOffset << std::endl;
        //std::cout << "da testare: " << probableCollisions.size() << std::endl;
    }

}

//given atoms ID, normal spring lenght, and elastic const k calculate
//the force of the spring and return it
qmol::Vec DynamicShape::calculateSpringForce(Scalar k, Scalar springLength, int atomID, int atomIDPaired){


    qmol::Pos a1Pos = ball.at(atomID).currPos;
    qmol::Pos a2Pos = ball.at(atomIDPaired).currPos;

//     std::cout << "a1 POS: " << printPos(a1Pos) << std::endl;
//     std::cout << "a2 POS: " << printPos(a2Pos) << std::endl;

     if(isnan(a1Pos.X())){
         std::cout << "is nan" << std::endl;
     }

    Scalar actualLength = qmol::distance(a1Pos,a2Pos);


    Scalar factor = (springLength-actualLength)/actualLength; //così facendo: pos in compressione, neg in allungamento


    if(actualLength < 0.001){
        if(springLength < 0.001)
            return qmol::Vec(0,0,0);
        else
            factor = 1;
    }
    qmol::Vec dir(a1Pos - a2Pos);


    return dir*(k*factor);

}


std::vector<Scalar> DynamicShape::findBounds(){
    Scalar xl,xh, yl, yh, zl, zh;

    xl = xh = ball[0].currPos.X();
    yl =  yh =  ball[0].currPos.Y();
    zl =  zh = ball[0].currPos.Z();

    qmol::Pos pos;
    for (uint i = 0; i < ball.size(); ++i) {
        pos = ball.at(i).currPos;
        if(pos.X() < xl){xl = pos.X();}
        if(pos.Y() < yl){yl = pos.Y();}
        if(pos.Z() < zl){zl = pos.Z();}
        if(pos.X() > xh){xh = pos.X();}
        if(pos.Y() > yh){yh = pos.Y();}
        if(pos.Z() > zh){zh = pos.Z();}
    }

    std::vector<Scalar> v;

    v.push_back(xl);
    v.push_back(xh);
    v.push_back(yl);
    v.push_back(yh);
    v.push_back(zl);
    v.push_back(zh);

    return v;
}

std::vector<Scalar> DynamicShape::findBounds(Scalar maxRadius){
    std::vector<Scalar> v = findBounds();

    v[0] -= maxRadius;
    v[1] += maxRadius;
    v[2] -= maxRadius;
    v[3] += maxRadius;
    v[4] -= maxRadius;
    v[5] += maxRadius;

    return v;
}

static Scalar randModOne(){
    return ((rand()%2001)-1000)/1000.0f;
}

static Vec randomVecMax1(){
    while(true){
        Vec v(randModOne(),randModOne(),randModOne());
        if (v.SquaredNorm()<1.0f)
            return v;
    }
}


void DynamicShape::wiggle(Scalar entropy){


    for (uint i = 0; i < ball.size(); ++i) {
        ball[i].currPos += randomVecMax1()*entropy;
    }
}

//check if 2 atoms intersect each other
bool DynamicShape::checkIntersect(int atomID1, int atomID2){
    qmol::Pos c1 = ball.at(atomID1).currPos;
    qmol::Pos c2 = ball.at(atomID2).currPos;
    Scalar r1 = ball.at(atomID1).rad;
    Scalar r2 = ball.at(atomID2).rad;

    Scalar rsum = r1 + r2;
    Scalar dist = (c1 - c2).Norm();

    if(dist <= rsum){
        return true;
    }
    else{
        return false;
    }
}


Scalar calculateSpringK(Scalar variance){
    if(variance < 0.01){
        std::cout << "ERRORE VARIANZA = 0 PER MOLLA" << std::endl;
        return 1000;
    }
    return 10.0f/variance;
}

//scandisce le coppie e assegna il legame corretto
//o scarta le coppie con varianza alta
void DynamicShape::calculatePSBonds(PairStatistics& ps){

    Scalar maxToleredVar = prefs.maxVarianceTrhesh;
    Scalar lowVariance = prefs.minVarianceTrhesh;
    int cInt = 0, cRem = 0, cSoft = 0, cHard = 0;
    vector<int> toRemove;

    for (uint i = 0; i < ps.pairs.size(); ++i) {
        if(checkIntersect(ps.pairs[i].atomID1, ps.pairs[i].atomID2)){
            ps.pairs[i].constr = BoundType::INTERSECT;
            cInt++;
        }
        else if(ps.pairs[i].variance > maxToleredVar){
            toRemove.push_back(i);
            cRem++;
        }
        else if(ps.pairs[i].variance > lowVariance && ps.pairs[i].variance <= maxToleredVar){
            ps.pairs[i].constr = BoundType::SOFT;
            ps.pairs[i].kSpring = calculateSpringK(ps.pairs[i].variance);
            cSoft++;
        }
        else{
            ps.pairs[i].constr = BoundType::HARD;
            cHard++;
        }
    }

    for (uint i = 0; i < toRemove.size(); ++i) {
        ps.pairs.erase(ps.pairs.begin() + toRemove[i] - i);
    }


    prefs.intersectCnt = cInt;
    prefs.removedCnt = cRem;
    prefs.softCnt = cSoft;
    prefs.hardCnt = cHard;

    ps.orderPairsByBound();
}


//genera le coppie di atomi data una soglia di distanza
//(quadratico con num di atomi)
PairStatistics DynamicShape::generatePSThreshold(Scalar thresh){
    PairStatistics ps;
    for (uint i = 0; i < ball.size(); ++i) {
        for (uint j = i+1; j < ball.size(); ++j) {
            Scalar sqrDist = sqrDistance(ball[i].currPos, ball[j].currPos);
            if(sqrDist <= (thresh*thresh)){
                AtomPair ap(i,j);
                ap.constr = BoundType::NONE;
                ps.pairs.push_back(ap);
            }
        }

    }
    return ps;
}

void DynamicShape::rotateOnYAxis(Scalar force){

    for (uint i = 0; i < ball.size(); ++i) {
        qmol::Pos projected(barycenter.X(),ball[i].currPos.Y(),barycenter.Z());
        Scalar dist = projected.Norm();

        qmol::Vec initDir = ball[i].currPos - projected;
        qmol::Vec perpDir;
        if(force < 0){
            perpDir = qmol::Vec(-(initDir.Z()), initDir.Y(), initDir.X());
        }
        else{
            perpDir = qmol::Vec(initDir.Z(), initDir.Y(),-(initDir.X()));
        }

        perpDir = perpDir.normalized();
        perpDir *= std::sqrt(dist*0.001*std::abs(force));
        ball[i].currPos += perpDir;

    }
}

Pos projectOnAxis(Pos p, Vec a, Pos c){
    return c + a*dot(p-c, a);
}

void DynamicShape::rotateOnAxis(Scalar force, Vec axis){

    const Scalar R = prefs.R; // radius of sphere which moves exactly as the user intended
    const Scalar APPLICATION_DISTANCE = prefs.applicationDistance;
    //if > 0: molec manipulated from inside, outside follows
    //if = 0: mol moves rigidly (barring centrifugal forces)
    //if < 0: mol manipuladed from the outside, inside follows

    axis = axis.normalized();
    force *= 0.004;

    for (uint i = 0; i < ball.size(); ++i) {

        Pos& pos = ball[i].currPos;
        Pos& prev = ball[i].prevPos;
        Pos proj = projectOnAxis( pos, axis, barycenter);

        //pos += cross(axis,pos - proj)*force;


        Vec k = pos - proj;

        Scalar nonlin =  powf(dot(k,k)/(R*R), APPLICATION_DISTANCE);

        if(nonlin > 0.001){
            Vec displacement = cross(axis,k)*(force/nonlin);
            pos += displacement;

            // =========================
            // OPTIMIZED FOR trail = 1;
            // =========================
            /*
            Scalar& trail = prefs.trailing;
            if((displacement*(1-trail)).Norm() <= prefs.trailThres){ //se norm <= a soglia
                prev += displacement*trail;
            }
            else{
               Scalar f = (displacement.Norm() - prefs.trailThres)/(displacement*trail).Norm();
               prev += displacement*(trail*f);

            }
            */
            //OPTIMIZED FOR trail = 1
            prev += displacement;

        }
    }
}

void DynamicShape::updateBoundSphereCenter(){

    qmol::Pos max = ball[0].currPos;
    qmol::Pos min = ball[0].currPos;;
    for (uint i = 1; i < ball.size(); ++i) {

        if(ball[i].currPos.X() < min.X()){
            min.X() = ball[i].currPos.X();
        }
        else if(ball[i].currPos.X() > max.X()){
            max.X() = ball[i].currPos.X();
        }

        if(ball[i].currPos.Y() < min.Y()){
            min.Y() = ball[i].currPos.Y();
        }
        else if(ball[i].currPos.Y() > max.Y()){
            max.Y() = ball[i].currPos.Y();
        }

        if(ball[i].currPos.Z() < min.Z()){
            min.Z() = ball[i].currPos.Z();
        }
        else if(ball[i].currPos.Z() > max.Z()){
            max.Z() = ball[i].currPos.Z();
        }
    }

    boundSphereCenter = qmol::Pos((max.X()+ min.X())/2,
                                  (max.Y()+min.Y())/2,
                                  (max.Z()+min.Z())/2);

}

void DynamicShape::updateBarycenter(){
    qmol::Pos bc(0,0,0);
    for (uint i = 0; i < ball.size(); ++i){
        bc += ball[i].currPos;
    }
    barycenter = bc/ball.size();

}




