#version 330 compatibility

//const int N_MAX_LIGHTS = 100;

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
uniform int nLights;
uniform sampler2D sampler;
uniform samplerBuffer lightsTexBuffer;

void main()
{
    //Ambient Light
    vec4 ambientColor = vec4(aLight.color,1.0)*aLight.intensity;
    //Point Lights
    vec3 normal = normalize(norm0);
    vec4 pTotalLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
    for (int i = 0; i < nLights ; i++) {
        vec3 lColor = texelFetch(lightsTexBuffer,i*4).rgb;
        float lIntensity = texelFetch(lightsTexBuffer,i*4+1).r;
        vec3 lPosition = texelFetch(lightsTexBuffer, i*4+2).xyz;
        vec3 lAttenuation = texelFetch(lightsTexBuffer, i*4+3).xyz;

        vec3 pDirection = position0 - lPosition;
        float pDistance = length(pDirection);
        pDirection = normalize(pDirection);

        float pLightDiffuseFactor = dot(normal, -pDirection);

        vec4 pLightDiffuseColor;
        if (pLightDiffuseFactor > 0.0) pLightDiffuseColor = vec4(lColor,1.0)*lIntensity*pLightDiffuseFactor;
        else pLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
        
        float pAttenuation = lAttenuation.x + lAttenuation.y*pDistance + lAttenuation.z*pDistance*pDistance;
        pTotalLightDiffuseColor += pLightDiffuseColor/pAttenuation;
    }
    
    gl_FragColor =  texture2D(sampler, texCoord0.xy)*
                    (ambientColor+pTotalLightDiffuseColor);
}
