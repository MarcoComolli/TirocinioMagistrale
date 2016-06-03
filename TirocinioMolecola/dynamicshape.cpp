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

    for (unsigned int i = ps.softStartIdx; i <=  ps.softEndIdx ; ++i) {
            qmol::Vec force = calculateSpringForce(ps.pairs[i].kSpring,
                                                   ps.pairs.at(i).distance,
                                                   ps.pairs.at(i).atomID1,
                                                   ps.pairs.at(i).atomID2
                                                   );

            force *= dt*dt;
            //std::cout << "force: " << force.X() << "," << force.Y() << "," << force.Z() << std::endl;


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


/*//void DynamicShape::resolveCompenetration(const PairStatistics& ps){
//    //std::cout << "count: " << f << std::endl;
//    //AtomPair current;
//   // AtomPair a;
//   // for (uint i = 0; i < ball.size(); ++i) {
//        //std::cout << "i: " << i << std::endl;
//        //for (uint j = i; j < ball.size(); ++j) {
//            //std::cout << "j: " << j << std::endl;
//            //a.atomID1 = i;
//            //a.atomID2 = j;
//            //if( (current = ps.getAtomPair(a) ).atomID1 != -1){
//                //if(current.constr != BoundType::INTERSECT){
//                    //if(checkIntersect(i,j)){
//                    //    ball[i].currPos = ball[i].prevPos;
//                    //    ball[j].currPos = ball[j].prevPos;
//                   // }
//               // }
//            //}
//        }
//    }
//}
*/

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
            applyFixedConstrains(ps);
            errorDiff  = prevErr-error;
            i++;
        }
    }
    updateBarycenter();

}

//given atoms ID, normal spring lenght, and elastic const k calculate
//the force of the spring and return it
qmol::Vec DynamicShape::calculateSpringForce(Scalar k, Scalar springLength, int atomID, int atomIDPaired){

    qmol::Pos a1Pos = ball.at(atomID).currPos;
    qmol::Pos a2Pos = ball.at(atomIDPaired).currPos;

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

//controlla se due atomi si intersecano
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

    std::cout << "Intersect: "  << cInt << std::endl;
    std::cout << "Removed: "  << cRem << std::endl;
    std::cout << "Soft: "  << cSoft << std::endl;
    std::cout << "Hard: "  << cHard << std::endl;


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
    /* PSEUDOCODICE:
     * per ogni sfera
     *      calcola la distanza dall'asse
     *      calcola la direzione della forza
     *      calcola la forza del vettore in base alla distanza
     *      somma il vec trovato alla currPos
     */

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
    /* PSEUDOCODICE:
     * per ogni sfera
     *      calcola la distanza dall'asse
     *      calcola la direzione del vettore
     *      calcola la forza del vettore in base alla distanza
     *      somma il vec trovato alla currPos
     */

    const Scalar R = prefs.R; // radius of sphere which moves exactly as the user intended
    const Scalar APPLICATION_DISTANCE = prefs.applicationDistance;
    //if >0: molec manipulated from inside, outside follows
    //if = 0: mol moves rigidly (barring centrifugal forces)
    //if < 0: mol manipuladed from the outside, inside follows

    axis = axis.normalized();
    force *= 0.004;

    for (uint i = 0; i < ball.size(); ++i) {

        Pos& pos = ball[i].currPos;
        Pos& prev = ball[i].prevPos;
        Pos proj = projectOnAxis( pos, axis, barycenter);

        //pos += cross(axis,pos - proj)*force;


        Vec k = pos- proj;

        k /= R;
        Scalar nonlin =  powf(dot(k,k), APPLICATION_DISTANCE);

        if(nonlin > 0.001){
            Vec displacement = cross(axis,k*R)*(force/nonlin);
            pos += displacement;

            Scalar& trail = prefs.trailing;
            if((displacement*(1-trail)).Norm() <= prefs.trailThres){ //se norm <= a soglia
                prev += displacement*trail;
            }
            else{
               Scalar f = (displacement.Norm() - prefs.trailThres)/(displacement*trail).Norm();
               prev += displacement*(trail*f);

            }
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




