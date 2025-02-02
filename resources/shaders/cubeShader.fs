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
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;

    float shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
// in vec4 Color;

uniform PointLight pointLight;
uniform DirLight dirlight;
uniform Material material;
uniform vec3 viewPos;
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
    vec3 ambient = light.ambient * material.ambient.xyz; //vec3(texture(material.texture_ambient1, TexCoords));
    vec3 diffuse = light.diffuse * diff * material.diffuse.xyz;//vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.specular.xyz;//vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *=  attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
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
    vec3 ambient = light.ambient * material.ambient.xyz; //vec3(texture(material.texture_ambient1, TexCoords));
    vec3 diffuse = light.diffuse * diff * material.diffuse.xyz;//vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.specular.xyz;//vec3(texture(material.texture_specular1, TexCoords).xxx);
    return (ambient + diffuse + specular);
}

void main(){
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos-FragPos);

    vec3 result = vec3(0);
    result += CalcPointLight(pointLight, norm, FragPos, viewDir);
    result += CalcDirLight(dirlight, norm, viewDir);

    FragColor = vec4(result, 1.0);
}