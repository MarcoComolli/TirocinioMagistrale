#include "quterenderer.h"
#include <GL/glew.h>
#include <qutemolLib/glSurface.h>
#include <qutemolLib/glProgram.h>

namespace attr{
declare_shader_attribute( position );
declare_shader_attribute( radius );
declare_shader_attribute( color );
declare_shader_attribute( uv );
}

declare_shader_macro( CARE_FOR_ALPHA );
declare_shader_macro( USE_CUT_PLANE );
declare_shader_macro( USE_FOG );
declare_shader_macro( USE_DIRECT_LIGHT );
declare_shader_macro( USE_SPECULAR );
declare_shader_macro( USE_COLORED_LIGHTS );
declare_shader_macro( USE_TWO_WAY_LIGHT );
declare_shader_macro( USE_SHADOW_MAP );
declare_shader_macro( USE_BORDERS );
declare_shader_macro( USE_AMBIENT_LIGHT );
declare_shader_macro( USE_FIXED_AMBIENT );
declare_shader_macro( USE_FLAT_HIGHLIGHTS );
declare_shader_macro( USE_SEM_MODE );
declare_shader_macro( USE_SHADOW_ATTENUATION );
declare_shader_macro( USE_DIRECT_LIGHT_BONUS );
declare_shader_macro( USE_TRIVIAL_LIGHT );

namespace unif {
declare_shader_uniform( shadowAttenuation );
declare_shader_uniform( clipUnitSize );
declare_shader_uniform( patchSize );
declare_shader_uniform( shadowMap );
declare_shader_uniform( irradianceMap );
declare_shader_uniform( scale );
declare_shader_uniform( texelSize );
declare_shader_uniform( colMap );
declare_shader_uniform( depthMap );
declare_shader_uniform( textureSize );
declare_shader_uniform( dofQuadric );
declare_shader_uniform( lightRot );
declare_shader_uniform( lightTra );
declare_shader_uniform( lightScale );
declare_shader_uniform( lightDir );
declare_shader_uniform( cutPlane );
declare_shader_uniform( keepInsideFactor );
declare_shader_uniform( saturation );
declare_shader_uniform( directLightBonus );
declare_shader_uniform( lightColorAmbient );
declare_shader_uniform( lightColorDirect );
declare_shader_uniform( lightColorOpposite );
declare_shader_uniform( oppositeLight );
declare_shader_uniform( halfwayDir );
declare_shader_uniform( shine );
declare_shader_uniform( gloss );
declare_shader_uniform( oppositeHalfwayDir );
declare_shader_uniform( oppositeShine );
declare_shader_uniform( borderSize );
declare_shader_uniform( borderColorMul );
declare_shader_uniform( borderVariance );
declare_shader_uniform( shineThreshold );
declare_shader_uniform( aoSum );
declare_shader_uniform( aoMul );
declare_shader_uniform( shadowMatrix );
declare_shader_uniform( lightoutCut );
declare_shader_uniform( fogAmount );
declare_shader_uniform( fogColor );
}


inline int toPercent( float x ){
    return int(x*100.0);
}

using namespace qmol;

QuteRenderer::QuteRenderer(DynamicShape &ds):ds(ds){

    irradianceMap = new qmol::glSurface();

    directShadowMapEnabled = false;

    performanceSettings.setDefaults();
    settings.setDefaults();

    cutPlane = Plane (1,0,0,0);
    cutPlaneEnabled = false;
    careForAlpha = false;

    dlBalls = 666; // display lists
    dlBallReady = false;

    overlayOpacity = 1.0; // fully opaque

    updateAllDefinesInShaders();

    patchSizeTexels = 1;

    rollRandomDirs();

    loadShadersSources();

    progFinalBall.uses( attr::position );
    progFinalBall.uses( attr::radius );
    progFinalBall.uses( attr::color );
    progFinalBall.uses( attr::uv );

}

void QuteRenderer::loadShadersSources(){

    progFinalBall.loadSources("F:/Documenti - Marco/Documenti/Universita/Tirocinio Magistrale/qt_workspace/TirocinioMolecola/ballFinal");
}


