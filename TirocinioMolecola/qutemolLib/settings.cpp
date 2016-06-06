#include <stdio.h>
#include "settings.h"


namespace qmol {

void RenderingSettings::setDefaults(){

	background = Col(0.3,0.3,0.3);

	saturation = 0.5;
	shininess = 0.0;
	glossiness = 1<<5;

	ambientLightAmount = 0.0;
	directLightAmount = 1.0;
	oppositeLightAmount = 0.0;

	coloredLightEnabled = false;
	colorDirectLight = Col(0,0.5,1.0);
	colorOppositeLight =  Col(1.0,0.5,0.0);
	colorAmbientLight = Col(1,1,1);


	semModeEnabled = false;

	fov = 0;

	dofStrength = 0;
	dofPos = 0;
	dofRange = 1.0;

	fogAmount = 0;

	flattenHighLights = false;
	flattenAmbientLight = flattenDirectLight = 0;

	borderSize  = borderVariance =0; borderDarkness = 1;

	attenuateShadows = 0;

}

void PerformanceSettings::setDefaults(){
    standardIrradianceMapSize = 2048*4; /*2048;*/
    maxIrradianceMapSize = 2048*4;
    minPatchSizeTexels = 11;
    maxPatchSizeTexels = 64 * 10;

	aoShadowMapSize = 512;
    numAoLights = 128; // how many views to cumulate in the irradiance map
	hiResSuperSampling = 200;
    lowResSuperSampling = 100;
	shadowMapSize = 1024;

    usePcf = true; // PCF
    displayListSize = 160000;
}

/*
 * the rest of this file:
 * LOADING AND SAVING TO FILES
 * with lots of quick hacks...
 */



static int toByte( float x ){
	return (x*255.0);
}

static Col fromString(char* tmp){
    //qDebug("Color... '%s'",tmp);
    int r,g,b;
    sscanf(tmp+4, "%x",&b );
    tmp[4] = 0;
    sscanf(tmp+2,"%x", &g );
    tmp[2] = 0;
    sscanf(tmp, "%x", &r );
    //qDebug("Read %x %x %x",r,g,b);
    return Col(r,g,b) / 255.0;
}

#define IO_F( X ) fprintf(f, "%s = %f ;\n", #X, X )
#define IO_B( X ) fprintf(f, "%s = %d ;\n", #X, (X)?1:0 )
#define IO_I( X ) fprintf(f, "%s = %d ;\n", #X, X )
#define IO_C( C ) fprintf(f, "%s = %02x%02x%02x ;\n", #C, toByte(C.X()),toByte(C.Y()),toByte(C.Z()) )
#define IO_V( V ) fprintf(f, "%s = %f , %f , %f ;\n", #V, V.X(),V.Y(),V.Z() )

void RenderingSettings::save(FILE *f) const{

    IO_C( background );

    IO_F( saturation );
    IO_F( shininess  );
    IO_F( glossiness );

    IO_F( ambientLightAmount );
    IO_F( directLightAmount );
    IO_F( oppositeLightAmount );

    IO_B( coloredLightEnabled );
    IO_C( colorDirectLight );
    IO_C( colorAmbientLight );
    IO_C( colorOppositeLight );

    IO_B( semModeEnabled );

    IO_V( viewLightDir );

    IO_I( fov );

    IO_F( dofStrength );
    IO_F( dofPos );
    IO_F( dofRange );

    IO_F( fogAmount );

    IO_F( flattenAmbientLight );
    IO_F( flattenDirectLight );
    IO_B( flattenHighLights );

    IO_F( borderSize );
    IO_F( borderVariance );
    IO_F( borderDarkness );

    IO_F( attenuateShadows );

}

void PerformanceSettings::save(FILE *f) const{
    IO_I( standardIrradianceMapSize );
    IO_I( maxIrradianceMapSize );
    IO_I( minPatchSizeTexels );
    IO_I( maxPatchSizeTexels );
    IO_I( numAoLights );
    IO_I( aoShadowMapSize );

    IO_I( shadowMapSize );
    IO_B( usePcf );

    IO_I( hiResSuperSampling );
    IO_I( lowResSuperSampling );
    IO_I( displayListSize );
}


#undef IO_F
#undef IO_B
#undef IO_I
#undef IO_C
#undef IO_V

#define IO_F( X ) fscanf(f, #X " = %f ;\n", &X )
#define IO_B( X ) fscanf(f, #X " = %d ;\n", &tmpi ); X = (tmpi!=0)
#define IO_I( X ) fscanf(f, #X " = %d ;\n", &X )
#define IO_C( C ) fscanf(f, #C " = %s ;\n", tmps ); C = fromString(tmps)
#define IO_V( V ) fscanf(f, #V " = %f , %f , %f ;\n", &(V.X()),&(V.Y()),&(V.Z()) )


void RenderingSettings::load(FILE *f){
	int tmpi;
	char tmps[2048];

    IO_C( background );

    IO_F( saturation );
    IO_F( shininess  );
    IO_F( glossiness );

    IO_F( ambientLightAmount );
    IO_F( directLightAmount );
    IO_F( oppositeLightAmount );

    IO_B( coloredLightEnabled );
    IO_C( colorDirectLight );
    IO_C( colorAmbientLight );
    IO_C( colorOppositeLight );

    IO_B( semModeEnabled );

    IO_V( viewLightDir );

    IO_I( fov );

    IO_F( dofStrength );
    IO_F( dofPos );
    IO_F( dofRange );

    IO_F( fogAmount );


    IO_F( flattenAmbientLight );
    IO_F( flattenDirectLight );
    IO_B( flattenHighLights );

    IO_F( borderSize );
    IO_F( borderVariance );
    IO_F( borderDarkness );

    IO_F( attenuateShadows );
}

void PerformanceSettings::load(FILE* f) {
    int tmpi;
    //char tmps[2048];

    IO_I( standardIrradianceMapSize );
    IO_I( maxIrradianceMapSize );
    IO_I( minPatchSizeTexels );
    IO_I( maxPatchSizeTexels );
    IO_I( numAoLights );
    IO_I( aoShadowMapSize );

    IO_I( shadowMapSize );
    IO_B( usePcf );

    IO_I( hiResSuperSampling );
    IO_I( lowResSuperSampling );
    IO_I( displayListSize );
}

} // namespace
