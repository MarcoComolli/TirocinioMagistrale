#include <iostream>
#include <fstream>
#include <vector>
#include <qutemolLib/molecule.h>
#include <qutemolLib/shape.h>
#include <qutemolLib/shapingOptions.h>
#include <qutemolLib/atomTable.h>
#include <atompair.h>
#include <myrenderer.h>
#include <dynamicshape.h>
#include <spatialgrid.h>
#include <AntTweakBar.h>
#include <quterenderer.h>

#include <mydroptarget.h>
#include <shobjidl.h>

#include <Commdlg.h>


#include <GL/glew.h>

#define LEFT_BUTTON 0
#define MID_BUTTON 1
#define RIGHT_BUTTON 2
#define SCROLL_UP 3
#define SCROLL_DOWN 4

#define RELEASED 1
#define PRESSED 0

#define NO_SPECIAL 0
#define SHIFT 1
#define CTRL 2
#define CTRL_SHIFT 3
#define ALT 4
#define ALT_SHIFT 5
#define CTRL_ALT 6
#define CTRL_ALT_SHIFT 7



using namespace std;


string printSphere(qmol::Ball ball){
    return "["
            + to_string(ball.pos.X()) + ","
            + to_string(ball.pos.Y()) + ","
            + to_string(ball.pos.Z()) + "]";
}

string printCoords(qmol::Pos coord){
    return "["
            + to_string(coord.X()) + ","
            + to_string(coord.Y()) + ","
            + to_string(coord.Z()) + "]";
}


//DynamicShape dsglobal;
//PairStatistics psglobal;
MyRenderer renderer;
QuteRenderer quteRenderer;


vector<qmol::Shape> getShapes(wstring filename, Preferences& prefs){
    qmol::Molecule mol;


    mol.loadPDB(filename);

    using namespace qmol;
    vector<Shape> shapes;


    if(!mol.isEmpty()){
        Shape shape;
        ShapingOptions opt;
//        opt.mode = 2;
//        opt.keepHeteros = true;
//        opt.keepHydrogens = true;
//        opt.selectedOnly = false;
//        opt.colorizeMode = 0;
//        opt.showOnlyModel = -1;
        opt.setDefaults();
        opt.colorizeMode = prefs.colorizeMode;

        AtomTable table;
        table.loadCSV("F:/Programmazione/qtmol/qutemol2/qutemolLib/atomTable.csv");
        ChainTable chaintable;
        chaintable.setRandom(153);

        for (int i = 1; i < mol.nModels() + 1 ; i++) { //per ogni modello
            mol.makeSingleShape(shape,opt,table, chaintable, i);  //crea lo shape di id = i
            shapes.push_back(shape);
        }

    }

    return shapes;
}

void initGrid(DynamicShape& ds, SpatialGrid& gr){

    gr.reset();
    vector<qmol::Scalar> shapeBounds = ds.findBounds(0.5);
    gr.set(shapeBounds,6);

    std::cout << "grid cell size: " << gr.cellDimX << "," << gr.cellDimY << "," << gr.cellDimZ << std::endl;
    gr.populateWithShape(ds);

    gr.printGridStats();

    renderer.nGrid =
            ((gr.gridX.size())*(gr.gridZ.size())) +
            ((gr.gridY.size())*(gr.gridZ.size())) +
            ((gr.gridX.size())*(gr.gridY.size()));
}

