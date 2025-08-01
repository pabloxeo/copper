#include "./Coder.h"
#include "./Renderer.h"


void Coder::generateShaderCode() {
    shaderCode =
        "fn sdf(pos: vec3<f32>) -> DistanceColor {\n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0), -1);\n";

    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& object = objects[i];
        if(object.id == selectedObjectId) {
            if(object.type == "sphere") {
                shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_sphere(pos, uniforms.position, uniforms.size[0], uniforms.color, " + std::to_string(object.id) + ");\n";
            } else if(object.type == "box") {
                shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_box(pos, uniforms.position, uniforms.size, uniforms.color, " + std::to_string(object.id) + ");\n";
            }
        } else {
            if(object.type == "sphere") {
                shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_sphere(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), " +
                    std::to_string(object.size[0]) + ", vec3<f32>(" + std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            } else if(object.type == "box") {
                shaderCode += "    let " + object.type + std::to_string(i) + " = sdf_box(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), vec3<f32>(" +
                    std::to_string(object.size[0]) + ", " + std::to_string(object.size[1]) + ", " + std::to_string(object.size[2]) + "), vec3<f32>(" +
                    std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            }
        }

        if (i == 0) {
            shaderCode += "result = " + object.type + std::to_string(i) + ";\n";
        } else if (object.operation == "smoothunion") {
            shaderCode += "result = opSmoothUnion(result, " + object.type + std::to_string(i) + ", 0.6);";
        } else if (object.operation == "intersection") {
            shaderCode += "result = opSmoothIntersect(result, " + object.type + std::to_string(i) + ", 0.6);";
        } else if (object.operation == "smoothsubtract") {
            shaderCode += "result = opSmoothSubtract(result, " + object.type + std::to_string(i) + ", 0.6);";
        }else if (object.operation == "union"){
            shaderCode += "result = opUnion(result, " + object.type + std::to_string(i) + ");\n";
        } else if (object.operation == "subtract") {
            shaderCode += "result = opSubtract(result, " + object.type + std::to_string(i) + ");\n";
        }
    }

    shaderCode +=
        "    if (uniforms.floor == 1) {\n"
        "       let floor = sdf_plane(pos);\n"
        "       if(floor.distance < result.distance) {\n"
        "           result = floor;\n"
        "       }\n"
        "    }\n"
        "    return result;\n"
        "}\n";
}

