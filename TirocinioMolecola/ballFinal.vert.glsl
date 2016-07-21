
uniform float scale;
uniform vec2 saturation; // x = saturation / 255.0. y = 1 - saturation
uniform vec2 clipUnitSize; // sizes of unit sized impostor in clip coords

#if USE_BORDERS
uniform float borderSize;
flat varying float radiusWithBorder;
#endif


attribute vec3 position;
attribute float radius;
attribute vec3 color;
attribute vec2 uv; // patchCenter

varying vec2 cor; // corner point, [-1,+1]^2
flat varying float rad;
flat varying vec3 col;
flat varying vec2 uvVar;

/*
uniform mat4 gl_ModelViewProjectionMatrix; // some compiler does not like this
*/

void main()
{


        float _radius = radius/4096.0;
#if USE_BORDERS
        radiusWithBorder = 1 + borderSize/_radius;
	cor = gl_Vertex.xy * radiusWithBorder;
	radiusWithBorder *= radiusWithBorder;
#else
	cor = gl_Vertex.xy;
#endif
        uvVar = uv/16384.0;
        col = color * saturation.x + saturation.y;

	vec4 clipPos = gl_ModelViewProjectionMatrix * vec4(position,1);
        clipPos.xy += (clipUnitSize*cor) * _radius;
	//clipPos.w -= (clipPos.w)*dot(cor,clipPos.xy)*0.02;// /gl_ProjectionMatrix[2][2];

        gl_Position = clipPos;


        rad = _radius * scale / (gl_Position.w*gl_Position.w) ;
	/*
#if USE_PERSPECTIVE
	// case PERSPECTIVE projection
        rad = -0.5 * _radius * scale * gl_ProjectionMatrix[3][2] / (gl_Position.w*gl_Position.w) ;
#else
	// case ORTOGONAL projection
        rad = -0.5 * _radius * scale * gl_ProjectionMatrix[2][2] ;
#endif
	*/
}
