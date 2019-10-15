#version 330 core

struct Light {
    vec3 position;
    vec3 diffuse;
    vec3 ambient;
};


out vec4 FragColor;

in vec3 fragNorm;
in vec3 fragPos;
in vec2 fragUV;

uniform vec3 viewPos;
uniform Light light;
uniform sampler2D objectTex;
uniform bool diffuseOn; 
void main()
{
    if (diffuseOn) {
        vec3 lightDir = normalize(light.position - fragPos);
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 norm = normalize(fragNorm);
        vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.diffuse * vec3(texture(objectTex, fragUV));
        vec3 ambient = light.ambient * vec3(texture(objectTex, fragUV));
        FragColor = vec4(ambient + diffuse, 1.0);
    } else {
    FragColor = texture(objectTex, fragUV);
    }
}