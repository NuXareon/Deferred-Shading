#version 330 compatibility

attribute vec3 position;
attribute vec2 texCoord;
attribute vec3 norm;

varying vec2 texCoord0;
varying vec3 norm0;
varying vec3 position0;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix*vec4(position.x,position.y,position.z,1.0);
	texCoord0 = texCoord;
	norm0 = norm;
	position0 = position;
}