void QuteRenderer::updateAllDefinesInShaders(){

    progFinalBall.set( USE_FOG , settings.fogAmount>0 );
    progFinalBall.set( USE_DIRECT_LIGHT , settings.directLightAmount>0 );
    progFinalBall.set( USE_SPECULAR , settings.shininess>0 );
    progFinalBall.set( USE_COLORED_LIGHTS , settings.coloredLightEnabled );
    progFinalBall.set( USE_TWO_WAY_LIGHT , settings.oppositeLightAmount );

    progFinalBall.set( USE_SHADOW_MAP , isShadowMapNeeded() );

    progFinalBall .set( USE_CUT_PLANE , cutPlaneEnabled );


    progFinalBall   .set( CARE_FOR_ALPHA, careForAlpha );
    progDepthOfField.set( CARE_FOR_ALPHA, careForAlpha );

    progFinalBall.set( USE_BORDERS , settings.borderSize>0 );

    progFinalBall.set( USE_AMBIENT_LIGHT , (settings.ambientLightAmount>0)&&(settings.flattenAmbientLight<1)  );
    progFinalBall.set( USE_FIXED_AMBIENT , (settings.ambientLightAmount>0)&&(settings.flattenAmbientLight>0)  );
    progFinalBall.set( USE_FLAT_HIGHLIGHTS , settings.flattenHighLights );
    progFinalBall.set( USE_SEM_MODE , settings.semModeEnabled );
    progFinalBall.set( USE_SHADOW_ATTENUATION , settings.attenuateShadows>0 );

    progFinalBall.set( USE_DIRECT_LIGHT_BONUS ,
                         settings.flattenDirectLight*settings.directLightAmount>0 );
    progFinalBall.set( USE_TRIVIAL_LIGHT , isLightDirTrivial()  );

}


static void _glClearMatrices(){
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glScalef(2,2,1);
    glTranslatef(-0.5,-0.5,0);
}


static void _glPopMatrices(){
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}


