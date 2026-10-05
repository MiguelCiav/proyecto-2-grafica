#version 330 core

out vec4 FragColor;

// Color plano de depuración (normales en cian, AABB en verde, vértices en amarillo)
uniform vec4 debugColor;

void main() {
    FragColor = debugColor;
}
