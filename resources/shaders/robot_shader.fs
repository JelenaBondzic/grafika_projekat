#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

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

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
// in vec4 Color;

in vec4 ambColor;
in vec4 diffColor;
in vec4 specColor;

uniform PointLight pointLight;
uniform DirLight dirLight;
uniform Material material;

uniform vec3 viewPosition;
// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * material.texture_ambient1.xyz; //vec3(texture(material.texture_ambient1, TexCoords));
    vec3 diffuse = light.diffuse * diff * material.texture_diffuse1.xyz;//vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.texture_specular1.xyz;//vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *=  attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return diffuse;
    //return (0.1*ambient + diffuse + 0.1*specular);
}

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


void main()
{
    Material material;
    material.texture_diffuse1 = diffColor;
    material.texture_ambient1 = ambColor;
    material.texture_specular1 = specColor;

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcPointLight(pointLight, normal, FragPos, viewDir);
    result = CalcDirLight(dirLight, normal, viewDir);
    FragColor = material.texture_diffuse1;
//     FragColor = vec4(result, 1.0);
}