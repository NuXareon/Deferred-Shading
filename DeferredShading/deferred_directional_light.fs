#version 330 compatibility

struct ambientLight {
    vec3 color;
    float intensity;
};

struct directionalLight {
    vec3 color;
    float intensity;
    vec3 direction;
};

varying vec2 texCoord;
uniform ambientLight aLight;
uniform directionalLight dLight;
uniform sampler2D normalBuffer;
uniform sampler2D diffuseBuffer;

void main()
{
    vec3 normal = texture2D(normalBuffer,texCoord).xyz;
    normal = normalize(normal);

    //Ambient Light
    vec4 ambientColor = vec4(aLight.color,1.0)*aLight.intensity;
    //Directional Light
    float dLightDiffuseFactor = dot(normal, -dLight.direction);
    vec4 dLightDiffuseColor;
    if (dLightDiffuseFactor > 0.0) dLightDiffuseColor = vec4(dLight.color,1.0)*dLight.intensity*dLightDiffuseFactor;
    else dLightDiffuseColor = vec4(0,0,0,0);

    gl_FragColor =  texture2D(diffuseBuffer, texCoord)*
                (ambientColor+dLightDiffuseColor);
}
