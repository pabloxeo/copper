#include <string>
#include "./coder.h"

void Coder::generateShaderCode() {
    shaderCode =
        "struct Uniforms {\n"
        "    mvp_matrix: mat4x4<f32>,\n"
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
        "fn mat4_to_mat3(m: mat4x4<f32>) -> mat3x3<f32> {\n"
        "    return mat3x3<f32>(\n"
        "        m[0].xyz,\n"
        "        m[1].xyz,\n"
        "        m[2].xyz\n"
        "    );\n"
        "}\n"
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
        // extract the camera eye
        "   let cam = transpose(uniforms.mvp_matrix);\n"
        "   let right = cam[0].xyz;\n"
        "   let up = cam[1].xyz;\n"
        "   let forward = cam[2].xyz;\n"
        "   let eye = cam[0].w * right + cam[1].w * up + cam[2].w * forward;\n"

        "   var uv: vec2<f32> = in.uv;\n"
        "   uv = uv * 2.0 - 1.0;\n"
        "   uv.y = (uv.y * 1.0 / uniforms.aspect_ratio);\n"
        "   uv.x = -uv.x;\n"
        // "   let ro = vec3<f32>(0.0, 0.0, -10.0);\n"
        "   let ro = eye;\n"
        "   let rd = normalize(mat4_to_mat3(cam) * vec3(uv, FOV));\n"
        "   return ray_march(ro, rd);\n"
        "}\n"
        "fn sdf(pos: vec3<f32>) -> DistanceColor {\n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0));\n";

    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& object = objects[i];
        if(object.type == "sphere") {
            shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_sphere(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), " +
                std::to_string(object.size[0]) + ", vec3<f32>(" + std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "));\n";
        } else if(object.type == "box") {
            shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_box(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), vec3<f32>(" +
                std::to_string(object.size[0]) + ", " + std::to_string(object.size[1]) + ", " + std::to_string(object.size[2]) + "), vec3<f32>(" +
                std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "));\n";
        }

        if (i == 0) {
            shaderCode += "result = " + object.type + std::to_string(i) + ";\n";
        } else if (object.operation == "union") {
            shaderCode += "result = opSmoothUnion(result, " + object.type + std::to_string(i) + ", 0.5);";
        } else if (object.operation == "intersection") {
            shaderCode += "result = opSmoothIntersect(result, " + object.type + std::to_string(i) + ", 0.5);";
        } else if (object.operation == "subtract") {
            shaderCode += "result = opSmoothSubtract(result, " + object.type + std::to_string(i) + ", 0.5);";
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
        "}\n"
        "fn sdf_sphere(pos: vec3<f32>, center: vec3<f32>, radius: f32, color: vec3<f32>) -> DistanceColor {\n"
        "    let distance = length(pos - center) - radius;\n"
        "    return DistanceColor(distance, color); // Red color for sphere\n"
        "}\n"
        "fn sdf_box(pos: vec3<f32>, center: vec3<f32>, size: vec3<f32>, color: vec3<f32>) -> DistanceColor {\n"
        "    let d = abs(pos - center) - size;\n"
        "    let distance = length(max(d, vec3<f32>(0.0))) + min(max(d.x, max(d.y, d.z)), 0.0);\n"
        "    return DistanceColor(distance, color);\n"
        "}\n"
        ;
}

void Coder::addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation) {
    std::string type = "sphere";
    std::vector<float> vsize ;
    vsize.push_back(size);
    objects.push_back({x, y, z, r, g, b, vsize, type, operation});
    generateShaderCode();
}

void Coder::addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation) {
    std::string type = "box";
    objects.push_back({x, y, z, r, g, b, size, type, operation});
    generateShaderCode();
}

void Coder::clearObjects() {
    objects.clear();
    generateShaderCode();
}

Coder::Coder() {
    generateShaderCode();
}