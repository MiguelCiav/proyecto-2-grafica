#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float alpha;

void main()
{
    // Iluminación ambiental base
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;

    // Iluminación difusa (Modelo de Lambert)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Mezcla con color del objeto y canal alfa
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, alpha);
}
