#include "myrenderer.h"
#include <GL/glew.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
//#include <ogldev_math_3d.h>
//#include <ogldev_util.h>
//#include <ogldev_pipeline.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <vector>
#include <windows.h>
#include <bitset>
//#include <vec3.hpp>
//#include <glm.hpp>
//#include <gtc/matrix_transform.hpp>
#include <set>
#include <AntTweakBar.h>

#include <vcg/math/quaternion.h>

#include "qutemolLib/baseTypes.h"


#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768


GLuint VBO;
GLuint VBOColor;
GLuint VBONormals;
GLuint VBOLines;
GLuint VBOColorLines;
GLuint VBOGrid;


// basetype.cpp
#include <vcg/math/quaternion.h>

namespace qmol{

typedef vcg::Quaternion<Scalar> Quat;

Vec4 quatMult(const Vec4 &a, const Vec4 &b){
    Quat res = Quat(a)*Quat(b);
    return Vec4(res);
}


Vec4 quatAxisAngle(const Vec &a, Scalar b){
    Quat res;
    res.FromAxis( b, a );
    return Vec4( res );
}

Matrix fromQuat( const Vec4& quat ){

    Quat q( quat );
    Matrix res;
    q.ToMatrix(res);
    return res;

}

}

const char* pVSFilePath = "F:/Documenti - Marco/Documenti/Universita/Tirocinio Magistrale/qt_workspace/TirocinioMolecola/shader.vs.glsl";
const char* pFSFilePath = "F:/Documenti - Marco/Documenti/Universita/Tirocinio Magistrale/qt_workspace/TirocinioMolecola/shader.fs.glsl";

//testing

GLuint gScaleLocation;

GLuint matrixID;
GLuint viewMatrixID;
GLuint modelMatrixID;

GLuint lightID;

GLuint lineColor;


GLuint shaderProg;




MyRenderer::MyRenderer(){

}


using namespace glm;
using namespace qmol;

static string matrixToStr(Matrix m){
    stringstream ss;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            ss << m[i][j] << "\t";
        }
        ss << "\n";
    }
    return ss.str();
}

static vector<vec3> generateCube(Scalar posX, Scalar posY, Scalar posZ, Scalar border){

    vector<vec3> cube;
    Scalar half = border/2;

    //vertices

    vec3 v1(posX-half,posY-half,posZ+half);
    vec3 v2(posX+half,posY-half,posZ+half);
    vec3 v3(posX+half,posY+half,posZ+half);
    vec3 v4(posX-half,posY+half,posZ+half);
    vec3 v5(posX-half,posY-half,posZ-half);
    vec3 v6(posX+half,posY-half,posZ-half);
    vec3 v7(posX+half,posY+half,posZ-half);
    vec3 v8(posX-half,posY+half,posZ-half);

    //front
    cube.push_back(v1); //1
    cube.push_back(v2); //2
    cube.push_back(v3); //3
    cube.push_back(v4); //4

    //back
    cube.push_back(v5); //5
    cube.push_back(v8); //8
    cube.push_back(v7); //7
    cube.push_back(v6); //6



    //right
    cube.push_back(v2);
    cube.push_back(v6);
    cube.push_back(v7);
    cube.push_back(v3);


    //left
    cube.push_back(v1);
    cube.push_back(v4);
    cube.push_back(v8);
    cube.push_back(v5);



    //top
    cube.push_back(v4);
    cube.push_back(v3);
    cube.push_back(v7);
    cube.push_back(v8);

    //Bottom
    cube.push_back(v1);
    cube.push_back(v5);
    cube.push_back(v6);
        cube.push_back(v2);

    return cube;
}

static void updateVBO(GLuint* buffer, vector<vec3> data){
    vec3* arr = new vec3[data.size()];
    //vec3 arr[data.size()];
    std::copy(data.begin(),data.end(),arr);

    //update
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3)*data.size(), &data[0]);
    delete(arr);
}

static void generateVBO(GLuint* buffer,vector<vec3> data){

     //genera buffer
    glGenBuffers(1, buffer);  //#1, numero di oggetti da creare
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*data.size(), &data[0], GL_STATIC_DRAW);
}

static void AddShader(GLuint shaderProgram, const char* pShaderText, GLenum shaderType){

    GLuint shaderObj = glCreateShader(shaderType);

    if (shaderObj == 0) {
        throw std::runtime_error("Error creating shader" + shaderObj);
    }


    const GLchar* p[1];
    p[0] = pShaderText;
    GLint lengths[1];
    lengths[0]= strlen(pShaderText);


    glShaderSource(shaderObj, 1, p, lengths);

    glCompileShader(shaderObj);

    GLint success;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(shaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, InfoLog);
        exit(1);
    }

    glAttachShader(shaderProgram, shaderObj);

}

