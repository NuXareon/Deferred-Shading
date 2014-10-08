#version 110

const int N_MAX_LIGHTS = 100;

varying vec2 texCoord0;
varying vec3 norm0;
varying vec3 position0;

struct ambientLight {
    vec3 color;
    float intensity;
};

struct directionalLight {
    vec3 color;
    float intensity;
    vec3 direction;
};

struct pointLight {
    vec3 color;
    float intensity;
    vec3 position;
    vec3 attenuation;
};

uniform ambientLight aLight;
uniform directionalLight dLight;
uniform pointLight pointLights[N_MAX_LIGHTS];
uniform int nLights;
uniform sampler2D sampler;
uniform float zOffset;

void main()
{
    // Ambient Light
    vec4 ambientColor = vec4(aLight.color,1.0)*aLight.intensity;
    //Diretional Light
    vec3 normal = normalize(norm0);
    float dLightDiffuseFactor = dot(normal, -dLight.direction);
    vec4 dLightDiffuseColor;
    if (dLightDiffuseFactor > 0.0) dLightDiffuseColor = vec4(dLight.color,1.0)*dLight.intensity*dLightDiffuseFactor;
    else dLightDiffuseFactor = vec4(0.0,0.0,0.0,0.0);
    // Point Lights
    vec4 pTotalLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
    for (int i = 0; i < nLights ; i++) {
        vec3 pDirection = position0 - pointLights[i].position;
        float pDistance = length(pDirection);
        pDirection = normalize(pDirection);

        float pLightDiffuseFactor = dot(normal, -pDirection);
        
        vec4 pLightDiffuseColor;
        if (pLightDiffuseFactor > 0.0) pLightDiffuseColor = vec4(pointLights[i].color,1.0)*pointLights[i].intensity*pLightDiffuseFactor;
        else pLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);

        float pAttenuation = pointLights[i].attenuation[0] + pointLights[i].attenuation[1]*pDistance + pointLights[i].attenuation[2]*pDistance*pDistance;
        pTotalLightDiffuseColor += pLightDiffuseColor/pAttenuation;
    }
    
    gl_FragColor =  texture2D(sampler, texCoord0.xy)*
                    (ambientColor+dLightDiffuseColor+pTotalLightDiffuseColor);
    gl_FragDepth = gl_FragCoord.z+zOffset;
}
