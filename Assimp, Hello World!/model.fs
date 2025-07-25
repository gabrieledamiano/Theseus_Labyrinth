#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

// --- NUOVO UNIFORM ---
// Aggiungiamo una variabile per controllare la trasparenza generale. 
// Il valore di default è 1.0 (completamente opaco).
uniform float alpha = 1.0;

void main()
{
    // Illuminazione di base (invariata)
    vec3 lightDir = normalize(vec3(0.5, -1.0, -0.5));
    vec3 norm = normalize(Normal);
    
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec4 texColor = texture(texture_diffuse1, TexCoords);
    vec3 result = (ambient + diffuse) * texColor.rgb;

    // --- MODIFICA ---
    // Impostiamo il colore finale usando l'alpha della texture moltiplicato per il nostro alpha personalizzato.
    FragColor = vec4(result, texColor.a * alpha);
}