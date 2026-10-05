#version 330 core

out vec4 FragColor;

// Color normalizado RGB que codifica el ID entero de la entidad
uniform vec3 codeColor;

void main() {
    FragColor = vec4(codeColor, 1.0);
}
