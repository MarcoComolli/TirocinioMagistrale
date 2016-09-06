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
#include <quterenderer.h>
#include <mydroptarget.h>

#include <tchar.h>
#include <windows.h>

#include <GL/glut.h>

#define WINDOW_WIDTH 922
#define WINDOW_HEIGHT 691

extern MyRenderer renderer;
extern QuteRenderer quteRenderer;


void processKey(unsigned char);
void processMouse(int,int,int,int,int);
void setSpecKeyStatus(int);
void initComponents(Preferences& prefs, bool isQuteRend);
void mouseMotion(int x, int y, bool isQuteRend);
bool isQuteRendering = true;


void idleCallback(){
    renderer.ds.doPhysicsStep(1.0f/30.0f,renderer.ps);
    glutPostRedisplay();
}

void idleCallbackQR(){
    quteRenderer.ds.doPhysicsStep(1.0f/30.0f,quteRenderer.ps);
    quteRenderer.geometryChanged();
    quteRenderer.maybePrepareDisplayList();
    glutPostRedisplay();
}

void displayCallback(){
    renderer.render();
    TwDraw();
    glutSwapBuffers();
}

void displayCallbackQR(){
    quteRenderer.glDrawDirect();
    TwDraw();
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
    mouseMotion(x,y, isQuteRendering);
}

static void reshapeCB(int w, int h){
    glViewport(0,0,w,h);
    TwWindowSize(w, h);
}

static void terminateGlut(){
    std::cout << "TERMINATO GLUT" << std::endl;
    OleUninitialize();
    TwTerminate();
}

bool checkPDBFromPath(const char * path){
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath_s(path,drive,dir,fname,ext);
    if(string(ext) == ".pdb"){
         return true;
    }
    else{
        return false;
    }
}

string openFileName(char *filter = _T("All Files (*.*)\0*.*\0"), HWND owner = NULL) {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = _T("");
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "";
    string fileNameStr;
    if ( GetOpenFileName(&ofn) )
        fileNameStr = fileName;
    return fileNameStr;
}

void initQuteRenderer(){


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(70, (WINDOW_WIDTH)/WINDOW_HEIGHT, 0.1, 100.1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0,0,-80);


    quteRenderer.settings.directLightAmount = 0.6;
    quteRenderer.settings.ambientLightAmount = 0.7;
    quteRenderer.settings.flattenDirectLight = 0.5;
    quteRenderer.settings.borderSize = 0.0;
    quteRenderer.settings.glossiness = 1;
    quteRenderer.settings.shininess = 0.4;
    quteRenderer.settings.saturation = 0.8;


    quteRenderer.glResetIrradianceMap();


    //currentCutPlane = qmol::Plane(0,0,0,0);

    quteRenderer.glCaptureCurrentMatrices();
}


void TW_CALL reloadAll(void * prefs){
    initComponents(*(Preferences *) (prefs), false );
    renderer.quaternion = Vec4(0,0,0,1);
    renderer.init();
}

void TW_CALL reloadAllQute(void * prefs){
    initComponents(*(Preferences *) (prefs), true );
    quteRenderer.geometryChanged();
}

void TW_CALL openNewMolQute(void * prefs){
    string str = openFileName();
    if(!str.empty()){
        if (checkPDBFromPath(str.c_str())) { //if is a .PDB file
            std::wstring wsTmp(str.begin(), str.end());
            (*(Preferences *)(prefs)).molPath = wsTmp;
            initComponents(*(Preferences *) (prefs), true );
            quteRenderer.geometryChanged();
        }
    }
}


//setters
void TW_CALL setCBShowSoftBonds(const void *value, void *clientData){
    (*(Preferences *)(clientData)).showSoftBonds = *(const bool *)value;
    renderer.generateBuffers((*(Preferences *)(clientData)));
}

void TW_CALL setCBShowHardBonds(const void *value, void *clientData){
    (*(Preferences *)(clientData)).showHardBonds = *(const bool *)value;
    renderer.generateBuffers((*(Preferences *)(clientData)));
}

void TW_CALL setCBShowIntersectBonds(const void *value, void *clientData){
    (*(Preferences *)(clientData)).showIntersectBonds = *(const bool *)value;
    renderer.generateBuffers((*(Preferences *)(clientData)));
}

void TW_CALL setCBExtraBonds(const void *value, void *clientData){
    (*(Preferences *)(clientData)).numberExtraBonds = *(const int *)value;
    //recalcPS();
    initComponents(*(Preferences *) (clientData), false);
    renderer.generateBuffers((*(Preferences *)(clientData)));
}
void TW_CALL setCBMaxVar(const void *value, void *clientData){
    (*(Preferences *)(clientData)).maxVarianceTrhesh = *(const float *)value;
    initComponents(*(Preferences *) (clientData), false);
    renderer.generateBuffers((*(Preferences *)(clientData)));
}
void TW_CALL setCBMinVar(const void *value, void *clientData){
    (*(Preferences *)(clientData)).minVarianceTrhesh = *(const float *)value;
    initComponents(*(Preferences *) (clientData), false);
    renderer.generateBuffers((*(Preferences *)(clientData)));
}


