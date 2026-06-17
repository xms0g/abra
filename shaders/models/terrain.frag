#version 410 core

in TES_OUT
{
   float Height;
} fs_in;

out vec4 fragColor;

void main() {
    float h = (fs_in.Height + 16) / 64.0f;
    fragColor = vec4(h, h, h, 1.0);
}
