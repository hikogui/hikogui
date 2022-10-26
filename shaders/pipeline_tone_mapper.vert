#version 450
#extension GL_ARB_separate_shader_objects : enable

//              + 1,-3
//             /|
//            / |
//           /  |
//          /   |
//         +----+ 1,-1
//        /|    |
//       / |    |
//      /  |    |
//     /   |    |
//    +----+----+ 1,1
//  -3,1 -1,1
vec2 positions[3] = vec2[](
    vec2(-3.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -3.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
