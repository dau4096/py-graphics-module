/* display.example.frag */
#version 460 core

in vec2 fragUV;
out vec4 fragColour;

layout(binding=0) uniform sampler2D imageToShow;

void main() {
    fragColour = vec4(
        texture(imageToShow, fragUV).rgb, 1.0f
    );
}