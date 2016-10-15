#ifndef RENDERINGOPTIONS_H
#define RENDERINGOPTIONS_H


#include "baseTypes.h"


namespace qmol {

/* qmol::RenderingSettings:
 * a set of values to direct the style in which qmol::Renderer renders a shape.
 */

struct RenderingSettings{

    Col background; // also used as fog color

    /* material */
    float saturation; // if <1, dilute the colors (no matter what that color represents)
    float shininess;  // highlights strenght
    float glossiness; // highlights size

    /* lights */
    float ambientLightAmount;
    float directLightAmount;
    float oppositeLightAmount;

    // colored lights  (also: color of highlights)
    bool coloredLightEnabled; // if false, lights is B&W (colors not used)
    Col colorDirectLight;
    Col colorAmbientLight;
    Col colorOppositeLight;

    bool semModeEnabled; // if true, uses experimental "SEM-like" lighting equation

    Vec viewLightDir; // normalized light dir (in view coordinates)

    /* camera */
    int fov; // stored for convenience, but NOT directly used by qmol::Renderer (just use it to set your own projection matrix)
    // depth of field params
    float dofStrength;
    float dofPos;
    float dofRange;

    float fogAmount;

    /* toon (NPR stuff) */

    float flattenAmbientLight; // when -->1, ambient lights --> constant
    float flattenDirectLight;  // when -->1, direct lights --> constant
    bool flattenHighLights;    // when true, hightlights = yes/no matter

    float borderSize;
    float borderVariance; // if >0, borders are thinner for small z-jumps
    float borderDarkness;

    float attenuateShadows;  // e.g.: if 0.5, 50% of direct lights passes even in shadow
    bool isShadowMapWanted;

    int a1,a2,a3;



    /***********/
    /* methods */
    /***********/

    void setDefaults();

    void save(FILE* f) const;
    void load(FILE* f);
};



/* qmol::PerformanceSettings:
 * a set of values to balance performance/quality of qmol::Renderer,
 * and adapt to specific card limitations and what not.
 */

struct PerformanceSettings{

    /* AO (Ambient Occlusion, i.e. prepocessed irradiance map) */
    int standardIrradianceMapSize; // exceed this one only if patch-size would otherwise be too small
    int maxIrradianceMapSize;  // cannot exceed this one
    int minPatchSizeTexels; // min / max sizes for a patch
    int maxPatchSizeTexels;
    int numAoLights; // how many probe-lights are cumulated to construct the irradiance map
    int aoShadowMapSize; // size of shadowmap used for probe-lights

    /* Shadowmap (used for direct lights): */
    int shadowMapSize;
    bool usePcf; // Percentage Closest Filtering

    /* super/under sampling:
     *   100 = none, 1:1.
     *   >100 use supersampling (antialiasing). E.g. 200 => 2x2 antialiasing.
     *   <100 use undersampling (blurred, but faster). */
    int hiResSuperSampling;  // value used for static images, e.g. screeshot or static scenes
    int lowResSuperSampling; // value used for dynamic images, e.g. when molecule moves

    /* geometry: */
    int displayListSize; // split geometry in chunks this big



    /***********/
    /* methods */
    /***********/

    void setDefaults();

    void save(FILE* f) const;
    void load(FILE* f);

};

} // namespace qmol

#endif //RENDERINGOPTIONS_H
