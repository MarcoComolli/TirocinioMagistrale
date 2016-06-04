#ifndef MYRENDERER_H
#define MYRENDERER_H

#include <atompair.h>
#include <qutemolLib/shape.h>
#include <ogldev_math_3d.h>
#include <vec3.hpp>
#include <dynamicshape.h>
#include <spatialgrid.h>

using namespace qmol;
class MyRenderer
{
public:

    DynamicShape ds;
    PairStatistics ps;
    SpatialGrid grid;



    MyRenderer();

    void init();

    void generateBuffers(Preferences prefs);
    void updateBuffers(PairStatistics ps);
    vector<glm::vec3> normalizeCenters(vector<glm::vec3> cubes, float radius);

    void render();
    void resetViewMat();

    void impressUserRotation(qmol::Vec axis, qmol::Scalar angle);

    Scalar panX = 0.0;
    Scalar panY = 0.0;
    Scalar panZ = 0.0;
    Scalar zoom = 11.0;
    Scalar zoom1 = 2.0;
    qmol::Vec4 quaternion = qmol::Vec4(0,0,0,1);

    int nElements;
    int nLines;
    int nGrid;

    qmol::Matrix initMVP(qmol::Matrix model, qmol::Matrix view);
    
    void printGrid();
    void printLines();

    void printCubes();

private:
    void rotateView(qmol::Vec axis, qmol::Scalar angle);

};

#endif // MYRENDERER_H
