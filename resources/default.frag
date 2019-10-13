#version 330 core
out vec4 FragColor;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;

uniform sampler2D objectTex;

void main()
{
    FragColor = texture(objectTex, fragUV);
    // FragColor = vec4(1.0);
}