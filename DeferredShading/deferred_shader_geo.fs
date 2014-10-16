#version 330 compatibility

varying vec2 texCoord0;
varying vec3 norm0;
varying vec3 position0;

uniform sampler2D sampler;


void main()
{
    gl_FragData[0] = vec4(position0,1.0);
    gl_FragData[1] = vec4(normalize(norm0),1.0);
    gl_FragData[2] = vec4(texture2D(sampler, texCoord0.xy).xyz,1.0);
}