static void CompileShaders(){


    GLuint shaderProgram = glCreateProgram();

    if (shaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }


    ifstream inFile;

    inFile.open(pVSFilePath);//open the input file

    if ( (inFile.rdstate() & std::ifstream::failbit ) != 0 )
        std::cerr << "Error opening file1\n";


    stringstream strStream;
    strStream << inFile.rdbuf();//read the file
    string vs = strStream.str();//vs holds the content of the file

    inFile.close();


    inFile.open(pFSFilePath);//open the input file

    if ( (inFile.rdstate() & std::ifstream::failbit ) != 0 )
        std::cerr << "Error opening file2\n";


    strStream.str("");
    strStream << inFile.rdbuf();//read the file
    string fs = strStream.str();//str holds the content of the file

    inFile.close();



    AddShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    AddShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(shaderProgram);



    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program1: '%s'\n", ErrorLog);
        exit(1);
    }





    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program3: '%s'\n", ErrorLog);
        exit(1);
    }



    glUseProgram(shaderProgram);

    //   gScaleLocation = glGetUniformLocation(shaderProgram, "gScale"); //riferimento alla variabile gscale
    //   assert(gScaleLocation != 0xFFFFFFFF);



    matrixID = glGetUniformLocation(shaderProgram, "MVP");
    //assert(matrixID != 0xFFFFFFFF);

    viewMatrixID = glGetUniformLocation(shaderProgram, "V");
    //assert(viewMatrixID != 0xFFFFFFFF);

    modelMatrixID = glGetUniformLocation(shaderProgram, "M");
    //assert(modelMatrixID != 0xFFFFFFFF);

    lightID = glGetUniformLocation(shaderProgram, "LightPositionWorld");
    //assert(lightID != 0xFFFFFFFF);

    lineColor = glGetUniformLocation(shaderProgram, "lineColor");
    //assert(lineColor != 0xFFFFFFFF);

    shaderProg = shaderProgram;
}

static vec3 cross(vec3 v1, vec3 v2){


    return vec3(v1.y*v2.z - v1.z*v2.y,
                v1.z*v2.x - v1.x*v2.z,
                v1.x*v2.y - v1.y*v2.x);
}

static vec3 normalize(vec3 v){
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return v/len;
}

static vector<vec3> generateCubeNormals(vector<vec3> cubes){

    vector<vec3> faceNormals;

    //0 front
    vec3 v1 = cubes[1] - cubes[0];
    vec3 v2 = cubes[3] - cubes[0];
    faceNormals.push_back(normalize(cross(v1,v2)));

    //1 right
    v1 = cubes[5] - cubes[1];
    v2 = cubes[2] - cubes[1];
    faceNormals.push_back(normalize(cross(v1,v2)));

    //2 top
    v1 = cubes[2] - cubes[3];
    v2 = cubes[7] - cubes[3];
    faceNormals.push_back(normalize(cross(v1,v2)));

    //3 bottom
    v1 = cubes[0] - cubes[1];
    v2 = cubes[5] - cubes[1];
    faceNormals.push_back(normalize(cross(v1,v2)));

    //4 left
    v1 = cubes[0] - cubes[4];
    v2 = cubes[7] - cubes[4];
    faceNormals.push_back(normalize(cross(v1,v2)));

    //5 back
    v1 = cubes[4] - cubes[5];
    v2 = cubes[6] - cubes[5];
    faceNormals.push_back(normalize(cross(v1,v2)));

//    for (unsigned int i = 0; i < faceNormals.size(); ++i) {
//        std::cout << "normal: "  << i << ": " << faceNormals[i].x << "," <<
//                     faceNormals[i].y << "," << faceNormals[i].z << std::endl;
//    }


    vector<vec3> vertexNormals;

    for ( unsigned int j = 0; j < cubes.size()/24; ++j) {



    //front
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(0));
    }

    //back
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(5));
    }

    //right
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(1));
    }

    //left
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(4));
    }

    //top
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(2));
    }

    //bottom
    for (int i = 0; i < 4; ++i) {
        vertexNormals.push_back(faceNormals.at(3));
    }

     }

    //senza ripetizioni di vertici
