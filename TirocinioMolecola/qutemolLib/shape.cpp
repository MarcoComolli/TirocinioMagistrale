#include <vcg/space/box3.h>
#include "shape.h"
#include "log.h"

namespace qmol {

Shape::Shape() {
}


Tube::Tube(const Pos &a, const Pos &b){
    pos = (a+b)/2.0;
    len = (a-b)/2.0;
}

bool operator < (const Ball& a,const Ball& b){
    return a.pos < b.pos;
}

void Shape::shuffle(){
    std::sort( ball.begin(), ball.end() );
    /*for (uint i=1; i<ball.size(); i++) {
        std::swap( ball[i], ball[rand()%i]);
    }*/
}

void Shape::updateBoundingSphere(){
    vcg::Box3<Scalar> box;

    for (uint i=0; i<ball.size(); i++)
        box.Add( ball[i].pos );

    center = box.Center();
    radius = 0;

    for (uint i=0; i<ball.size(); i++)
        radius = std::max(
            radius,
            vcg::Distance( ball[i].pos, center) + ball[i].rad
        );

}

/**
 * Svuota i vettori interni allo shape (tube e ball)
 */
void Shape::clear(){
	//center = Pos(0,0,0);
	//radius = 1.0;
	tube.clear();
	ball.clear();
}

} // namespace qmol
