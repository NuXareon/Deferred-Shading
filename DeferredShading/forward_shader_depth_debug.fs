#version 330 compatibility

varying vec2 texCoord;
uniform sampler2D texture;

void main()
{
    gl_FragColor = texture2D(texture,texCoord);
    gl_FragColor.r = pow(gl_FragColor.x, 8.0);
    gl_FragColor.g = pow(gl_FragColor.y, 8.0);
    gl_FragColor.b = pow(gl_FragColor.z, 8.0);
}