//    vertexNormals.push_back(normalize(faceNormals[0] + faceNormals[4] + faceNormals[3]));
//    vertexNormals.push_back(normalize(faceNormals[0] + faceNormals[1] + faceNormals[3]));
//    vertexNormals.push_back(normalize(faceNormals[0] + faceNormals[1] + faceNormals[2]));
//    vertexNormals.push_back(normalize(faceNormals[0] + faceNormals[2] + faceNormals[4]));

//    vertexNormals.push_back(normalize(faceNormals[3] + faceNormals[4] + faceNormals[5]));
//    vertexNormals.push_back(normalize(faceNormals[1] + faceNormals[3] + faceNormals[5]));
//    vertexNormals.push_back(normalize(faceNormals[1] + faceNormals[2] + faceNormals[5]));
//    vertexNormals.push_back(normalize(faceNormals[2] + faceNormals[4] + faceNormals[5]));

//    for (unsigned int i = 0; i < vertexNormals.size(); ++i) {
//        std::cout << "vert norm: "  << i << ": " << vertexNormals[i].x << "," <<
//                     vertexNormals[i].y << "," << vertexNormals[i].z << std::endl;
//    }



    return vertexNormals;

}

void MyRenderer::init(){
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        glewGetErrorString(res);
        throw std::runtime_error("A GLEW error occurred");
    }

    generateBuffers(ds.prefs);

    glClearColor(0.17f, 0.20f, 0.20f, 0.0f);


    std::cout << "GL VERSION: " << glGetString(GL_VERSION) << endl;

    CompileShaders();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);


    //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);



}

qmol::Matrix 	perspective(
        Scalar fovy,
        Scalar aspect,
        Scalar zNear,
        Scalar zFar
        )
{
    Scalar const tanHalfFovy = tan(fovy / 2 );

    qmol::Matrix Result;
    Result.SetZero();

    Result.ElementAt(0,0) = 1 / (aspect * tanHalfFovy);
    Result.ElementAt(1,1) = 1 / (tanHalfFovy);
    Result.ElementAt(2,2) = - (zFar + zNear) / (zFar - zNear);
    Result.ElementAt(2,3) = - 1;
    Result.ElementAt(3,2) = - (2 * zFar * zNear) / (zFar - zNear);
    return Result;
}

void sendMatrix(GLuint unif, qmol::Matrix &m ) {
    float tmp[16];
    const qmol::Scalar * d = &(m[0][0]);
    for (int i=0; i<16; i++) tmp[i] = (float) d[i];
    glUniformMatrix4fv(unif, 1, GL_TRUE, tmp);
}

void MyRenderer::printGrid()
{
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBOGrid);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //specifica gli attributi nel vbo

    glDrawArrays(GL_LINES, 0, nGrid);
    glDisableVertexAttribArray(0);
}

void MyRenderer::printLines(){

    //glenable
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VBOLines);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //specifica gli attributi nel vbo

    glBindBuffer(GL_ARRAY_BUFFER, VBOColorLines);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_LINES, 0, nLines); //disegna il vbo!

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

}

void MyRenderer::printCubes(){
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //specifica gli attributi nel vbo
    //#1,index of attribute
    //#2, numero componenti attributo

     glEnableVertexAttribArray(1);

     glBindBuffer(GL_ARRAY_BUFFER, VBOColor);

     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

     glEnableVertexAttribArray(2);

     glBindBuffer(GL_ARRAY_BUFFER, VBONormals);

     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);


    glDrawArrays(GL_QUADS, 0, nElements*24); //disegna il vbo!
    //#2, indice da dove iniziare
    //#3, elementi da disegnare


    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void MyRenderer::render(){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProg);

    updateBuffers(ps);

    qmol::Matrix model;
    qmol::Matrix view;

    view.SetIdentity();

    Vec scale(0.02,0.02,0.001);

     model.SetTranslate(-ds.barycenter);

//   model.SetTranslate(-(ds.barycenter.X()/scale.X()),
//                       -(ds.barycenter.Y()/scale.Y()),
//                       -(ds.barycenter.Z()/scale.Z()));


    model *= qmol::fromQuat( quaternion );

    // todo: centare sulla molecola
    //model.SetTranslate( panX,panY,panZ);

    // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    //qmol::Matrix projection = perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
    qmol::Matrix projection;
    projection.SetScale(scale);


    qmol::Matrix MVP = projection * view * model ;

    //MVP.SetTranslate(-ds.barycenter);
    //std::cout << "MVP " << std::endl << matrixToStr(MVP) << std::endl;


    sendMatrix( matrixID, MVP ); 
    sendMatrix( modelMatrixID, model );
    sendMatrix( viewMatrixID, view );

    vec3 lightPos = vec3(9.55,17.5,0.5);
    glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);

    //printCubes();
    //printLines();

    if(ds.prefs.showGrid){
        printGrid();
    }

}

