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
    progFinalBall.loadSources("./ballFinal");
}























