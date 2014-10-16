#version 330 compatibility

varying vec2 texCoord;
uniform sampler2D texture;

void main()
{
    gl_FragDepth = texture2D(texture,texCoord);
}