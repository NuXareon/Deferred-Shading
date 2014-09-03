#version 110

varying vec4 color;
varying vec2 texCoord0;
varying vec3 norm0;

struct ambientLight {
    vec3 color;
    float intensity;
};

struct directionalLight {
    vec3 color;
    float intensity;
    vec3 direction;
};

uniform ambientLight aLight;
uniform directionalLight dLight;
uniform sampler2D sampler;

void main()
{
    //gl_FragColor = color;
    vec4 ambientColor = vec4(aLight.color,1.0)*aLight.intensity;

    float dLightDiffuseFactor = dot(normalize(norm0), -dLight.direction);
    vec4 dLightDiffuseColor;
    if (dLightDiffuseFactor > 0.0) dLightDiffuseColor = vec4(dLight.color,1.0)*dLight.intensity*dLightDiffuseFactor;
    else dLightDiffuseFactor = vec4(0,0,0,0);

    gl_FragColor =  texture2D(sampler, texCoord0.xy)*
                    (ambientColor+dLightDiffuseColor);
}
