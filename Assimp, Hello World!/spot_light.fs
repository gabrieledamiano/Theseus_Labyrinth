#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normalMap; // Uniform per la normal map
    float shininess;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN; // Matrice TBN ricevuta dal vertex shader

uniform vec3 viewPos;
uniform Material material;

#define NR_SPOT_LIGHTS 20
uniform int activeSpotLights;
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

const float globalAmbientStrength = 0.1;

vec3 CalcSpotLight(SpotLight light, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    // 1. Campionare la normale dalla normal map
    // La normal map contiene normali nello spazio tangente (valori da 0 a 1, mappati a -1 a 1)
    vec3 normalFromMap = texture(material.normalMap, texCoords).rgb;
    vec3 normalTangentSpace = normalize(normalFromMap * 2.0 - 1.0); // Mappa da [0,1] a [-1,1]

    // 2. Trasformare la normale dallo spazio tangente allo spazio mondo
    // Moltiplicando la normale campionata per la matrice TBN otteniamo la normale finale nello spazio mondo
    vec3 N = normalize(TBN * normalTangentSpace); // N è la normale finale nello spazio mondo

    vec3 lightDir = normalize(light.position - fragPos);

    // Calcolo del cutoff dello spotlight
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = max(light.cutOff - light.outerCutOff, 0.00001); // Previene divisione per zero
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Ambient lighting
    vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;

    // Diffuse lighting
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb; // Corretto: light.diffuses -> light.diffuse

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb; // Assumendo specular map è sulla unit 1

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    // Apply attenuation and intensity
    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 viewDir = normalize(viewPos - FragPos);

    // Global ambient light
    vec3 globalAmbient = globalAmbientStrength * texture(material.diffuse, TexCoords).rgb;
    vec3 result = globalAmbient;

    // Sum up all spot lights
    for(int i = 0; i < activeSpotLights; i++)
        result += CalcSpotLight(spotLights[i], FragPos, viewDir, TexCoords);

    FragColor = vec4(result, 1.0);
}