#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {
    const float ambientStrength = 0.1f;
    const vec3 lightColor = vec3(1.0f);
    const vec3 lightPosition = vec3(5.0f, 5.0f, 5.0f);
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(fragNormal);
    vec3 lightDirection = normalize(lightPosition - fragPosition);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * fragColor;
    outColor = vec4(result, 1.0);
    //outColor = vec4(fragColor, 1.0f);
}
