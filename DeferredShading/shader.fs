#version 110

varying vec4 color;
varying vec2 texCoord0;

uniform sampler2D sampler;

void main()
{
    //gl_FragColor = color;
    gl_FragColor = texture2D(sampler, texCoord0.st);
}
