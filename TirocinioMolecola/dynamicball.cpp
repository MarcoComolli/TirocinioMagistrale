#include "dynamicshape.h"


DynamicBall::DynamicBall(){}

DynamicBall::DynamicBall(const qmol::Ball &ball, int id){

    col = ball.col;
    currPos = ball.pos;
    prevPos = ball.pos;
    rad = ball.rad;
    ballID = id;

}
