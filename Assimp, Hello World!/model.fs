#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

void main()
{
    // Illuminazione di base per iniziare
    vec3 lightDir = normalize(vec3(0.5, -1.0, -0.5));
    vec3 norm = normalize(Normal);
    
    // Ambientale
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0);

    // Diffusa
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 result = (ambient + diffuse) * texture(texture_diffuse1, TexCoords).rgb;
    FragColor = vec4(result, 1.0);
}