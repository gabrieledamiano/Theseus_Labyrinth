#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;   // Nuovo attributo per la tangente
layout (location = 4) in vec3 aBitangent;  // Nuovo attributo per la bitangente

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN; // La matrice Tangent-Bitangent-Normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calcola la posizione del frammento nello spazio mondo
    vec3 fragPosWorld = vec3(model * vec4(aPos, 1.0));
    FragPos = fragPosWorld;
    TexCoords = aTexCoords;

    // Calcola la normale nello spazio mondo
    // È importante usare la matrice normale (trasposta dell'inversa del modello)
    // per trasformare le normali correttamente.
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Trasforma i vettori tangente e bitangente nello spazio mondo
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);

    // Costruisci la matrice TBN
    // Ogni colonna della matrice è un vettore dello spazio TBN nello spazio mondo
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(fragPosWorld, 1.0);
}