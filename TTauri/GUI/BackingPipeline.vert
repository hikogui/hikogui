#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(100.0, 100.0),
    vec2(200.0, 100.0),
    vec2(100.0, 150.0)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

vec2 convertToViewport(vec2 windowPosition) {
    return (windowPosition * pushConstants.viewportScale) - vec2(1.0, 1.0);
}

void main() {
    vec2 windowPosition = positions[gl_VertexIndex];
    vec2 viewportPosition = convertToViewport(windowPosition);

    gl_Position = vec4(viewportPosition, 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