static vector<vec3> generateColors(vector<vec3> cubesVertices){
    vector<vec3> cols;
    vec3 color;

    Scalar r = 1.0, g = 0.5, b = 0.0;
    Scalar tick = 1.0f/(cubesVertices.size()/16);

    //ogni due cubi cambia colore dei vertici
    for (unsigned int i = 0; i < cubesVertices.size()/16; ++i) {
        r -= tick;
        //g -= 0.015;
        b += tick;


        for (int j = 0; j < 16; ++j) {
            color.x = r;
            color.y = g;
            color.z = b;

            //std::cout << "color: " << rf << "," << gf << "," << bf << std::endl;
            cols.push_back(color);
        }
    }

    return cols;


}

static vector<vec3> generateGrid(SpatialGrid grid){
    vector<vec3> limits;

    Scalar xl = grid.gridX[0];
    Scalar yl = grid.gridY[0];
    Scalar zl = grid.gridZ[0];
    Scalar xh = grid.gridX[grid.gridX.size()-1];
    Scalar yh = grid.gridY[grid.gridY.size()-1];
    Scalar zh = grid.gridZ[grid.gridZ.size()-1];



    //righe
    vec3 p1;
    vec3 p2;
    for (uint z = 0; z < grid.gridZ.size(); z++){
       for(uint  y = 0; y < grid.gridY.size(); y++) {
           p1 = vec3(xl, yl + y*grid.cellDimY, zl+ z*grid.cellDimZ);
           p2 = vec3(xh, yl + y*grid.cellDimY, zl+ z*grid.cellDimZ);
           limits.push_back(p1);
           limits.push_back(p2);
        }
    }


    for (uint z = 0; z < grid.gridZ.size(); z++){
       for(uint  x = 0; x < grid.gridX.size(); x++) {
           p1 = vec3(xl + x*grid.cellDimX, yl, zl + z*grid.cellDimZ);
           p2 = vec3(xl + x*grid.cellDimX, yh, zl + z*grid.cellDimZ);
           limits.push_back(p1);
           limits.push_back(p2);
        }
    }


    for (uint x = 0; x < grid.gridX.size(); x++){
       for(uint  y = 0; y < grid.gridY.size(); y++) {
           p1 = vec3(xl + x*grid.cellDimX, yl + y*grid.cellDimY, zl);
           p2 = vec3(xl + x*grid.cellDimX, yl + y*grid.cellDimY, zh);
           limits.push_back(p1);
           limits.push_back(p2);
        }
    }

    std::cout << "limits size "  <<limits.size() << std::endl;
    return limits;

}

static vector<Scalar> normalizedVariances(vector<Scalar> variances){
    //Find min and max
   Scalar min = *std::min_element(variances.begin(),variances.end());
   Scalar max = *std::max_element(variances.begin(),variances.end());


   vector<Scalar> normalized;
   Scalar slope = 1.0f/(max-min);

   Scalar tmp;
   for (unsigned int i = 0; i < variances.size(); ++i) {
       tmp = slope*(variances[i]-min);
       normalized.push_back(tmp);
   }


   return normalized;



}

static vector<vec3> generateColorsLines(vector<BoundType> bounds ){
    vector<vec3> cols;
    vec3 color;

    Scalar r = 0, g = 0, b = 0;



    for (unsigned int i = 0; i < bounds.size(); ++i) {
        if(bounds[i] == BoundType::HARD){ //verde
            g = 1;
            r = 0;
            b = 0;
        }
        else if(bounds[i] == BoundType::INTERSECT){ //bianco
            g = 1;
            r = 1;
            b = 1;
        }
        else if(bounds[i] == BoundType::SOFT){ //viola
            g = 0;
            r = 1;
            b = 1;
        }

        for (int j = 0; j < 2; ++j) {
            color.x = r;
            color.y = g;
            color.z = b;
            cols.push_back(color);
        }
    }

    return cols;

}

//per test
static void uniq(vector<int> v){
    set<int> a;
    for (uint i = 0; i < v.size(); ++i) {
        a.insert(v[i]);
    }
    std::cout << "Unique check: "  << a.size() << " of " << v.size() <<std::endl;
}

