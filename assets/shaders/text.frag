#version 330 core
in vec2 texCoord;
in vec3 fragColor;
out vec4 FragColor;

uniform sampler2D fontTexture;

void main()
{
    float alpha = texture(fontTexture, texCoord).r;
    FragColor = vec4(fragColor, alpha);
}
