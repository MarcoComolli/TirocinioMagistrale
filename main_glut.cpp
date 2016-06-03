// quali ci vogliono davvero di questi include?
#include <iostream>
#include <fstream>
#include <vector>
#include <qutemolLib/molecule.h>
#include <qutemolLib/shape.h>
#include <qutemolLib/shapingOptions.h>
#include <qutemolLib/atomTable.h>
#include <atomposition.h>
#include <moleculeconnections.h>
#include <atompair.h>
#include <myrenderer.h>
#include <AntTweakBar.h>
#include <preferences.h>


#include <GL/glut.h>

#define WINDOW_WIDTH 922
#define WINDOW_HEIGHT 691

extern MyRenderer renderer;
void processKey(unsigned char);
void processMouse(int,int,int,int,int);
void setSpecKeyStatus(int);
void initComponents(Preferences prefs);
void testSprings(Preferences prefs);
void mouseMotion(int x, int y);

void idleCallback(){
    renderer.ds.doPhysicsStep(1.0f/30.0f,renderer.ps);
    glutPostRedisplay();



}

void displayCallback(){
    renderer.render();
    glutSwapBuffers();
}

static void mouseCallback(int btn, int state, int x, int y){
    processMouse(btn,state,x,y,glutGetModifiers());

}

static void keyboardCallback(unsigned char key, int xmouse, int ymouse){
    processKey(key);
    glutPostRedisplay();
    TwEventKeyboardGLUT(key,xmouse,ymouse);
}

static void mouseMotionCallback(int x, int y){
    mouseMotion(x,y);
}

static void reshapeCB(int w, int h){
    glViewport(0,0,w,h);
    TwWindowSize(w, h);
}

static void terminateGlut(){
    std::cout << "TERMINATO GLUT" << std::endl;
    TwTerminate();
}

void TW_CALL reloadAll(void * prefs){
    initComponents(*(Preferences *)(prefs));
    renderer.quaternion = Vec4(0,0,0,1);
    renderer.init(*(Preferences *)(prefs));
}

void TW_CALL applyPrefs(void * prefs){
    renderer.init(*(Preferences *)(prefs));
}

void initTweakBar(Preferences prefs)
{
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    TwBar *myBar;


    myBar = TwNewBar("Parameters");
    TwDefine(" Parameters size='200 400' color='15 225 15' "); // change default tweak bar size and color

    TwAddVarRW(myBar, "Extra bonds", TW_TYPE_INT32, &prefs.numberExtraBonds , " min=0 max=500000 step=100 keyIncr=z keyDecr=x help=' '");

    TwAddVarRW(myBar, "Damp", TW_TYPE_FLOAT, &prefs.damp, " min=0.0 max=1.0 step=0.001 keyIncr=a keyDecr=s help=' '");
    TwAddVarRW(myBar, "Trail", TW_TYPE_FLOAT, &prefs.trailing, " min=0.0 max=1.0 step=0.01 keyIncr=i keyDecr=u help=' '");
    TwAddVarRW(myBar, "TrailThresh", TW_TYPE_FLOAT, &prefs.trailThres, " min=0.0 max=20.0 step=0.01 keyIncr=h keyDecr=j help=' '");
    TwAddVarRW(myBar, "Min variance", TW_TYPE_FLOAT, &prefs.minVarianceTrhesh, " min=0.0 max=5.0 step=0.01 keyIncr=o keyDecr=p help=' '");
    TwAddVarRW(myBar, "Max variance", TW_TYPE_FLOAT, &prefs.maxVarianceTrhesh, " min=0.0 max=20.0 step=0.01 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "R", TW_TYPE_FLOAT, &prefs.R, " min=0.0 max=200.0 step=0.1 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "APPLICATION DISTANCE", TW_TYPE_FLOAT, &prefs.applicationDistance, " min=-10.0 max=10.0 step=0.01 keyIncr=k keyDecr=l help=' '");

    TwAddSeparator(myBar,"sep0",NULL);

    TwAddVarRW(myBar, "UseHARD", TW_TYPE_BOOL8, &prefs.hardBonds, "label='Use HARD' key=8 help='Process hard bonds.'");
    TwAddVarRW(myBar, "UseSOFT", TW_TYPE_BOOL8, &prefs.softBonds, "label='Use SOFT' key=9 help='Process soft bonds.'");

    TwAddSeparator(myBar,"sep1",NULL);

    TwAddVarRW(myBar, "ShowINTERSECT", TW_TYPE_BOOL8, &prefs.showIntersectBonds, "label='Show INTERSECT' key=1 help='Show intersect bonds.'");
    TwAddVarRW(myBar, "ShowHARD", TW_TYPE_BOOL8, &prefs.showHardBonds, "label='Show HARD' key=2 help='Show hard bonds.'");
    TwAddVarRW(myBar, "ShowSOFT", TW_TYPE_BOOL8, &prefs.showSoftBonds, "label='Show SOFT' key=3 help='Show soft bonds.'");

    TwAddSeparator(myBar,"sep2",NULL);

    TwAddVarRW(myBar, "Showgrid", TW_TYPE_BOOL8, &prefs.showGrid, "label='Show grid' key=space help='Show the grid'");

    TwAddSeparator(myBar,"sep3",NULL);

    //da aggiustare
    //TwAddButton(myBar,"Aply!", applyPrefs, &prefs, NULL);

    TwAddButton(myBar,"Reload!", reloadAll, &prefs, NULL);



}

int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA |GLUT_DOUBLE );
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(5, 5);
    glutCreateWindow("Animated Mol");

    Preferences prefs;
    initComponents(prefs);
    // Check if glew is ok
    renderer.init(prefs);

    glutReshapeFunc(reshapeCB);

    glutDisplayFunc(displayCallback); //funzione invocata quando c'è del rendering
    //da fare
    glutIdleFunc(idleCallback); //funzione quando non c'è necessità
    //di rendering
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);


    //Preferences* prefs2 = new Preferences();
    initTweakBar(prefs);



    atexit(terminateGlut); //Funzione chiamata quando termina il main loop

    glutMainLoop();




}
