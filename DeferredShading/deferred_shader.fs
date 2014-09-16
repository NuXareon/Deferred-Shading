#version 110

varying vec2 texCoord0;
varying vec3 norm0;
varying vec3 position0;

uniform sampler2D sampler;
uniform vec3 minP;
uniform vec3 maxP;

//layout (location = 0) vec3 position;
//layout (location = 1) vec3 normal;
//layout (location = 2) vec3 difuseColor;

void main()
{
    vec3 posScale = vec3((position0.x-minP.x)/(maxP.x-minP.x),(position0.y-minP.y)/(maxP.y-minP.y),(position0.z-minP.z)/(maxP.z-minP.z));
    //gl_FragData[0] = vec4(position0,1.0);
    gl_FragData[0] = vec4(posScale,1.0);
    gl_FragData[1] = vec4(normalize(norm0),1.0);
    gl_FragData[2] = vec4(texture2D(sampler, texCoord0.xy).xyz,1.0);
    //gl_FragData[0] = vec4(1.0,0.0,0.0,1.0);
    //gl_FragData[1] = vec4(0.0,1.0,0.0,1.0);
    //gl_FragData[2] = vec4(0.0,0.0,1.0,1.0);
    //gl_FragData[0] = vec4(1.0,0.0,0.0,1.0);
}