#version 110

varying vec2 texCoord;
uniform sampler2D texture;

void main()
{
    gl_FragDepth = texture2D(texture,texCoord);
}