Coder::Coder(Renderer* renderer){
    this->renderer = renderer;
    base =
        "struct Uniforms {\n"
        "    mvp_matrix: mat4x4<f32>,\n"
        "    light_position: vec3<f32>,\n"
        "    aspect_ratio: f32,\n"
        "    size : vec3<f32>,\n"
        "    color: vec3<f32>,\n"
        "    position: vec3<f32>,\n"
        "    mouse_position: vec2<f32>,\n"
        "    id: i32,\n"
        "    floor: i32,\n"
        "};\n"
        "@group(0) @binding(0) var<uniform> uniforms: Uniforms;\n"
        "struct VertexOutput {\n"
        "    @builtin(position) clip_pos: vec4<f32>,\n"
        "    @location(0) uv: vec2<f32>\n"
        "};\n"
        "struct DistanceColor {\n"
        "    distance: f32,\n"
        "    color: vec3<f32>,\n"
        "    id: i32\n"
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
        "const MAX_MARCHING_STEPS: i32 = 1000;\n"
        "const MIN_DISTANCE: f32 = 0.01;\n"
        "const MAX_DISTANCE: f32 = 20.0;\n"
        "@fragment\n"
        "fn fragmentMain(in: VertexOutput) -> @location(0) vec4<f32> {\n"
        "    let cam = transpose(uniforms.mvp_matrix);\n"
        "    let right = cam[0].xyz;\n"
        "    let up = cam[1].xyz;\n"
        "    let forward = cam[2].xyz;\n"
        "    let eye = cam[0].w * right + cam[1].w * up + cam[2].w * forward;\n"
        "    var uv: vec2<f32> = in.uv;\n"
        "    uv = uv * 2.0 - 1.0;\n"
        "    uv.y = -(uv.y * 1.0 / uniforms.aspect_ratio);\n"
        "    let ro = eye;\n"
        "    let rd = normalize(uv.x * right + uv.y * up + FOV * forward);\n"
        "    let dc = ray_march(ro, rd);\n"
        "    return vec4<f32>(dc.color, 1.0);\n"
        "}\n"
        "fn calculate_normal(p: vec3<f32>) -> vec3<f32> {\n"
        "    let eps = 0.0001;\n"
        "    let dx = vec3<f32>(eps, 0.0, 0.0);\n"
        "    let dy = vec3<f32>(0.0, eps, 0.0);\n"
        "    let dz = vec3<f32>(0.0, 0.0, eps);\n"
        "    let nx = sdf(p + dx).distance - sdf(p - dx).distance;\n"
        "    let ny = sdf(p + dy).distance - sdf(p - dy).distance;\n"
        "    let nz = sdf(p + dz).distance - sdf(p - dz).distance;\n"
        "    return normalize(vec3<f32>(nx, ny, nz));\n"
        "}\n"
        "fn calculate_shadow(ro: vec3<f32>, rd: vec3<f32>) -> f32 {\n"
        "   var res: f32 = 1.0;\n"
        "   var t: f32 = 0.05;\n"
        "   let max_t: f32 = 20.0;\n"
        "   let softness: f32 = 0.2;\n"
        "   for (var i: i32 = 0; i < MAX_MARCHING_STEPS && t < max_t; i++) {\n"
        "       let pos = ro + rd * t;\n"
        "       let h = sdf(pos).distance;\n"
        "       res = min(res, h / (softness * t));\n"
        "       t = t + clamp(h, 0.005, 0.5);\n"
        "       if (res < -1.0 || t > max_t) {\n"
        "           break;\n"
        "       }\n"
        "   }\n"
        "   res = max(res, -1.0);\n"
        "   return (0.25 * (1.0 + res) * (1.0 + res) * (2.0 - res));\n"
        "}\n"
        "fn blinn_phong_lighting(pos: vec3<f32>, normal: vec3<f32>, view_dir: vec3<f32>, base_color: vec3<f32>) -> vec3<f32> {\n"
        "    let light_dir = normalize(uniforms.light_position - pos);\n"
        "    let light_color = vec3<f32>(1.0, 1.0, 1.0);\n"
        "    let ambient = 0.2 * light_color;\n"
        "    let diff = max(dot(normal, light_dir), 0.0);\n"
        "    let diffuse = diff * light_color;\n"
        "    let halfway = normalize(light_dir + view_dir);\n"
        "    let spec = pow(max(dot(normal, halfway), 0.0), 32.0);\n"
        "    let specular = spec * light_color;\n"
        "    let shadow = calculate_shadow(pos, light_dir);\n"
        "    return base_color * (ambient + (1.0 + shadow) * (diffuse + specular));\n"
        "}\n"
        "fn ray_march(ro: vec3<f32>, rd: vec3<f32>) -> DistanceColor {\n"
        "   var total_distance: f32 = 0.0;\n"
        "   for (var i: i32 = 0; i < MAX_MARCHING_STEPS && total_distance < MAX_DISTANCE; i++) {\n"
        "       let pos = ro + rd * total_distance;\n"
        "       let dc = sdf(pos);\n"
        "       if (dc.distance < MIN_DISTANCE){\n"
        "           let normal = calculate_normal(pos);\n"
        "           let view_dir = normalize(ro - pos);\n"
        "           let lit_color = blinn_phong_lighting(pos, normal, view_dir, dc.color);\n"
        "           if (dc.id == uniforms.id) {\n"
        "               let color = mix(lit_color, vec3<f32>(0.0, 1.0, 0.0), 0.1);\n"
        "               return DistanceColor(dc.distance, color, dc.id);\n"
        "           }\n"
        "           return DistanceColor(dc.distance, lit_color, dc.id);\n"
        "       }\n"
        "       total_distance += dc.distance;\n"
        "   }\n"
        "   return DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0), -1);\n"
        "}\n"
        "fn opSmoothUnion(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 + 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d2.distance, d1.distance, h) - k * h * (1.0 - h);\n"
        "    let blend = smoothstep(0, 1, h);\n"
        "    let blended_color = mix(d2.color, d1.color, blend); // Blend colors\n"
        "    if(d1.distance < d2.distance) {\n"
        "        return DistanceColor(blended_distance, blended_color, d1.id);\n"
        "    }\n"
        "    return DistanceColor(blended_distance, blended_color, d2.id);\n"
        "}\n"
        "fn opSmoothIntersect(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 - 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d2.distance, d1.distance, h) + k * h * (1.0 - h);\n"
        "    let blended_color = mix(d2.color, d1.color, h); // Blend colors\n"
        "    if(d1.distance < d2.distance) {\n"
        "        return DistanceColor(blended_distance, blended_color, d1.id);\n"
        "    }\n"
        "    return DistanceColor(blended_distance, blended_color, d2.id);\n"
        "}\n"
        "fn opSmoothSubtract(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {\n"
        "    let h = clamp(0.5 - 0.5 * (d2.distance + d1.distance) / k, 0.0, 1.0);\n"
        "    let blended_distance = mix(d1.distance, -d2.distance, h) + k * h * (1.0 - h);\n"
        "    let blended_color = mix(d1.color, d2.color, h); // Blend colors\n"
        "    if(d1.distance < d2.distance) {\n"
        "        return DistanceColor(blended_distance, blended_color, d1.id);\n"
        "    }\n"
        "    return DistanceColor(blended_distance, blended_color, d2.id);\n"
        "}\n"
        "fn opUnion(d1: DistanceColor, d2: DistanceColor) -> DistanceColor {\n"
        "    if (d1.distance < d2.distance) {\n"
        "        return d1;\n"
        "    }\n"
        "    return d2;\n"
        "}\n"
        "fn opSubtract(d1: DistanceColor, d2: DistanceColor) -> DistanceColor {\n"
        "    let h = clamp(0.5 - 0.5 * (d2.distance + d1.distance) / 0.1, 0.0, 1.0);\n"
        "    let d = max(d1.distance, -d2.distance);\n"
        "    let color = mix(d1.color, d2.color, h);\n"
        "    if (d1.distance < d2.distance) {\n"
        "        return DistanceColor(d, color, d1.id);\n"
        "    }\n"
        "    return DistanceColor(d, color, d2.id);\n"
        "}\n"
        "fn sdf_sphere(pos: vec3<f32>, center: vec3<f32>, radius: f32, color: vec3<f32>, id: i32) -> DistanceColor {\n"
        "    var distance = length(pos - center) - radius;\n"
        "    return DistanceColor(distance, color, id);\n"
        "}\n"
        "fn sdf_box(pos: vec3<f32>, center: vec3<f32>, size: vec3<f32>, color: vec3<f32>, id: i32) -> DistanceColor {\n"
        "    let d = abs(pos - center) - size;\n"
        "    let distance = length(max(d, vec3<f32>(0.0))) + min(max(d.x, max(d.y, d.z)), 0.0);\n"
        "    return DistanceColor(distance, color, id);\n"
        "}\n"
        "fn sdf_plane(pos: vec3<f32>) -> DistanceColor {\n"
        "    let distance = abs(pos.y + 1.0);\n"
        "    var color = vec3<f32>(0.1, 0.1, 0.1);\n"
        "    let thickness = 0.01;\n"
        "    if (abs(pos.x - round(pos.x)) < thickness || abs(pos.z - round(pos.z)) < thickness) {\n"
        "        color = vec3<f32>(0.2, 0.2, 0.2);\n"
        "    }\n"
        "    return DistanceColor(distance, color, 0);\n"
        "}\n"
        "@fragment\n"
        "fn fragmentPickingMain(in: VertexOutput) -> @location(0) i32{\n"
        "    let cam = transpose(uniforms.mvp_matrix);\n"
        "    let right = cam[0].xyz;\n"
        "    let up = cam[1].xyz;\n"
        "    let forward = cam[2].xyz;\n"
        "    let eye = cam[0].w * right + cam[1].w * up + cam[2].w * forward;\n"
        "    let m = uniforms.mouse_position * 2.0 - vec2<f32>(1.0, 1.0);\n"
        "    let mouse_uv = vec2<f32>(m.x, -m.y / uniforms.aspect_ratio);\n"
        "    let rd = normalize(mouse_uv.x * right + mouse_uv.y * up + FOV * forward);\n"
        "    let ro = eye;\n"
        "    let dc = ray_march(ro, rd);\n"
        "    return dc.id;\n"
        "}\n"
        ;

}

void Coder::addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation) {
    std::string type = "sphere";
    std::vector<float> vsize ;
    vsize.push_back(size);
    objects.push_back({objectIdCounter, x, y, z, r, g, b, vsize, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation) {
    std::string type = "box";
    objects.push_back({objectIdCounter, x, y, z, r, g, b, size, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::clearObjects() {
    objects.clear();
    renderer->pipelineDirty = true;
}