void TW_CALL setCBMaxVarQute(const void *value, void *clientData){
    (*(Preferences *)(clientData)).maxVarianceTrhesh = *(const float *)value;
    initComponents(*(Preferences *) (clientData), true);
    quteRenderer.geometryChanged();
}
void TW_CALL setCBMinVarQute(const void *value, void *clientData){
    (*(Preferences *)(clientData)).minVarianceTrhesh = *(const float *)value;
    initComponents(*(Preferences *) (clientData), true);
    quteRenderer.geometryChanged();
}

void TW_CALL setCBExtraBondsQute(const void *value, void *clientData){
    (*(Preferences *)(clientData)).numberExtraBonds = *(const int *)value;
    initComponents(*(Preferences *) (clientData), true);
   quteRenderer.geometryChanged();
}



//getters
void TW_CALL getCBShowSoftBonds(void *value, void *clientData){
    *(bool *)value = (*(Preferences *)(clientData)).showSoftBonds;
}

void TW_CALL getCBShowHardBonds(void *value, void *clientData){
    *(bool *)value = (*(Preferences *)(clientData)).showHardBonds;
}

void TW_CALL getCBShowIntersectBonds(void *value, void *clientData){
    *(bool *)value = (*(Preferences *)(clientData)).showIntersectBonds;
}

void TW_CALL getCBExtraBonds(void *value, void *clientData){
    *(int *)value = (*(Preferences *)(clientData)).numberExtraBonds;
}

void TW_CALL getCBMaxVar(void *value, void *clientData){
    *(float *)value = (*(Preferences *)(clientData)).maxVarianceTrhesh;
}

