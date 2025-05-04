#version 330 core

layout(location = 0) in vec2 aPos;  // Vertex position input
out vec2 TexCoord;                   // Output texture coordinate

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);  // Transform positions
    TexCoord = aPos * 0.5 + 0.5;  // Map to [0, 1] for texture coordinates
}
