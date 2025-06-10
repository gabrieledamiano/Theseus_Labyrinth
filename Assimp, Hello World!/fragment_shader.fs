#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D myTextureSampler;
uniform vec3 objectColor;
uniform bool useTexture;

const float ambientStrength = 0.15;
vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

void main()
{
    vec4 texColor = texture(myTextureSampler, TexCoord);
    vec3 finalColor;
    
    if (useTexture) {
        finalColor = ambient + (texColor.rgb * objectColor);
    } else {
        finalColor = ambient + objectColor;
    }

    FragColor = vec4(finalColor, 1.0);
}