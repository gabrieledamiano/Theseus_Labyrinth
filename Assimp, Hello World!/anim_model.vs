#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
uniform mat4 finalBoneMatrices[MAX_BONES];

void main()
{
    mat4 totalBoneTransform = mat4(0.0);
    for(int i = 0; i < 4; i++)
    {
        if(aBoneIDs[i] == -1) 
            continue;
        if(aBoneIDs[i] >= MAX_BONES)
        {
            totalBoneTransform = mat4(1.0);
            break;
        }
        totalBoneTransform += finalBoneMatrices[aBoneIDs[i]] * aWeights[i];
    }

    vec4 pos = model * totalBoneTransform * vec4(aPos, 1.0);
    gl_Position = projection * view * pos;
    FragPos = vec3(pos);
    
    mat3 normalMatrix = mat3(transpose(inverse(model * totalBoneTransform)));
    Normal = normalMatrix * aNormal;
    TexCoords = aTexCoords;
}