vector<bool> getBoundIndexesForRendering(Preferences p, const PairStatistics & ps){
    bitset<3> bits(0);
    if(p.showIntersectBonds){bits.set(0);} //001
    if(p.showHardBonds){bits.set(1);}       //010
    if(p.showSoftBonds){bits.set(2);}       //100

    vector<bool> result;


    short s = (short)bits.to_ulong();

    switch (s) {
    case 0:
        result.push_back(false); //intersect
        result.push_back(false); //hard
        result.push_back(false); //soft
        return result;
    case 1:
        result.push_back(true); //intersect
        result.push_back(false); //hard
        result.push_back(false); //soft
        return result;
    case 2:
        result.push_back(false); //intersect
        result.push_back(true); //hard
        result.push_back(false); //soft
        return result;
    case 3:
        result.push_back(true); //intersect
        result.push_back(true); //hard
        result.push_back(false); //soft
        return result;
    case 4:
        result.push_back(false); //intersect
        result.push_back(false); //hard
        result.push_back(true); //soft
        return result;
    case 5:
        result.push_back(true); //intersect
        result.push_back(false); //hard
        result.push_back(true); //soft
        return result;
    case 6:
        result.push_back(false); //intersect
        result.push_back(true); //hard
        result.push_back(true); //soft
        return result;
    case 7:
        result.push_back(true); //intersect
        result.push_back(true); //hard
        result.push_back(true); //soft
        return result;
    default:
        return result;
    }
}

void MyRenderer::generateBuffers(Preferences prefs){

    std::cout << "--INIZIO GENERATE--" << std::endl;
    vector<vec3> centers;
    vector<vec3> centersNormalized;
    vector<vec3> cubes;
    vector<vec3> lines;
    vector<BoundType> bounds;
 //   vector<Scalar> variances;
 //   vector<Scalar> variancesNorm;
    vector<bool> renderCentersSelector(ds.ball.size());

    std::cout << "size ball " << ds.ball.size() << std::endl;



    for (uint i = 0; i < ds.ball.size(); ++i) {
        renderCentersSelector[i] = false;
    }


    vec3 v1,v2;
    Scalar radius = 1.5;

    int c1 = 0, c2 = 0;
    int a1,a2;
    DynamicBall b1,b2;
    vector<bool> showBonds = getBoundIndexesForRendering(prefs,ps);


     glUniform3f(lineColor,0.5,1.0,1.0);
    if (showBonds[0]) {
        for (int i = ps.intersectStartIdx; i <= ps.intersectEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            //variances.push_back(ps.pairs[i].variance);
           // std::cout << "var: " << ps.pairs[i].variance << std::endl;
            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
                c1++;
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
                c2++;
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);
        }
    }

    glUniform3f(lineColor,0.0,1.0,0.0);
    if (showBonds[1]) {
        for (int i = ps.hardStartIdx; i <= ps.hardEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            //variances.push_back(ps.pairs[i].variance);
           // std::cout << "var: " << ps.pairs[i].variance << std::endl;
            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
                c1++;
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
                c2++;
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);
        }
    }

    glUniform3f(lineColor,0.5,1.0,0.5);
    if (showBonds[2]) {
        for (int i = ps.softStartIdx; i <= ps.softEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            //variances.push_back(ps.pairs[i].variance);
           // std::cout << "var: " << ps.pairs[i].variance << std::endl;
            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
                c1++;
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
                c2++;
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);
        }
    }


    nElements = centers.size();
    nLines = lines.size();


    centersNormalized = centers;//normalizeCenters(centers, radius);
    //variancesNorm = normalizedVariances(variances);

    vector<vec3> temp;

    vector<vec3> colors;
    vector<vec3> colorLines;
    vector<vec3> normals;
    vector<vec3> grid;



//    for (uint i = 0; i < centersNormalized.size(); ++i) {
//        temp = generateCube(centersNormalized[i].x,centersNormalized[i].y,centersNormalized[i].z,radius);
//        cubes.insert(cubes.end(),temp.begin(), temp.end());
//    }


//    colors = generateColors(cubes);
    colorLines = generateColorsLines(bounds);
//    normals = generateCubeNormals(cubes);
    grid = generateGrid(this->grid);

    nGrid = grid.size();


//    for (unsigned int i = 0; i < cubes.size(); ++i) {
//        std::cout << "cubo " << i << ": " << cubes.at(i).x << " " << cubes.at(i).y << " " << cubes.at(i).z << std::endl;
//    }

