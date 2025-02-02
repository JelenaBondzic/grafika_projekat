#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in vec4 inAmbColor;
layout (location = 6) in vec4 inDiffColor;
layout (location = 7) in vec4 inSpecColor;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

out vec4 ambColor;
out vec4 diffColor;
out vec4 specColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
    TexCoords = aTexCoords;

//    Color = inAmbColor;
    ambColor = inAmbColor;
    diffColor = inDiffColor;
    specColor = inSpecColor;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}