void initComponents(Preferences& prefs, bool isQuteRendering){

    vector<qmol::Shape> shapes = getShapes(prefs.molPath, prefs);

    if (isQuteRendering) {
        DynamicShape& ds(quteRenderer.ds);
        ds.copyFrom(shapes[0]);
        ds.prefs = prefs;
        SpatialGrid &gr(quteRenderer.grid);
        initGrid(ds,gr);
        PairStatistics& ps (quteRenderer.ps);
        ps = gr.generatePairStatistcs(ds,3.4,ds.prefs.numberExtraBonds);

        for (uint i = 0; i < shapes.size() ; i++) { //per ogni modello
            ps.add_shape(shapes[i]);
        }

        ps.calculateStats();
        //ps.printStats();

        cout << "rispetto al max: " << ds.ball.size()*ds.ball.size() << " e': " << ps.pairs.size() <<
                "\nCioe' " << ds.ball.size()*(qmol::Scalar)ds.ball.size()/ps.pairs.size() << " volte meno numeroso" << endl;

        ds.calculatePSBonds(ps);

        ps.printStats();

        ps.updateDistancesFromShape(shapes[0]);

        ds.initCollisionDetection();


        prefs.R = ds.radius/2;

    }
    else{
        DynamicShape& ds (renderer.ds);
        ds.copyFrom(shapes[0]);
        ds.prefs = prefs;
        SpatialGrid &gr(renderer.grid);
        initGrid(ds,gr);
        PairStatistics& ps (renderer.ps);

        ps = gr.generatePairStatistcs(ds,3.4,ds.prefs.numberExtraBonds);

        for (uint i = 0; i < shapes.size() ; i++) { //per ogni modello
            ps.add_shape(shapes[i]);
        }

        ps.calculateStats();
        ps.printStats();

        cout << "rispetto al max: " << ds.ball.size()*ds.ball.size() << " e': " << ps.pairs.size() <<
                "\nCioe' " << ds.ball.size()*(qmol::Scalar)ds.ball.size()/ps.pairs.size() << " volte meno numeroso" << endl;

        ds.calculatePSBonds(ps);

        ps.printStats();

        ps.updateDistancesFromShape(shapes[0]);
    }

}



static int xStored = 0;
static int yStored = 0;
int pressedButton = -1;


void processMouse(int button, int state, int x, int y, int modifiers){
    if(TwEventMouseButtonGLUT(button, state, x, y)){
        return;
    }

    if(state == PRESSED){
        pressedButton = button;
    }else{
        pressedButton = -1;
    }

    if(button == SCROLL_DOWN){
        //renderer.panZ--;
        quteRenderer.glZoomView(-1);
    }
    if(button == SCROLL_UP){
        //renderer.panZ++;;
        quteRenderer.glZoomView(1);

    }


    xStored = x;
    yStored = y;

}


void mouseMotion(int x, int y, bool isQuteRend){
    if(TwEventMouseMotionGLUT(x,y)){
        return;
    }
    int dx = x-xStored;
    int dy = y-yStored;


    if(pressedButton == RIGHT_BUTTON){
        qmol::Vec axis(0,0,1);

        if(isQuteRend){quteRenderer.impressUserRotation(axis, dx);}
        else{renderer.impressUserRotation(axis, dx);}
    }else if(pressedButton == MID_BUTTON){
        quteRenderer.glRotY(dx);
    }else{
        //trascinamento
        qmol::Vec axis(dy, dx,0);
        if(isQuteRend){quteRenderer.impressUserRotation(axis, axis.Norm());}
        else{renderer.impressUserRotation(axis, axis.Norm());}

    }

    xStored = x;
    yStored = y;

}




void processKey(unsigned char key){
    switch (key){
    case 'w':
        //dsglobal.ball[0].prevPos.X() += 20;
        renderer.ds.wiggle(0.9f);
        renderer.ds.updateBarycenter();
        quteRenderer.ds.updateBarycenter();
        break;
    case 'c':
    {
//        std::cout << "pressed C" << std::endl;
//       OleInitialize(NULL);
//       HDC a = wglGetCurrentDC();
//       HWND hw = WindowFromDC(a);
//      // HWND hw = GetForegroundWindow();

//        char wnd_title[256];
//        GetWindowText(hw,wnd_title,sizeof(wnd_title));
//        std::cout << "wnd_title " << wnd_title << std::endl;


//       RegisterDragDrop(hw,&aa);

}
        break;

    default:
        break;
    }

}




