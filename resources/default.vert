#version 330

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNorm;
layout(location = 2) in vec2 UV;

out vec3 fragNorm;
out vec3 fragPos;
out vec2 fragUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos  = vec3(model * vec4(vertexPos, 1.0));
	fragNorm = mat3(transpose(inverse(model))) * vertexNorm;
	fragUV   = UV;
    
	gl_Position = projection * view * vec4(fragPos, 1.0);
}
