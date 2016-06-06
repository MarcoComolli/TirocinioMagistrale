#ifndef QUTERENDERER_H
#define QUTERENDERER_H

#include <qutemolLib/baseTypes.h>
#include <dynamicshape.h>
#include <spatialgrid.h>
#include <qutemolLib/settings.h>
#include <qutemolLib/glSurface.h>
#include <qutemolLib/glProgram.h>

using namespace qmol;
class QuteRenderer{


public:
    QuteRenderer(DynamicShape &ds);

    DynamicShape ds;
    PairStatistics ps;
    SpatialGrid grid;

    void geometryChanged();
    void updatePatchSizes();

    void maybePrepareDisplayList( FeedbackFunctionT f = NULL );

    /* rendering functions */
    /* ******************* */

    void glCenterView();
    void glCaptureCurrentMatrices(); // call before any rendering

    void glDrawDirect();
    void glDrawHiRes();
    void glDrawLowRes();

    void glResetIrradianceMap();
    bool glProgressIrradianceMap(int nSteps); // returns true if more needed
    bool isIrradianceMapFinished() const;


    /* rendering performance settings */
    /* ****************************** */
    const PerformanceSettings& getPerformanceSettings() const;
    void setPerformanceSettings (PerformanceSettings s);

    void enableShadowMapPCF(bool on);
    void setShadowMapSize(int i);



    /* rendering settings */
    /* ****************** */

    RenderingSettings settings; // just set at will, and redeaw
    bool cutPlaneEnabled;
    Plane cutPlane; // view space; used in shadowmap rendering and final rendering
    bool careForAlpha; // care to produce good alpha... useful for screenshots


    void loadShadersSources();

private:

    PerformanceSettings performanceSettings;

    void updateAllDefinesInShaders();

    float overlayOpacity; // if <1 it means that this rendered has to be splash over an existing image

    qmol::glProgram
        progFinalBall,
        progBakeAoBall,
        progShadowBall,
        progDepthOfField;

    glSurface
        *irradianceMap,
        aoShadowMap,
        shadowMap,
        hiResBuffer,
        lowResBuffer;

    /* other private internal functions */
    /* -------------------------------- */

    Plane cutPlaneScreen; // in screen space!
    void glSendBallData( );

    // from 1 to 200...  100 => 1-on-1.
    void glDrawOnTextureAndSplash( bool withDof, int percent, glSurface &s);

    void maybePrepareShaders();
    void maybePrepareShader(qmol::glProgram& p);
    void prepareBuffers();

    void glRenderOnIrradianceMap();
    void glRenderAoShadowMap();
    void glRenderShadowMap();
    void glRenderDepthMap(glSurface& target, int res, bool doubleSided, bool bilinear);

    bool isShadowMapNeeded() const;
    bool isLightDirTrivial() const;
    bool directShadowMapEnabled;

    int cumulatedIrradiance; // how much irradiance has been cumulated in irradiance map
    int patchSizeTexels;  // current size of patches
    int irradianceMapSize; // current size of Irrandiance Map

    std::vector< Vec > randomDir;
    void rollRandomDirs();

    // captured matrices
    float matMv[16];
    float matPr[16];
    int matVp[4];
    Vec mvDirX, mvDirY, mvDirZ; // model-view DirX, Y and Z
    float clipSizeX, clipSizeY; //
    float mvScale;
    float zMin, zMax;

    // parameters for shadowmap
    float lightRot[9]; // includes scaling!
    Vec   lightTra;
    Vec   lightDir;
    float lightScale;

    void flipLight();
    void flipCutPlane();

    void glSplashFullScreen(glSurface& glSurface, int w, int h , int w2, int h2, float alpha);
    void glSplashWithDepthOfField(glSurface& glSurface, int w, int h , int w2, int h2, float multisample);
    void glSendShadowmapParam(qmol::glProgram &p) const;
    void glSendCutPlaneParam(qmol::glProgram &p) const;

    void updateShadowmapParam(Vec lightdir);

    // total matrix = model-view-projection-viewport matrix
    Matrix currentTotalMatrix() const;
    Matrix currentTotalMatrixInvTransp() const;
    Matrix currentTotalMatrixInv() const;
    // total shadowmap matrix: from mol-space to shadowmap (screen-)space
    Matrix molSpace2shadowMapSpaceMatrix() const;
    Matrix currentRotMatrix() const;
    Vec viewSpaceVec2molSpaceVec(Vec t);

    // simulated (i.e. CPU) lighting (n is in view space)
    float simulatedLightingAmbient();
    float simulatedLightingDiffuse(Vec n);
    float simulatedLightingSpecular(Vec n);
    float simulatedNormDotLight(Vec n);

    bool dlBallReady;
    bool shadersReady;

    // display lists
    int dlBallsCount;
    unsigned int dlBalls;

    friend class Precomp;

};


#endif // QUTERENDERER_H
