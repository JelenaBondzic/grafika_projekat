#version 330 core
out vec4 fragColor;

in vec4 outColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    vec4 texture_diffuse1;
    vec4 texture_specular1;
    vec4 texture_ambient1;

    float shininess;
};

uniform DirLight dirLight;
uniform Material material;
uniform vec3 viewPosition;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * material.texture_ambient1.xyz; //vec3(texture(material.texture_ambient1, TexCoords));
    vec3 diffuse = light.diffuse * diff * material.texture_diffuse1.xyz;//vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.texture_specular1.xyz;//vec3(texture(material.texture_specular1, TexCoords).xxx);
    return diffuse;
//     return (0.1*ambient + diffuse + 0.1*specular);
}

void main(){
    fragColor = outColor;
}