void TW_CALL getCBMinVar(void *value, void *clientData){
    *(float *)value = (*(Preferences *)(clientData)).minVarianceTrhesh;
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

    //TwAddVarRW(myBar, "Extra bonds", TW_TYPE_INT32, &ds.prefs.numberExtraBonds , " min=0 max=500000 step=100 keyIncr=z keyDecr=x help=' '");
    TwAddVarCB(myBar, "ExtraBonds", TW_TYPE_INT32, setCBExtraBonds, getCBExtraBonds, &ds.prefs, "label='Extra bonds' min=0 max=500000 step=100 keyIncr=z keyDecr=x help=' '");

    TwAddVarRW(myBar, "Damp", TW_TYPE_FLOAT, &ds.prefs.damp, " min=0.0 max=1.0 step=0.001 keyIncr=a keyDecr=s help=' '");
    TwAddVarRW(myBar, "Trail", TW_TYPE_FLOAT, &ds.prefs.trailing, " min=0.0 max=1.0 step=0.01 keyIncr=i keyDecr=u help=' '");
    TwAddVarRW(myBar, "TrailThresh", TW_TYPE_FLOAT, &ds.prefs.trailThres, " min=0.0 max=20.0 step=0.01 keyIncr=h keyDecr=j help=' '");

//  TwAddVarRW(myBar, "Min variance", TW_TYPE_FLOAT, &ds.prefs.minVarianceTrhesh, " min=0.0 max=5.0 step=0.01 keyIncr=o keyDecr=p help=' '");
//  TwAddVarRW(myBar, "Max variance", TW_TYPE_FLOAT, &ds.prefs.maxVarianceTrhesh, " min=0.0 max=20.0 step=0.01 keyIncr=k keyDecr=l help=' '");
    TwAddVarCB(myBar, "Min variance", TW_TYPE_FLOAT, setCBMinVar, getCBMinVar, &ds.prefs, " min=0.0 max=5.0 step=0.01 keyIncr=o keyDecr=p help=' '");
    TwAddVarCB(myBar, "Max variance", TW_TYPE_FLOAT, setCBMaxVar, getCBMaxVar, &ds.prefs, " min=0.0 max=20.0 step=0.01 keyIncr=k keyDecr=l help=' '");

    TwAddVarRW(myBar, "R", TW_TYPE_FLOAT, &ds.prefs.R, " min=0.1 max=200.0 step=0.1 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "APPLICATION DISTANCE", TW_TYPE_FLOAT, &ds.prefs.applicationDistance, " min=-10.0 max=10.0 step=0.01 keyIncr=k keyDecr=l help=' ' ");

    TwAddSeparator(myBar,"sep0",NULL);

    TwAddVarRW(myBar, "UseHARD", TW_TYPE_BOOL8, &ds.prefs.hardBonds, "label='Use HARD' key=8 help='Process hard bonds.' true='YES' false='NO' ");
    TwAddVarRW(myBar, "UseSOFT", TW_TYPE_BOOL8, &ds.prefs.softBonds, "label='Use SOFT' key=9 help='Process soft bonds.' true='YES' false='NO' ");

    TwAddSeparator(myBar,"sep1",NULL);

    //TwAddVarRW(myBar, "ShowINTERSECT", TW_TYPE_BOOL8, &ds.prefs.showIntersectBonds, "label='Show INTERSECT' key=1 help='Show intersect bonds.' true='YES' false='NO' group=Visualization ");
    TwAddVarCB(myBar, "ShowINTERSECT", TW_TYPE_BOOL8, setCBShowIntersectBonds, getCBShowIntersectBonds, &ds.prefs, "label='Show INTERSECT' key=1 help='Show intersect bonds.' true='YES' false='NO' group=Visualization ");

    //TwAddVarRW(myBar, "ShowHARD", TW_TYPE_BOOL8, &ds.prefs.showHardBonds, "label='Show HARD' key=2 help='Show hard bonds.' true='YES' false='NO' group=Visualization ");
    TwAddVarCB(myBar, "ShowHARD", TW_TYPE_BOOL8, setCBShowHardBonds, getCBShowHardBonds, &ds.prefs, "label='Show HARD' key=2 help='Show hard bonds.' true='YES' false='NO' group=Visualization ");

    //TwAddVarRW(myBar, "ShowSOFT", TW_TYPE_BOOL8, &ds.prefs.showSoftBonds, "label='Show SOFT' key=3 help='Show soft bonds.' true='YES' false='NO' group=Visualization ");
    TwAddVarCB(myBar, "ShowSOFT", TW_TYPE_BOOL8, setCBShowSoftBonds, getCBShowSoftBonds, &ds.prefs, "label='Show SOFT' key=3 help='Show soft bonds.' true='YES' false='NO' group=Visualization ");

    TwAddVarRW(myBar, "Showgrid", TW_TYPE_BOOL8, &ds.prefs.showGrid, "label='Show grid' key=space help='Show the grid' true='YES' false='NO' group=Visualization ");

    TwAddSeparator(myBar,"sep2",NULL);

    TwAddVarRO(myBar, "RemovedCnt", TW_TYPE_INT32, &ds.prefs.removedCnt, "label='Removed:' group='Bonds number' ");
    TwAddVarRO(myBar, "IntersectCnt", TW_TYPE_INT32, &ds.prefs.intersectCnt, "label='Intersect:' group='Bonds number' ");
    TwAddVarRO(myBar, "HardCnt", TW_TYPE_INT32, &ds.prefs.hardCnt, "label='Hard' group='Bonds number' ");
    TwAddVarRO(myBar, "SoftCnt", TW_TYPE_INT32, &ds.prefs.softCnt, "label='Soft:' group='Bonds number' ");


    TwAddSeparator(myBar,"sep3",NULL);

    TwAddButton(myBar,"Reload!", reloadAll, &ds.prefs, NULL);

}


void initTweakBarQute(DynamicShape& ds){
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    TwBar *myBar;

    myBar = TwNewBar("Parameters");
    TwDefine(" Parameters size='200 420' color='0 170 0' text=light"); // change default tweak bar size and color
    TwDefine(" Parameters alpha = 30 valueswidth=60 iconalign=horizontal iconpos=tl ");
    TwDefine(" GLOBAL iconmargin='5 5' ");
    TwDefine(" GLOBAL fontsize=2 fontstyle=default contained=true buttonalign=left ");

    TwAddVarCB(myBar, "ExtraBonds", TW_TYPE_INT32, setCBExtraBondsQute, getCBExtraBonds, &ds.prefs, "label='Extra bonds' min=0 max=500000 step=100 keyIncr=z keyDecr=x help=' '");

    TwAddVarRW(myBar, "Damp", TW_TYPE_FLOAT, &ds.prefs.damp, " min=0.0 max=1.0 step=0.001 keyIncr=a keyDecr=s help=' '");
    TwAddVarRW(myBar, "Trail", TW_TYPE_FLOAT, &ds.prefs.trailing, " min=0.0 max=1.0 step=0.01 keyIncr=i keyDecr=u help=' '");
    TwAddVarRW(myBar, "TrailThresh", TW_TYPE_FLOAT, &ds.prefs.trailThres, " min=0.0 max=20.0 step=0.01 keyIncr=h keyDecr=j help=' '");

    TwAddVarCB(myBar, "Min variance", TW_TYPE_FLOAT, setCBMinVarQute, getCBMinVar, &ds.prefs, " min=0.0 max=5.0 step=0.01 keyIncr=o keyDecr=p help=' '");
    TwAddVarCB(myBar, "Max variance", TW_TYPE_FLOAT, setCBMaxVarQute, getCBMaxVar, &ds.prefs, " min=0.0 max=20.0 step=0.01 keyIncr=k keyDecr=l help=' '");

    TwAddVarRW(myBar, "R", TW_TYPE_FLOAT, &ds.prefs.R, " min=0.1 max=200.0 step=0.1 keyIncr=k keyDecr=l help=' '");
    TwAddVarRW(myBar, "APPLICATION DISTANCE", TW_TYPE_FLOAT, &ds.prefs.applicationDistance, " min=-10.0 max=10.0 step=0.01 keyIncr=k keyDecr=l help=' ' ");

    TwAddSeparator(myBar,"sep0",NULL);

    TwAddVarRW(myBar, "UseHARD", TW_TYPE_BOOL8, &ds.prefs.hardBonds, "label='Use HARD' key=8 help='Process hard bonds.' true='YES' false='NO' ");
    TwAddVarRW(myBar, "UseSOFT", TW_TYPE_BOOL8, &ds.prefs.softBonds, "label='Use SOFT' key=9 help='Process soft bonds.' true='YES' false='NO' ");

    TwAddSeparator(myBar,"sep1",NULL);

    TwAddVarRO(myBar, "RemovedCnt", TW_TYPE_INT32, &ds.prefs.removedCnt, "label='Removed:' group='Bonds number' ");
    TwAddVarRO(myBar, "IntersectCnt", TW_TYPE_INT32, &ds.prefs.intersectCnt, "label='Intersect:' group='Bonds number' ");
    TwAddVarRO(myBar, "HardCnt", TW_TYPE_INT32, &ds.prefs.hardCnt, "label='Hard' group='Bonds number' ");
    TwAddVarRO(myBar, "SoftCnt", TW_TYPE_INT32, &ds.prefs.softCnt, "label='Soft:' group='Bonds number' ");

    TwAddSeparator(myBar,"sep3",NULL);

    TwAddVarRW(myBar, "Direct Light", TW_TYPE_FLOAT, &quteRenderer.settings.directLightAmount, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Flat Direct Light", TW_TYPE_FLOAT, &quteRenderer.settings.flattenDirectLight, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Ambient Light", TW_TYPE_FLOAT, &quteRenderer.settings.ambientLightAmount, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Border", TW_TYPE_FLOAT, &quteRenderer.settings.borderSize, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Glossiness", TW_TYPE_FLOAT, &quteRenderer.settings.glossiness, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Shininess", TW_TYPE_FLOAT, &quteRenderer.settings.shininess, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");
    TwAddVarRW(myBar, "Saturation", TW_TYPE_FLOAT, &quteRenderer.settings.saturation, " min=0.0 max=1.0 step=0.01  group='Rendering settings'");

    TwAddSeparator(myBar,"sep4",NULL);

    TwAddButton(myBar,"Reload!", reloadAllQute, &ds.prefs, NULL);

    TwAddButton(myBar,"Open...", openNewMolQute, &ds.prefs, NULL);


}




int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA |GLUT_DOUBLE );
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(5, 5);
    glutCreateWindow("Animated Mol");

    if(isQuteRendering){
        initComponents(quteRenderer.ds.prefs, isQuteRendering);
        quteRenderer.initGlew();
        glutDisplayFunc(displayCallbackQR); //funzione invocata quando c'è del rendering da fare
        glutIdleFunc(idleCallbackQR); //funzione quando non c'è necessità di rendering

        initTweakBarQute(quteRenderer.ds);

        initQuteRenderer();
    }
    else{
        initComponents(renderer.ds.prefs, isQuteRendering); //wireframe Renderer

        // Check if glew is ok
        renderer.init();

        glutDisplayFunc(displayCallback); //funzione invocata quando c'è del rendering da fare
        glutIdleFunc(idleCallback); //funzione quando non c'è necessità di rendering

        initTweakBar(renderer.ds);
    }

    glutReshapeFunc(reshapeCB);

    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);

    atexit(terminateGlut); //Funzione chiamata quando termina il main loop


    OleInitialize(NULL);
    HDC a = wglGetCurrentDC();
    HWND hw = WindowFromDC(a);

    char wnd_title[256];
    GetWindowText(hw,wnd_title,sizeof(wnd_title));
    std::cout << "wnd_title " << wnd_title << std::endl;

    MyDropTarget aa;
    RegisterDragDrop(hw,&aa);

    glutMainLoop();
}