//    generateVBO(&VBO,cubes);
//    generateVBO(&VBOColor,colors);
//    generateVBO(&VBONormals,normals);
    generateVBO(&VBOLines,lines);
    generateVBO(&VBOColorLines,colorLines);
    generateVBO(&VBOGrid,grid);

std::cout << "--FINE GENERATE--" << std::endl;

}

vec3 randVec(){
    return vec3(
                (rand()%100)/50.0 -1 ,
                (rand()%100)/50.0 -1,
                (rand()%100)/50.0 -1
                )  ;
}

void MyRenderer::updateBuffers(PairStatistics ps){
    vector<vec3> centers;
    //vector<vec3> centersNormalized;
    vector<vec3> cubes;
    vector<vec3> lines;
    vector<Scalar> variances;
    vector<Scalar> variancesNorm;
    vector<BoundType> bounds;

    vector<bool> renderCentersSelector(ds.ball.size());

    for (uint i = 0; i < ds.ball.size(); ++i) {
        renderCentersSelector[i] = false;
    }

    vec3 v1,v2;
    Scalar radius = 0.5;

    int a1,a2;
    DynamicBall b1,b2;


    vector<bool> showBonds = getBoundIndexesForRendering(ds.prefs,ps);



    glUniform3f(lineColor,0.6,0.6,0.6);
    if (showBonds[0]) {
        for (int i = ps.intersectStartIdx; i <= ps.intersectEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);

            glBegin(GL_LINES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glEnd();
        }
    }

    glUniform3f(lineColor,0.0,6.0,0.0);
    if (showBonds[1]) {
        for (int i = ps.hardStartIdx; i <= ps.hardEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);

            glBegin(GL_LINES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glEnd();
        }
    }

    glUniform3f(lineColor,0.6,0.0,0.6);
    if (showBonds[2]) {
        for (int i = ps.softStartIdx; i <= ps.softEndIdx; ++i) {
            a1 =  ps.pairs[i].atomID1;
            a2 =  ps.pairs[i].atomID2;
            b1 = ds.ball.at(a1);
            b2 = ds.ball.at(a2);
            v1 = vec3(b1.currPos.X(), b1.currPos.Y(), b1.currPos.Z());
            v2 = vec3(b2.currPos.X(), b2.currPos.Y(), b2.currPos.Z());

            if(renderCentersSelector.at(a1) == false){
                renderCentersSelector.at(a1) = true;
                centers.push_back(v1);
            }
            if(renderCentersSelector.at(a2) == false){
                renderCentersSelector.at(a2) = true;
                centers.push_back(v2);
            }
            lines.push_back(v1);
            lines.push_back(v2);
            bounds.push_back(ps.pairs[i].constr);
            glBegin(GL_LINES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glEnd();
        }
    }

    nElements = centers.size();

     nLines = lines.size();

    //variancesNorm = normalizedVariances(variances);

    vector<vec3> temp;
    vector<vec3> colors;
    vector<vec3> colorLines;
    vector<vec3> normals;

//    for (uint i = 0; i < centers.size(); ++i) {
//        temp = generateCube(centers[i].x,centers[i].y,centers[i].z,radius);
//        cubes.insert(cubes.end(),temp.begin(), temp.end());
//    }

//    colors = generateColors(cubes);
    colorLines = generateColorsLines(bounds);
//    normals = generateCubeNormals(cubes);


    //updateVBO(&VBO,cubes);
    //updateVBO(&VBOColor,colors);
    //updateVBO(&VBONormals,normals);
    updateVBO(&VBOLines,lines);
    updateVBO(&VBOColor,colorLines);


}

void MyRenderer::resetViewMat(){
     panX = 0.0;
     panY = 0.0;
     panZ = 0.0;
     zoom = 11.0;
     zoom1 = 2.0;

}

void MyRenderer::rotateView(qmol::Vec axis, qmol::Scalar angle){
    quaternion = qmol::quatMult(  qmol::quatAxisAngle(axis,angle) , quaternion );
}

void MyRenderer::impressUserRotation(qmol::Vec axis, qmol::Scalar angle){

    Scalar angleView = angle*0.0;
    Scalar anglePhys = angle*1.0;


    rotateView( axis, -angleView  *  0.0025 );

    //ds.updateCenter();

    for (int i = 0; i < 10; ++i) {
        ds.rotateOnAxis(anglePhys/10, axis);
    }

}


