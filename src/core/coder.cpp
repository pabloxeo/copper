#include <string>
#include "./coder.h"

void Coder::generateShaderCode() {
    shaderCode =
        "struct Uniforms {\n"
        "    aspect_ratio: f32,\n"
        "};\n"
        "@group(0) @binding(0) var<uniform> uniforms: Uniforms;\n"
        "struct VertexOutput {\n"
        "    @builtin(position) clip_pos: vec4<f32>,\n"
        "    @location(0) uv: vec2<f32>\n"
        "};\n"
        "struct DistanceColor {\n"
        "    distance: f32,\n"
        "    color: vec3<f32>\n"
        "};\n"
        "const VIEW_POSITION: vec3<f32> = vec3(0.0, 0.0, -1.0);\n"
        "const FOV: f32 = 1.0;\n"
        "@vertex\n"
        "fn vertexMain(@builtin(vertex_index) i : u32) -> VertexOutput {\n"
        "    var output: VertexOutput;\n"
        "    var p = vec2<f32>(0.0, 0.0);\n"
        "    switch (i) {\n"
        "        case 0u, 4u: { p = vec2<f32>(-1.0, -1.0); output.uv = vec2<f32>(0.0, 1.0); }\n"
        "        case 1u: { p = vec2<f32>(-1.0, 1.0); output.uv = vec2<f32>(0.0, 0.0); }\n"
        "        case 2u, 5u: { p = vec2<f32>(1.0, 1.0); output.uv = vec2<f32>(1.0, 0.0); }\n"
        "        case 3u: { p = vec2<f32>(1.0, -1.0); output.uv = vec2<f32>(1.0, 1.0); }\n"
        "        default: { p = vec2<f32>(0.0, 0.0); output.uv = vec2<f32>(0.0, 0.0); }\n"
        "    }\n"
        "    output.clip_pos = vec4<f32>(p, 0.0, 1.0);\n"
        "    return output;\n"
        "}\n"
        "const MAX_MARCHING_STEPS: i32 = 100;\n"
        "const MIN_DISTANCE: f32 = 0.001;\n"
        "const MAX_DISTANCE: f32 = 100.0;\n"
        "@fragment\n"
        "fn fragmentMain(in: VertexOutput) -> @location(0) vec4<f32> {\n"
        "   var uv: vec2<f32> = in.uv;\n"
        "   uv = uv * 2.0 - 1.0;\n"
        "   uv.y = -(uv.y * 1.0 / uniforms.aspect_ratio);\n"
        "   let ro = VIEW_POSITION;\n"
        "   let rd = normalize(vec3(uv, FOV));\n"
        "   return ray_march(ro, rd);\n"
        "}\n"
        "fn sdf(pos: vec3<f32>) -> DistanceColor {\n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0));\n";

    for (size_t i = 0; i < spheres.size(); ++i) {
        const auto& s = spheres[i];
        shaderCode += "    let sphere" + std::to_string(i) + " = DistanceColor(length(pos - vec3<f32>(" +
            std::to_string(s.x) + ", " + std::to_string(s.y) + ", " + std::to_string(s.z) +
            ")) - " + std::to_string(s.radius) + ", vec3<f32>(" + std::to_string(s.r) + ", " +
            std::to_string(s.g) + ", " + std::to_string(s.b) + "));\n";

        if (s.operation == "union") {
            shaderCode += "    result = opSmoothUnion(result, sphere" + std::to_string(i) + ", 0.5);\n";
        } else if (s.operation == "intersection") {
            shaderCode += "    result = opSmoothIntersect(result, sphere" + std::to_string(i) + ", 0.5);\n";
        } else if (s.operation == "subtract") {
            shaderCode += "    result = opSmoothSubtract(result, sphere" + std::to_string(i) + ", 0.5);\n";
        }
    }

    shaderCode += "    return result;\n"
        "}\n"
        "fn ray_march(ro: vec3<f32>, rd: vec3<f32>) -> vec4<f32> {\n"
        "   var total_distance: f32 = 0.0;\n"
        "   for (var i: i32 = 0; i < MAX_MARCHING_STEPS && total_distance < MAX_DISTANCE; i++) {\n"
        "       let pos = ro + rd * total_distance;\n"
        "       let dc = sdf(pos);\n"
        "       if (dc.distance < MIN_DISTANCE){\n"
        "           return vec4<f32>(dc.color, 1.0);\n"
        "       }\n"
        "       total_distance += dc.distance;\n"
        "   }\n"
        "   return vec4<f32>(0.1, 0.1, 0.1, 1.0);\n"
        "}\n"
        "fn opSmoothUnion(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 + 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d2.distance, d1.distance, h) - k * h * (1.0 - h);\n"
        "    let blended_color = mix(d2.color, d1.color, h); // Blend colors\n"
        "    return DistanceColor(blended_distance, blended_color);\n"
        "}\n"
        "fn opSmoothIntersect(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 - 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d2.distance, d1.distance, h) + k * h * (1.0 - h);\n"
        "    let blended_color = mix(d2.color, d1.color, h); // Blend colors\n"
        "    return DistanceColor(blended_distance, blended_color);\n"
        "}\n"
        "fn opSmoothSubtract(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 - 0.5 * (d2.distance + d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d1.distance, -d2.distance, h) + k * h * (1.0 - h);\n"
        "    let blended_color = mix(d1.color, vec3<f32>(0.0, 0.0, 0.0), h); // Subtract color\n"
        "    return DistanceColor(blended_distance, blended_color);\n"
        "}\n";
}

void Coder::addSphere(float x, float y, float z, float radius, float r, float g, float b, const std::string& operation) {
    spheres.push_back({x, y, z, radius, r, g, b, operation});
    generateShaderCode();
}

void Coder::clearSpheres() {
    spheres.clear();
    generateShaderCode();
}

Coder::Coder() {
    generateShaderCode();
}