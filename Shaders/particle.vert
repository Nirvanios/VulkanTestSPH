#version 450

struct ParticleRecord {
    vec4 position;
    vec4 color;
    vec4 velocity;
    float mass;
    float density;
    float pressure;
    float particleRadius;
};

layout(push_constant) uniform modelType{
    int isParticle;
};
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(std430, binding = 2) buffer positionBuffer{
    ParticleRecord particleData[];
};

layout(std430, binding = 3) buffer floorBuffer{
    ParticleRecord floorData[];
};

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPosition;

void main() {
    gl_PointSize = 3;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    if(isParticle == 1){
        vec3 movedPos = inPosition * 0.1 * particleData[gl_InstanceIndex].particleRadius + particleData[gl_InstanceIndex].position.xyz;
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(movedPos, 1.0);
        fragColor = particleData[gl_InstanceIndex].color.xyz;
        fragPosition = vec3(ubo.model * vec4(movedPos, 1.0));
    }
    else if(isParticle == 2){
        vec3 movedPos = inPosition + floorData[gl_InstanceIndex].position.xyz;
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(movedPos, 1.0);
        fragColor = floorData[gl_InstanceIndex].color.xyz;
        fragPosition = vec3(ubo.model * vec4(movedPos, 1.0));
    }
    else {
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
        fragColor = inColor;
        fragPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    }
}