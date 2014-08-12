#version 110

attribute vec3 position;
varying vec4 color;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix*vec4(position.x,position.y,position.z,1.0);
	color = vec4(clamp(position, 0.0, 1.0),1.0);
}