#version 330 core

in vec2 TexCoord;  // Incoming texture coordinates
out vec4 FragColor; // Final color output

uniform vec3 rayColor;  // Color of the ray-traced pixel

void main()
{
    FragColor = vec4(rayColor, 1.0);  // Set final pixel color
}
