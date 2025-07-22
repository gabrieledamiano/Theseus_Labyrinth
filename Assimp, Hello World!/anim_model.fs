#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

// --- NUOVE RIGHE ---
uniform bool isEnraged;
uniform vec3 rageColor = vec3(0.8, 0.0, 0.0); // Colore del bagliore

void main()
{
    // Semplice illuminazione per testare
    vec3 lightDir = normalize(vec3(0.5, -1.0, -0.5));
    vec3 norm = normalize(Normal);
    
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 result = (ambient + diffuse) * texture(texture_diffuse1, TexCoords).rgb;
    
    // --- NUOVE RIGHE ---
    // Se il Minotauro è infuriato, aggiungi il colore del bagliore al risultato finale
    if (isEnraged) {
        result += rageColor * 0.3; // Puoi cambiare 0.4 per aumentare/diminuire l'intensità
    }

    FragColor = vec4(result, 1.0);
}