void QuteRenderer::glSplashFullScreen(glSurface& surface, int w, int h , int w2, int h2, float alpha){

    _glClearMatrices();
    bind_surface_target( surface,0 );
    bind_program(0);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (alpha<1.0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDisable(GL_DEPTH_TEST);
    glColor4f(1,1,1,alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0          ,float(h)/h2);glVertex2d(0,1);
    glTexCoord2f(0          ,0          );glVertex2d(0,0);
    glTexCoord2f(float(w)/w2,0          );glVertex2d(1,0);
    glTexCoord2f(float(w)/w2,float(h)/h2);glVertex2d(1,1);
    glEnd();

    if (alpha<1.0) glDisable(GL_BLEND);

    _glPopMatrices();
}


void QuteRenderer::glSplashWithDepthOfField(glSurface& surface, int w, int h , int w2, int h2, float multiSample){

    bind_program(progDepthOfField);
    progDepthOfField.set(unif::texelSize,
                             4.0f/w2*settings.dofStrength*multiSample,
                             4.0f/h2*settings.dofStrength*multiSample);
    progDepthOfField.set(unif::colMap, 1);
    progDepthOfField.set(unif::depthMap, 0);
    progDepthOfField.set(unif::textureSize,float(w)/w2, float(h)/h2);

    float z = zMin * (1-settings.dofPos) + zMax * (settings.dofPos)  ;
    float range = settings.dofRange * fabs(zMax-zMin)/6;

    /*
    float aa = (str==0)?0:-range*range/(str*16),
          bb = -z*2*aa,
          cc = (aa==0)?0:(bb*bb - range*range/4)/(4*aa);
    */

    Vec q(-z, 1/(3*range), -1/3.0f );

    progDepthOfField.set( unif::dofQuadric , q );

    _glClearMatrices();

    glActiveTexture(GL_TEXTURE0);
    bind_surface_target( surface,-1 );
    glActiveTexture(GL_TEXTURE1);
    bind_surface_target( surface,0 );

    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glVertex2d(0,1);
    glVertex2d(0,0);
    glVertex2d(1,0);
    glVertex2d(1,1);
    glEnd();

    glActiveTexture(GL_TEXTURE0);
    _glPopMatrices();
}

int div_ceil( int a, int b) { return (a+b-1)/b; }


void QuteRenderer::maybePrepareDisplayList(FeedbackFunctionT feedback) {

    const int MAX_DL = 1000;

    if (dlBalls == 666) dlBalls = glGenLists(MAX_DL);

    if (!dlBallReady)
    {
        static int _invocation = 0;
        int invocation = _invocation++;

        //debug("Making a DL for %d balls", shape.ball.size() );
        //if (feedback) debug("(with feedback)"); else debug("(withOUT feedback)");

        dlBallsCount = 0;
        int countdown = 0;

        int nPatchesPerRow = irradianceMapSize / patchSizeTexels;
        int nExpectedLists = div_ceil( ds.ball.size() , performanceSettings.displayListSize);


        for (uint i=0; i<ds.ball.size(); i++) {
            if (countdown-- <= 0 ) {

                //debug("(%d) %d-th list (on %d)...",invocation,dlBallsCount+1,nExpectedLists);
                if (dlBallsCount+1>=MAX_DL) {
                    //debug("\n\nToo many balls!!!!!!!\n\n");
                    break;
                }
                countdown = performanceSettings.displayListSize;
                if (dlBallsCount!=0) {
                    glEnd();
                    glEndList();
                }
                if (feedback) {
                    bool canceled = (*feedback)( div_ceil( 100 * (dlBallsCount), nExpectedLists ) );
                    if (canceled) break;
                }
                glNewList( dlBalls + dlBallsCount, GL_COMPILE );
                if (glGetError()==GL_OUT_OF_MEMORY) {
                    //debug("\n==============\nOUT OF MEMORY!\n==============\n");
                    break;
                }
                glBegin(GL_QUADS);

                dlBallsCount++;

            }
            float u = i%nPatchesPerRow + 0.5f, v = i/nPatchesPerRow + 0.5f;
            const int BORDER = 0; // JUST FOR A TEST  (even, please)
            glVertexAttrShort(attr::uv,
                         u*(BORDER+patchSizeTexels)/irradianceMapSize, v*(BORDER+patchSizeTexels)/irradianceMapSize );
            glVertexAttrShort(attr::radius,   ds.ball[i].rad );
            glVertexAttrByte(attr::color,    ds.ball[i].col );
            glVertexAttr(attr::position, ds.ball[i].currPos );

            // the impostor QUAD
            glVertex2s( +1, +1 );
            glVertex2s( +1, -1 );
            glVertex2s( -1, -1 );
            glVertex2s( -1, +1 );

            // HEXA
            /*const Scalar K = 2/sqrt(3.0f), H = 1/sqrt(3.0f);
            glBegin(GL_POLYGON);
            glVertex2f(+K, 0);
            glVertex2f(+H,-1);
            glVertex2f(-H,-1);
            glVertex2f(-K, 0);
            glVertex2f(-H,+1);
            glVertex2f(+H,+1);
            glEnd();*/

        }

        if (dlBallsCount!=0) {
            glEnd();
            glEndList();
        }

        dlBallReady = true;
        if (feedback) (*feedback)(100);

        //debug( "compiled %d list(s) from index %d",dlBallsCount,dlBalls);
        glFinish();
    }

}


void QuteRenderer::glSendBallData(){

    maybePrepareDisplayList();
    for (int i=0; i<dlBallsCount; i++)
    glCallList( dlBalls + i );

}

void QuteRenderer::geometryChanged(){
    dlBallReady = false;
}

void QuteRenderer::maybePrepareShader(qmol::glProgram &p){
    if (!p.isBuilt())
        p.buildAll();
    if (!p.isBuilt()) {
        std::string msg = "A GLSL Compiler Error Occurred :-(\n\n" + p.log();
        std::cout << msg << std::endl;
        // critical(msg.c_str());
    }
}



void QuteRenderer::maybePrepareShaders(){
    updateAllDefinesInShaders();
    maybePrepareShader(progDepthOfField);
    maybePrepareShader(progFinalBall);
}

void QuteRenderer::prepareBuffers(){

    bool res = irradianceMap->setup(
                irradianceMapSize,irradianceMapSize,0,
                rt_format( GL_R16F, /*LUMINANCE16F_ARB ,*/ rt_linear ),
                GL_NONE
                );
    //    if (!res)
    //        debug("CANNOT ALLOCATE BUFFERS!!!");
    //    else
    //        debug("%dx%d irradianceMap prepared...",irradianceMapSize,irradianceMapSize);
}


void QuteRenderer::updatePatchSizes(){
   // debug("updatepatchsize");
    int totalPatches = ds.ball.size()+ds.tube.size();
    int patchesPerSide = (int) ceil( sqrt( totalPatches ) );
    if (patchesPerSide==0) {patchSizeTexels = 1; return;} // nothing to do...

    irradianceMapSize = performanceSettings.standardIrradianceMapSize;
    patchSizeTexels =  irradianceMapSize / patchesPerSide ;

    if (patchSizeTexels<performanceSettings.minPatchSizeTexels) {
        while (patchSizeTexels<performanceSettings.minPatchSizeTexels) {
            // patches too small! enlarge irradiance map
            if (irradianceMapSize>=performanceSettings.maxIrradianceMapSize) break; // fail to provide enough patches
            irradianceMapSize *= 2;
            patchSizeTexels =  irradianceMapSize / patchesPerSide ;
        }
        if (patchSizeTexels>performanceSettings.minPatchSizeTexels)
            patchSizeTexels = performanceSettings.minPatchSizeTexels;
    }

    else if (patchSizeTexels>performanceSettings.maxPatchSizeTexels) {
        while (patchSizeTexels>performanceSettings.maxPatchSizeTexels) {
            // patches too big! shrink irradiance map
            irradianceMapSize /= 2;
            patchSizeTexels =  irradianceMapSize / patchesPerSide ;
        }
        if (patchSizeTexels<performanceSettings.maxPatchSizeTexels) {
            patchSizeTexels = performanceSettings.maxPatchSizeTexels;
            irradianceMapSize *= 2;
        }
    }

   // debug("%d %dx%d patches", totalPatches, patchSizeTexels,patchSizeTexels);
   // debug("in a %dx%d texture", irradianceMapSize, irradianceMapSize);
}


void QuteRenderer::enableShadowMapPCF(bool on){
    bind_surface_target(shadowMap,-1);

    if (on) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

void QuteRenderer::setShadowMapSize(int i){
    performanceSettings.shadowMapSize = i;
}

void QuteRenderer::glSendShadowmapParam(qmol::glProgram &p) const{
    p.set  (unif::lightTra, -ds.barycenter );
    p.set3x3(unif::lightRot, lightRot );
    p.set   (unif::lightScale, lightScale );
    p.set  (unif::lightDir, lightDir*(2.0/performanceSettings.numAoLights) );
}

void QuteRenderer::glSendCutPlaneParam(qmol::glProgram &p) const{
    p.set  (unif::cutPlane, cutPlaneScreen );
}

void QuteRenderer::flipLight(){
    for (int i=6; i<9; i++) lightRot[i]*=-1;
}


void QuteRenderer::updateShadowmapParam(Vec _lightDir){
    lightScale = 1/ds.radius;

    lightDir = _lightDir.normalized();

    Vec tmpx(2,3,1), tmpy, tmpz;
    tmpz = lightDir;  tmpz.Normalize(); tmpz*=lightScale;
    tmpy = tmpz^tmpx; tmpy.Normalize(); tmpy*=lightScale;
    tmpx = tmpy^tmpz; tmpx.Normalize(); tmpx*=lightScale;

    lightTra = -ds.barycenter;
    //lightTra = //Vec( tmpx*lightTra, tmpy *lightTra, tmpz *lightTra  );
    //    tmpx*lightTra.X(), tmpy *lightTra.Y(), tmpz *lightTra.Z() ;

    float rot[9] = {
        tmpx[0], tmpx[1], tmpx[2],
        tmpy[0], tmpy[1], tmpy[2],
        tmpz[0], tmpz[1], tmpz[2]
    };

    for (int i=0; i<9; i++) lightRot[i] = rot[i];

}



static void maybeSetup(glSurface & surf, int w, int h, unsigned int rtCol, const render_target &rtDepth)
{

    if (w<1) w=1; if (h<1) h=1;
    if (!surf.valid()) {
        bool res = surf.setup(w,h,0,rtCol,rtDepth);
        //if (!res) debug("ALARM setup!!!"); //else qDebug("shadowmap %dx%d setup ok",w,h);
    } else {
        bool res = surf.resize(w,h,0);
        //if (!res) debug("ALARM resize!!!"); //else qDebug("shadowmap %dx%d resize ok",w,h);
    }

}

static Matrix lastUsedShadowMapMatrix; // tmp quick hack

Matrix transposedInverseM(const Matrix &m){
    Matrix m2 = m;
    return vcg::Inverse( vcg::Transpose(m2) );
}

void QuteRenderer::glRenderDepthMap(glSurface& target, int res, bool doubleSided, bool bilinear){
    const int nX = (doubleSided)?2:1, nY = 1;

    maybeSetup(
                target,
                res*nX,res*nY,
                GL_NONE,
                rt_format( GL_DEPTH_COMPONENT24 , bilinear? rt_linear: rt_nearest )
                );
    target.push();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    bind_program(progShadowBall);


    for (int copyY=0; copyY<nY; copyY++) {
        //qDebug("ok %d... ",copyY);
        if (nY==2) cutPlane *= -1;

        for (int copyX=0; copyX<nX; copyX++) {

            glViewport(res*copyX,res*copyY,res,res);
            if (nX==2) flipLight();

            lastUsedShadowMapMatrix =  molSpace2shadowMapSpaceMatrix();

            cutPlaneScreen = transposedInverseM( lastUsedShadowMapMatrix ) * cutPlane;

            glSendShadowmapParam(progShadowBall);
            glSendCutPlaneParam(progShadowBall);
            glSendBallData();

        }
    }
    target.pop();
}


void QuteRenderer::glRenderShadowMap(){
    glRenderDepthMap(
                shadowMap,
                performanceSettings.shadowMapSize,
                (settings.oppositeLightAmount>0),
                performanceSettings.usePcf );
}

void QuteRenderer::glResetIrradianceMap(){
    //debug("Reset irrandance map!");
    maybePrepareShaders();
    if (irradianceMapSize==0) irradianceMapSize=1;

    if (!irradianceMap->valid()) prepareBuffers();

    bind_surface(*irradianceMap,false);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    cumulatedIrradiance = 0;
    bind_surface(0);
}

Vec randUnitVec(){
    Vec res;
    do res = Vec( (rand()%2001/1000.0f)-1, (rand()%2001/1000.0f)-1, (rand()%2001/1000.0f)-1 );
    while (res.SquaredNorm()>1.0);
    return res.normalized();
}


void QuteRenderer::rollRandomDirs(){
    randomDir.resize( performanceSettings.numAoLights );
    for(uint i=0; i<randomDir.size(); i++ ){
        randomDir[i] = randUnitVec();
    }
}

bool QuteRenderer::isIrradianceMapFinished() const{
    return (cumulatedIrradiance>=performanceSettings.numAoLights);
}


bool QuteRenderer::glProgressIrradianceMap(int nSteps ){

    int missingSteps = performanceSettings.numAoLights-cumulatedIrradiance;
    if (nSteps>=missingSteps) nSteps = missingSteps;

    if (!nSteps) return false;

    for (int i=0; i<nSteps; i++) {
        updateShadowmapParam( randomDir[ cumulatedIrradiance + i ] );
        //glRenderAoShadowMap();
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glRenderOnIrradianceMap();
        glDisable(GL_BLEND);
    }
    cumulatedIrradiance += nSteps;
    return cumulatedIrradiance < performanceSettings.numAoLights;
}


void QuteRenderer::glRenderOnIrradianceMap(){
    bind_surface(*irradianceMap,false);

    bind_surface_target(aoShadowMap,-1);
    irradianceMap->push();

    glDisable(GL_DEPTH_TEST);

    bind_program(progBakeAoBall);
    progBakeAoBall.set(unif::patchSize, 0.5f*patchSizeTexels/irradianceMapSize);
    progBakeAoBall.set(unif::keepInsideFactor, 1.f*patchSizeTexels/(patchSizeTexels-1));
    progBakeAoBall.set(unif::cutPlane, cutPlane );
    glSendShadowmapParam(progBakeAoBall);

    glSendBallData();

    irradianceMap->pop();

    bind_surface(0,true);

}


static int _minPow2(int k){
    int i=0; while ((1<<i) < k) i++; return 1<<i;
}

void QuteRenderer::glDrawLowRes(){
    bool useDof = (settings.dofStrength>0);
    glDrawOnTextureAndSplash( useDof, performanceSettings.lowResSuperSampling , lowResBuffer );
}

void QuteRenderer::glDrawHiRes(){
    bool useDof = (settings.dofStrength>0);

    // for testing
    //Col oldC = settings.background; settings.background = qmol::Col(0,0,1);

    glDrawOnTextureAndSplash( useDof, performanceSettings.hiResSuperSampling , hiResBuffer );

    //settings.background = oldC;
}


void QuteRenderer::glDrawOnTextureAndSplash(bool useDof, int superSample, glSurface &surface){

    if (overlayOpacity == 0) return;

    maybePrepareShaders();
    if ( (superSample==100) && (!useDof) && (overlayOpacity==1) ) {
        // can draw directly on screen
        glDrawDirect();
    } else {
        // draw to a buffer and splash
        int vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        int w = vp[2]*superSample/100;
        int h = vp[3]*superSample/100;
        int w2 = _minPow2(w);
        int h2 = _minPow2(h);

        render_target colTarget = rt_format(
                    (careForAlpha)?GL_RGBA8:GL_RGB8,
                    rt_linear
        );

        if (surface.width()!=w2 || surface.height()!=h2 || surface.color_target()!=colTarget ) {
            //debug("surface setup");
            surface.setup(
                w2,h2,0,
                colTarget,
                rt_format( GL_DEPTH_COMPONENT24 , rt_nearest )
            );
            //debug("ok");
        }

        surface.push();
        matVp[2]=w; matVp[3]=h;
        glViewport(0,0,w,h);

        glDrawDirect();
        surface.pop();

        if (useDof)
            glSplashWithDepthOfField(surface, w,h , w2,h2, superSample/100.0);
        else
            glSplashFullScreen(surface, w,h , w2,h2, overlayOpacity );
    }

}



Matrix QuteRenderer::molSpace2shadowMapSpaceMatrix() const{
    float m[16] = { lightRot[0], lightRot[1], lightRot[2], 0,
                    lightRot[3], lightRot[4], lightRot[5], 0,
                    lightRot[6], lightRot[7], lightRot[8], 0,
                    0,0,0,1};


    Matrix tr; tr.SetTranslate(lightTra);
    Matrix mv = Matrix(m) * tr ;


    // viewport matrix...
    int vpi[4];
    glGetIntegerv(GL_VIEWPORT, vpi);
    Matrix aa; aa.SetTranslate( Vec((float)vpi[0],(float)vpi[1],0) );
    Matrix bb; bb.SetScale( 0.5f*vpi[2],0.5f*vpi[3],0.5f );
    Matrix cc; cc.SetTranslate( Vec(1,1,1) );
    Matrix vp = aa*bb*cc;

    Matrix tot = vp*mv;

    return tot;
}

Vec QuteRenderer::viewSpaceVec2molSpaceVec(Vec t){
    return mvDirX * t.X() + mvDirY * t.Y() + mvDirZ * t.Z();
}

Vec4 myVec4(Pos p){
    return Vec4(p.X(),p.Y(),p.Z(),1);
}

void QuteRenderer::glCaptureCurrentMatrices(){
    glGetFloatv(GL_MODELVIEW_MATRIX,matMv);
    glGetFloatv(GL_PROJECTION_MATRIX,matPr);
    glGetIntegerv(GL_VIEWPORT, matVp);

    mvDirX = Vec(matMv[0],matMv[4],matMv[8]);
    mvScale = mvDirX.Norm(); mvDirX/=mvScale;
    mvDirY = Vec(matMv[1],matMv[5],matMv[9]).Normalize();
    mvDirZ = mvDirX^mvDirY;


    Matrix mv(matMv);	mv = vcg::Transpose( mv );
    Matrix pr(matPr);	pr = vcg::Transpose( pr );
    Matrix mvpr = pr * mv;

    clipSizeX = (mvpr * Vec4( mvDirX.X(), mvDirX.Y(), mvDirX.Z() , 0 ) ).X();
    clipSizeY = (mvpr * Vec4( mvDirY.X(), mvDirY.Y(), mvDirY.Z() , 0 ) ).Y();


    // find zMin and zMax (screen space)
    Matrix m  = currentTotalMatrix();

    Pos p = m * (ds.barycenter+mvDirZ*ds.radius);
    Pos q = m * (ds.barycenter-mvDirZ*ds.radius);

    /*Vec4 p = m * myVec4(shape.center+mvDirZ*shape.radius);
    Vec4 q = m * myVec4(shape.center-mvDirZ*shape.radius);*/
    zMin = p.Z();
    zMax = q.Z();
    if (zMin>zMax) std::swap(zMin,zMax);
    //debug("Zmin = %f, zMax = %f (%f)",zMin,zMax,q.X() );
}


Matrix QuteRenderer::currentTotalMatrixInvTransp() const{
    Matrix m  = currentTotalMatrix();
    m = vcg::Inverse(m);
    m = vcg::Transpose(m);
    return m;
}

Matrix QuteRenderer::currentRotMatrix() const{
    Matrix mv(matMv);
    mv = vcg::Transpose( mv );
    return mv;  // keeping translation (just lazyness)
}

Matrix QuteRenderer::currentTotalMatrix() const{
    // modelview
    Matrix mv(matMv);
    mv = vcg::Transpose( mv );

    // projection
    Matrix pr(matPr);
    pr = vcg::Transpose( pr );

    // viewport
    Matrix aa; aa.SetTranslate( Vec((float)matVp[0],(float)matVp[1],0) );
    Matrix bb; bb.SetScale( 0.5f*matVp[2],0.5f*matVp[3],0.5f );
    Matrix cc; cc.SetTranslate( Vec(1,1,1) );
    Matrix vp = bb*cc;

    Matrix tot = vp*pr*mv;
    return tot;
}

static Vec _getXYZ(Plane p){
    return Vec( p.X(),p.Y(),p.Z() );
}
static Plane _zeroW(Plane p){
    return Plane( p.X(),p.Y(),p.Z(),0 );
}

float QuteRenderer::simulatedLightingAmbient(){
    return settings.ambientLightAmount;
}


float QuteRenderer::simulatedLightingDiffuse(Vec n){
    float d = (n*settings.viewLightDir);

    float absD = fabs(d);
    if (settings.semModeEnabled) absD = 1.0 - absD;
    absD = absD*(1-settings.flattenDirectLight) + settings.flattenDirectLight;
    if (d>0) return settings.directLightAmount*absD;
    else {
        //float res = settings.directLightAmount*absD;

        return ((!settings.coloredLightEnabled)?
                    settings.oppositeLightAmount*settings.directLightAmount:
                    settings.directLightAmount
                    )
                *absD;
    }

}


float QuteRenderer::simulatedNormDotLight(Vec n){
    return n*settings.viewLightDir;
}

Vec tmpGetCloserTo(Vec tmp){
    float x = tmp[0], y = tmp[1], z=1;
    if (x>0.25) x = 1.0; else if (x<-0.25) x = -1.0; else x=0;
    if (y>0.25) y = 1.0; else if (y<-0.25) y = -1.0; else y=0;
    return Vec( x,y,z ).normalized();
}

bool QuteRenderer::isLightDirTrivial() const{
    // light dir is basically == to view dir
    return settings.viewLightDir.Z()>0.9980;
}

bool QuteRenderer::isShadowMapNeeded() const{
    return ( !isLightDirTrivial() ) &&
            ( (settings.directLightAmount>0) || (settings.shininess>0) ) &&
            ( settings.attenuateShadows < 1 ) &&
            ( !settings.semModeEnabled ) ;
}


void QuteRenderer::glDrawDirect(){

    glClearColor(settings.background[0],settings.background[1],settings.background[2],0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    if (!ds.ball.size()) return;

    maybePrepareShaders();

    directShadowMapEnabled =  isShadowMapNeeded() ;

    if (directShadowMapEnabled) {
        updateShadowmapParam( viewSpaceVec2molSpaceVec( -settings.viewLightDir ) );
        glRenderShadowMap();
    }



    glEnable(GL_DEPTH_TEST);

    bind_program(progFinalBall);

    progFinalBall.set( unif::clipUnitSize, clipSizeX, clipSizeY );

    progFinalBall.set( unif::patchSize,0.5f*(patchSizeTexels-1)/irradianceMapSize);
    progFinalBall.set( unif::saturation, settings.saturation/255.0, 1.0f-settings.saturation);

    if (settings.flattenDirectLight>0) {
        progFinalBall.set( unif::directLightBonus, settings.flattenDirectLight*settings.directLightAmount );
    }

    if (settings.coloredLightEnabled){
        progFinalBall.set(unif::lightColorAmbient,settings.colorAmbientLight );
        progFinalBall.set(unif::lightColorDirect,settings.colorDirectLight );
        progFinalBall.set(unif::lightColorOpposite,settings.colorOppositeLight*settings.oppositeLightAmount );
    }

    if (settings.fogAmount>0) {
        // reminder: fog = depth * fogAm.x + fogAm.y
        progFinalBall.set(unif::fogAmount,settings.fogAmount/(zMax-zMin),-settings.fogAmount*zMin/(zMax-zMin));
        progFinalBall.set(unif::fogColor ,settings.background );
    }

    if (settings.directLightAmount>0){
        progFinalBall.set(unif::lightDir,settings.viewLightDir*settings.directLightAmount*(1.005f-settings.flattenDirectLight));
        if (settings.oppositeLightAmount>0){
            progFinalBall.set(unif::oppositeLight,-settings.oppositeLightAmount);
        }
    }

    if (settings.shininess>0) {
        progFinalBall.set(unif::halfwayDir,(settings.viewLightDir+Vec(0,0,1)).normalized() );
        progFinalBall.set(unif::shine,settings.shininess);
        progFinalBall.set(unif::gloss,settings.glossiness);
        if (settings.oppositeLightAmount>0){
            progFinalBall.set(unif::oppositeHalfwayDir,(-settings.viewLightDir+Vec(0,0,1)).normalized() );
            progFinalBall.set(unif::oppositeShine,settings.shininess*settings.oppositeLightAmount);
        }
    }


    if (settings.borderSize>0){
        progFinalBall.set(unif::borderSize,settings.borderSize);
        progFinalBall.set(unif::borderColorMul,1-settings.borderDarkness);
        progFinalBall.set(unif::borderVariance,settings.borderVariance);
    }

    if (settings.flattenHighLights) {
        progFinalBall.set(unif::shineThreshold, float(qmol::pow(0.5,1.0/settings.glossiness)) );
    }


    if (settings.ambientLightAmount>0) {

        glActiveTexture(GL_TEXTURE0);
        bind_surface_target(*irradianceMap,0 ); // binds texture

        progFinalBall.set(unif::irradianceMap,0);

        /* show approx AO during its computation */
        float percDone = 1.0f*(cumulatedIrradiance)/(performanceSettings.numAoLights); // how much of AO is ready
        const float START = 0.5f; // initially, assume AO is START
        const float K = 0.25f;  // after of AO computation is K%, compensate linearly
        float aoSum, aoMul;
        if (percDone < K) {
            aoSum = START * (1-percDone/K);
            aoMul = 1/K;
        } else {
            aoSum = 0;
            aoMul = 1/percDone ;
        }
        if (settings.flattenAmbientLight>0) {
            aoSum = aoSum * (1-settings.flattenAmbientLight) + settings.flattenAmbientLight;
            aoMul *= (1-settings.flattenAmbientLight);
        }

        progFinalBall.set(unif::aoSum, (aoSum)*settings.ambientLightAmount );
        progFinalBall.set(unif::aoMul, (aoMul)*settings.ambientLightAmount );
    }

    if (matPr[11]==0) {
        // orthogonal view
        progFinalBall.set(unif::scale,-0.5f * mvScale * matPr[10] );
    } else {
        // projection view
        progFinalBall.set(unif::scale,-0.5f * mvScale * matPr[14] );
    }

    if (settings.attenuateShadows>0) {
        progFinalBall.set( unif::shadowAttenuation, settings.attenuateShadows );
    }

    if ( isShadowMapNeeded() ) {
        progFinalBall.set( unif::shadowMap, 1);
        glActiveTexture( GL_TEXTURE1 );

        bind_surface_target( shadowMap, -1 );
        vcg::Matrix44f oneOverShadowMapSize;
        oneOverShadowMapSize.SetScale(1.0/performanceSettings.shadowMapSize,1.0/performanceSettings.shadowMapSize,1.0);
        progFinalBall.set( unif::shadowMatrix,
            (oneOverShadowMapSize*lastUsedShadowMapMatrix*vcg::Inverse(currentTotalMatrix()))
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

        glActiveTexture( GL_TEXTURE0 );
    }


    if (cutPlaneEnabled) {
        cutPlaneScreen = transposedInverseM( currentTotalMatrix() )*cutPlane;
        glSendCutPlaneParam( progFinalBall );
        Vec cutPlaneView = -_getXYZ( currentRotMatrix()*_zeroW(cutPlane) ).normalized();

        // pre-compute local lighting for cut-surfaces
        progFinalBall.set(
            unif::lightoutCut,
            Vec4(
                1.0f*simulatedLightingAmbient(), // ambient occlusion
                1.0f*simulatedLightingDiffuse( cutPlaneView ),
                0.0f,//*simulatedLightingSpecular( cutPlaneView ) // choice: no highlights on cut surfaces (looks bad on flat)
                simulatedNormDotLight(cutPlaneView)
            )
        );
    }

    glSendBallData();
    bind_program(0);
}


void QuteRenderer::glCenterView(){
    glScalef( 1/ds.radius , 1/ds.radius, 1/ds.radius);
    glTranslatef( -ds.barycenter[0],-ds.barycenter[1], -ds.barycenter[2] );
}

