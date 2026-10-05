#version 330 core

out vec4 FragColor;

// Color normalizado RGB para Modo Global y Local
uniform vec3 codeColor;

// Modo de picking: 0 = Global / Local (usa codeColor), 1 = Triángulo (usa gl_PrimitiveID)
uniform int pickingMode;
uniform int baseTriangleID;

void main() {
    if (pickingMode == 1) {
        // En modo Triángulo: el ID único de 24 bits es baseTriangleID + gl_PrimitiveID + 1
        uint triangleID = uint(baseTriangleID) + uint(gl_PrimitiveID) + 1u;
        float r = float(triangleID & 0xFFu) / 255.0;
        float g = float((triangleID >> 8u) & 0xFFu) / 255.0;
        float b = float((triangleID >> 16u) & 0xFFu) / 255.0;
        FragColor = vec4(r, g, b, 1.0);
    } else {
        FragColor = vec4(codeColor, 1.0);
    }
}
