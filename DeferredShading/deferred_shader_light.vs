#version 330 compatibility

attribute vec3 position;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vec4(position,1.0);
}