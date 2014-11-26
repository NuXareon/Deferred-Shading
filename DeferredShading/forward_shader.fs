#version 330 compatibility

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

uniform ambientLight aLight;
uniform directionalLight dLight;
uniform int nLights;
uniform sampler2D sampler;
uniform samplerBuffer lightsTexBuffer;

void main()
{
    vec3 normal = normalize(norm0);
    //Ambient Light
    vec4 ambientColor = vec4(aLight.color,1.0)*aLight.intensity;
    //Directional Light
    float dLightDiffuseFactor = dot(normal, -dLight.direction);
    vec4 dLightDiffuseColor;
    if (dLightDiffuseFactor > 0.0) dLightDiffuseColor = vec4(dLight.color,1.0)*dLight.intensity*dLightDiffuseFactor;
    else dLightDiffuseColor = vec4(0,0,0,0);
    //Point Lights
    vec4 pTotalLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
    for (int i = 0; i < nLights ; i++) {
        float lRadius = texelFetch(lightsTexBuffer,i*4+1).y;
        vec3 lPosition = texelFetch(lightsTexBuffer, i*4+2).xyz;

        vec3 pDirection = position0 - lPosition;
        float pDistance = length(pDirection);

        if (pDistance <= lRadius) {
            float lIntensity = texelFetch(lightsTexBuffer,i*4+1).x;
            vec3 lColor = texelFetch(lightsTexBuffer,i*4).rgb;
            vec3 lAttenuation = texelFetch(lightsTexBuffer, i*4+3).xyz;

            pDirection = normalize(pDirection);
            float pLightDiffuseFactor = dot(normal, -pDirection);

            vec4 pLightDiffuseColor;
            if (pLightDiffuseFactor > 0.0) pLightDiffuseColor = vec4(lColor,1.0)*lIntensity*pLightDiffuseFactor;
            else pLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
        
            float pAttenuation = lAttenuation.x + lAttenuation.y*pDistance + lAttenuation.z*pDistance*pDistance;
            pTotalLightDiffuseColor += pLightDiffuseColor/pAttenuation;
        }
    }
    
    gl_FragColor =  texture2D(sampler, texCoord0.xy)*
                    (ambientColor+dLightDiffuseColor+pTotalLightDiffuseColor);
}
