// quali ci vogliono davvero di questi include?
#include <iostream>
#include <fstream>
#include <vector>
#include <qutemolLib/molecule.h>
#include <qutemolLib/shape.h>
#include <qutemolLib/shapingOptions.h>
#include <qutemolLib/atomTable.h>
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
void initComponents(Preferences& prefs);
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
    initComponents(*(Preferences *) (prefs));
    renderer.quaternion = Vec4(0,0,0,1);
    renderer.init();
}

void TW_CALL SetCallback(const void *value, void *clientData)
{
    //std::cout << "SET CALLBACK!" << std::endl;
    (*(Preferences *)(clientData)).softBonds = *(const bool *)value;  // for instance
}

void TW_CALL GetCallback(void *value, void *clientData)
{
     //   std::cout << "GET CALLBACK!" << std::endl;
    *(bool *)value = (*(Preferences *)(clientData)).softBonds;  // for instance
}

void initTweakBar(DynamicShape& ds){
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    TwBar *myBar;

    myBar = TwNewBar("Parameters");
    TwDefine(" Parameters size='200 420' color='0 170 0' text=light"); // change default tweak bar size and color
    TwDefine(" Parameters alpha = 30 valueswidth=60 iconalign=horizontal iconpos=tl ");
    TwDefine(" GLOBAL iconmargin='5 5' ");
    TwDefine(" GLOBAL fontsize=2 fontstyle=default contained=true buttonalign=left ");

    TwAddVarRW(myBar, "Extra bonds", TW_TYPE_INT32, &ds.prefs.numberExtraBonds , " min=0 max=500000 step=100 keyIncr=z keyDecr=x help=' '");

    TwAddVarRW(myBar, "Damp", TW_TYPE_FLOAT, &ds.prefs.damp, " min=0.0 max=1.0 step=0.001 keyIncr=a keyDecr=s help=' '");
    TwAddVarRW(myBar, "Trail", TW_TYPE_FLOAT, &ds.prefs.trailing, " min=0.0 max=1.0 step=0.01 keyIncr=i keyDecr=u help=' '");
    TwAddVarRW(myBar, "TrailThresh", TW_TYPE_FLOAT, &ds.prefs.trailThres, " min=0.0 max=20.0 step=0.01 keyIncr=h keyDecr=j help=' '");
    TwAddVarRW(myBar, "Min variance", TW_TYPE_FLOAT, &ds.prefs.minVarianceTrhesh, " min=0.0 max=5.0 step=0.01 keyIncr=o keyDecr=p help=' '");
    TwAddVarRW(myBar, "Max variance", TW_TYPE_FLOAT, &ds.prefs.maxVarianceTrhesh, " min=0.0 max=20.0 step=0.01 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "R", TW_TYPE_FLOAT, &ds.prefs.R, " min=0.0 max=200.0 step=0.1 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "APPLICATION DISTANCE", TW_TYPE_FLOAT, &ds.prefs.applicationDistance, " min=-10.0 max=10.0 step=0.01 keyIncr=k keyDecr=l help=' ' ");

    TwAddSeparator(myBar,"sep0",NULL);

    TwAddVarRW(myBar, "UseHARD", TW_TYPE_BOOL8, &ds.prefs.hardBonds, "label='Use HARD' key=8 help='Process hard bonds.' true='YES' false='NO' ");
    TwAddVarRW(myBar, "UseSOFT", TW_TYPE_BOOL8, &ds.prefs.softBonds, "label='Use SOFT' key=9 help='Process soft bonds.' true='YES' false='NO' ");

    TwAddSeparator(myBar,"sep1",NULL);

    TwAddVarRW(myBar, "ShowINTERSECT", TW_TYPE_BOOL8, &ds.prefs.showIntersectBonds, "label='Show INTERSECT' key=1 help='Show intersect bonds.' true='YES' false='NO' group=Visualization ");
    TwAddVarRW(myBar, "ShowHARD", TW_TYPE_BOOL8, &ds.prefs.showHardBonds, "label='Show HARD' key=2 help='Show hard bonds.' true='YES' false='NO' group=Visualization ");

    //TwAddVarRW(myBar, "ShowSOFT", TW_TYPE_BOOL8, &ds.prefs.showSoftBonds, "label='Show SOFT' key=3 help='Show soft bonds.' true='YES' false='NO' group=Visualization ");
    TwAddVarCB(myBar, "ShowSOFT", TW_TYPE_BOOL8, SetCallback, GetCallback, &ds.prefs, NULL);
    TwAddVarRW(myBar, "Showgrid", TW_TYPE_BOOL8, &ds.prefs.showGrid, "label='Show grid' key=space help='Show the grid' true='YES' false='NO' group=Visualization ");

    TwAddSeparator(myBar,"sep2",NULL);

    TwAddVarRO(myBar, "RemovedCnt", TW_TYPE_INT32, &ds.prefs.removedCnt, "label='Removed:' group='Bonds number' ");
    TwAddVarRO(myBar, "IntersectCnt", TW_TYPE_INT32, &ds.prefs.intersectCnt, "label='Intersect:' group='Bonds number' ");
    TwAddVarRO(myBar, "HardCnt", TW_TYPE_INT32, &ds.prefs.hardCnt, "label='Hard' group='Bonds number' ");
    TwAddVarRO(myBar, "SoftCnt", TW_TYPE_INT32, &ds.prefs.softCnt, "label='Soft:' group='Bonds number' ");

    //TwAddVarCB(myBar, "aacb", TW_TYPE_INT32, SetCallback, GetCallback, NULL, NULL);

    TwAddSeparator(myBar,"sep3",NULL);

    TwAddButton(myBar,"Reload!", reloadAll, &ds.prefs, NULL);



}

int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA |GLUT_DOUBLE );
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(5, 5);
    glutCreateWindow("Animated Mol");

    //Preferences prefs;
    initComponents(renderer.ds.prefs);
    // Check if glew is ok
    renderer.init();

    glutReshapeFunc(reshapeCB);

    glutDisplayFunc(displayCallback); //funzione invocata quando c'è del rendering
    //da fare
    glutIdleFunc(idleCallback); //funzione quando non c'è necessità
    //di rendering
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);


    //Preferences* prefs2 = new Preferences();
    initTweakBar(renderer.ds);



    atexit(terminateGlut); //Funzione chiamata quando termina il main loop

    glutMainLoop();




}
