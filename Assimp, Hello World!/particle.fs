#version 330 core
out vec4 FragColor;

void main()
{
    // Effetto cerchio sfumato. I punti più lontani dal centro sono più trasparenti.
    float distance = length(2.0 * gl_PointCoord - 1.0);
    if(distance > 1.0)
        discard; // Scarta i pixel fuori dal cerchio

    // Il colore del fuoco (arancione/giallo) con una trasparenza che diminuisce verso i bordi
    FragColor = vec4(1.0, 0.6, 0.0, 1.0 - distance);
}