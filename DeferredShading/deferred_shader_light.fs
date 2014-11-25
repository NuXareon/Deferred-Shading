#version 330 compatibility

struct pointLight {
    vec3 color;
    float intensity;
    vec3 position;
    vec3 attenuation;
    float radius;
};

uniform vec2 screenSize;
uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D diffuseBuffer;
uniform pointLight pLight;

void main() 
{
    vec4 pLightDiffuseColor;
    float screenLocX = gl_FragCoord.x/screenSize.x;
    float screenLocY = gl_FragCoord.y/screenSize.y;
    vec2 screenLoc = vec2(screenLocX,screenLocY);
    vec3 position = texture2D(positionBuffer,screenLoc).xyz;
    if (length(position-pLight.position) > pLight.radius) discard;      // discard fragment that are too far away
    else {
        vec3 normal = texture2D(normalBuffer,screenLoc).xyz;
        normal = normalize(normal);
        vec3 diffuse = texture2D(diffuseBuffer,screenLoc).xyz;

        vec4 pTotalLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);
        vec3 pDirection = position - pLight.position;
        float pDistance = length(pDirection);
        pDirection = normalize(pDirection);

        float pLightDiffuseFactor = dot(normal, -pDirection);
        
        if (pLightDiffuseFactor > 0.0) pLightDiffuseColor = vec4(pLight.color,1.0)*pLight.intensity*pLightDiffuseFactor;
        else pLightDiffuseColor = vec4(0.0,0.0,0.0,0.0);

        float pAttenuation = pLight.attenuation[0] + pLight.attenuation[1]*pDistance + pLight.attenuation[2]*pDistance*pDistance;
        pLightDiffuseColor /= pAttenuation;
    
        gl_FragColor = vec4(diffuse, 1.0) * pLightDiffuseColor;
    }
}