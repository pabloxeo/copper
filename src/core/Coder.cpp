#include "./Coder.h"
#include "./Renderer.h"
#include <fstream>
#include <sstream>

void Coder::generateShaderCode() {
    shaderCode =
        "fn sdf(pos: vec3<f32>) -> DistanceColor {\n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0), -1);\n";

    std::string sdfPickingCode =
        "fn sdf_picking(pos: vec3<f32>) -> DistanceColor {\n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0), -1);\n";

    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& object = objects[i];
        std::string varName = object.type + std::to_string(i);
        std::string objCode;

        if (object.id == selectedObjectId) {
            if (object.type == "sphere") {
                objCode = "    var " + varName + ": DistanceColor = sdf_sphere(pos, uniforms.position, uniforms.size[0], uniforms.color, " + std::to_string(object.id) + ");\n";
            } else if (object.type == "box") {
                objCode = "    var " + varName + ": DistanceColor = sdf_box(pos, uniforms.position, uniforms.size, uniforms.color, " + std::to_string(object.id) + ");\n";
            }else if(object.type == "cone") {
                objCode = "    var " + varName + ": DistanceColor = sdf_cone(pos, uniforms.position, vec2<f32>(uniforms.size[1], uniforms.size[2]), uniforms.size[0], uniforms.color, " + std::to_string(object.id) + ");\n";
            } else if(object.type == "cylinder") {
                objCode = "    var " + varName + ": DistanceColor = sdf_cylinder(pos, uniforms.position, uniforms.size[0], uniforms.size[1], uniforms.color, " + std::to_string(object.id) + ");\n";
            }
        } else {
            if (object.type == "sphere") {
                objCode = "    var " + varName + ": DistanceColor = sdf_sphere(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), " +
                    std::to_string(object.size[0]) + ", vec3<f32>(" + std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            } else if (object.type == "box") {
                objCode = "    var " + varName + ": DistanceColor = sdf_box(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), vec3<f32>(" +
                    std::to_string(object.size[0]) + ", " + std::to_string(object.size[1]) + ", " + std::to_string(object.size[2]) + "), vec3<f32>(" +
                    std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            }else if(object.type == "cone") {
                objCode = "    var " + varName + ": DistanceColor = sdf_cone(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), vec2<f32>(" +
                    std::to_string(object.size[1]) + ", " + std::to_string(object.size[2]) + "), " + std::to_string(object.size[0]) + ", vec3<f32>(" +
                    std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            } else if(object.type == "cylinder") {
                objCode = "    var " + varName + ": DistanceColor = sdf_cylinder(pos, vec3<f32>(" + std::to_string(object.x) + ", " + std::to_string(object.y) + ", " + std::to_string(object.z) + "), " +
                    std::to_string(object.size[0]) + ", " + std::to_string(object.size[1]) + ", vec3<f32>(" +
                    std::to_string(object.r) + ", " + std::to_string(object.g) + ", " + std::to_string(object.b) + "), " + std::to_string(object.id) + ");\n";
            }
        }


        shaderCode += objCode;
        sdfPickingCode += objCode;

        
        if (object.operation == "smoothunion") {
            shaderCode += "    result = opSmoothUnion(result, " + varName + ", 0.6);\n";
        } else if (object.operation == "intersection") {
            shaderCode += "    result = opSmoothIntersect(result, " + varName + ", 0.6);\n";
        } else if (object.operation == "smoothsubtract" ) {
            shaderCode += "    result = opSmoothSubtract(result, " + varName + ", 0.6);\n";
        } else if (object.operation == "union") {
            shaderCode += "    result = opUnion(result, " + varName + ");\n";
        } else if (object.operation == "subtract") {
            shaderCode += "    result = opSubtract(result, " + varName + ");\n";
        }
        sdfPickingCode +=
            "    if (" + varName + ".distance < result.distance) {\n"
            "        result = " + varName + ";\n"
            "    }\n";
    }
    if (renderer->floor) {
        shaderCode +=
            "    let floor = sdf_plane(pos);\n"
            "    if (floor.distance < result.distance) {\n"
            "        result = floor;\n"
            "    }\n";
    }

    shaderCode +=
        "    return result;\n"
        "}\n";

    sdfPickingCode +=
        "    return result;\n"
        "}\n";

    shaderCode += sdfPickingCode;

    if (selectedObjectId != -1) {
        
        shaderCode +=
            "fn sdf_combined(pos: vec3<f32>) -> DistanceColor {\n"
            "    let object_result = sdf(pos);\n"
            "    let gizmo_result = sdf_gizmos(pos);\n"
            "    \n"
            "    // Always show gizmos when they're close enough\n"
            "    if (gizmo_result.distance < 0.01) {\n"  // Use a small threshold
            "        return gizmo_result;\n"
            "    }\n"
            "    \n"
            "    // Use a small offset to ensure gizmos appear in front\n"
            "    if (gizmo_result.distance < object_result.distance + 0.02) {\n"
            "        return gizmo_result;\n"
            "    }\n"
            "    \n"
            "    return object_result;\n"
            "}\n";
    } else {
        shaderCode +=
            "fn sdf_combined(pos: vec3<f32>) -> DistanceColor {\n"
            "    return sdf(pos);\n"
            "}\n";
    }
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
        "const MIN_DISTANCE: f32 = 0.001;\n"
        "const MAX_DISTANCE: f32 = 100.0;\n"
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
        "    let nx = sdf_combined(p + dx).distance - sdf_combined(p - dx).distance;\n"
        "    let ny = sdf_combined(p + dy).distance - sdf_combined(p - dy).distance;\n"
        "    let nz = sdf_combined(p + dz).distance - sdf_combined(p - dz).distance;\n"
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
        "   // First pass: Render objects normally\n"
        "   var total_distance: f32 = 0.0;\n"
        "   var object_result: DistanceColor;\n"
        "   var object_hit: bool = false;\n"
        "   \n"
        "   for (var i: i32 = 0; i < MAX_MARCHING_STEPS && total_distance < MAX_DISTANCE; i++) {\n"
        "       let pos = ro + rd * total_distance;\n"
        "       let dc = sdf(pos);\n"  // Only objects, no gizmos
        "       if (dc.distance < MIN_DISTANCE){\n"
        "           let normal = calculate_normal(pos);\n"
        "           let view_dir = normalize(ro - pos);\n"
        "           var lit_color = blinn_phong_lighting(pos, normal, view_dir, dc.color);\n"
        "           object_result = DistanceColor(total_distance, lit_color, dc.id);\n"
        "           object_hit = true;\n"
        "           break;\n"
        "       }\n"
        "       total_distance += dc.distance;\n"
        "   }\n"
        "   // Second pass: Check for gizmos along the entire ray\n"
        "   if (uniforms.id != -1) {\n"
        "       let gizmo_result = ray_march_gizmos_only(ro, rd);\n"
        "       if (gizmo_result.id != -1) {\n"
        "           return gizmo_result;  // Gizmos always win\n"
        "       }\n"
        "   }\n"
        "   // Return object result or background\n"
        "   if (object_hit) {\n"
        "       return object_result;\n"
        "   }\n"
        "   return DistanceColor(1e6, vec3<f32>(0.0, 0.0, 0.0), -1);\n"
        "}\n"
        "fn ray_march_gizmos_only(ro: vec3<f32>, rd: vec3<f32>) -> DistanceColor {\n"
        "   var total_distance: f32 = 0.0;\n"
        "   \n"
        "   for (var i: i32 = 0; i < MAX_MARCHING_STEPS && total_distance < MAX_DISTANCE; i++) {\n"
        "       let pos = ro + rd * total_distance;\n"
        "       let gizmo_result = sdf_gizmos(pos);\n"
        "       \n"
        "       if (gizmo_result.id != -1 && gizmo_result.distance < MIN_DISTANCE) {\n"
        "           let view_dir = normalize(ro - pos);\n"
        "           return DistanceColor(gizmo_result.distance, gizmo_result.color, gizmo_result.id);\n"
        "       }\n"
        "       total_distance += gizmo_result.distance;\n"
        "   }\n"
        "   \n"
        "   return DistanceColor(1e6, vec3<f32>(0.0), -1);\n"
        "}\n"
        "fn rotate_to_axis(pos: vec3<f32>, axis: vec3<f32>) -> vec3<f32> {\n"
        "    let up = vec3<f32>(0.0, 1.0, 0.0);\n"
        "    let v = normalize(axis);\n"
        "    let c = dot(up, v);\n"
        "    if (c > 0.999) { return pos; }\n"
        "    if (c < -0.999) { return vec3<f32>(pos.x, -pos.y, -pos.z); }\n"
        "    let rot_axis = normalize(cross(up, v));\n"
        "    let angle = acos(c);\n"
        "    let s = sin(angle);\n"
        "    let t = 1.0 - cos(angle);\n"
        "    let x = rot_axis.x;\n"
        "    let y = rot_axis.y;\n"
        "    let z = rot_axis.z;\n"
        "    let m = mat3x3<f32>(\n"
        "        vec3<f32>(t*x*x + cos(angle), t*x*y - s*z, t*x*z + s*y),\n"
        "        vec3<f32>(t*x*y + s*z, t*y*y + cos(angle), t*y*z - s*x),\n"
        "        vec3<f32>(t*x*z - s*y, t*y*z + s*x, t*z*z + cos(angle))\n"
        "    );\n"
        "    return m * pos;\n"
        "}\n"
        "\n"
        "fn sdf_gizmo_arrow(pos: vec3<f32>, center: vec3<f32>, direction: vec3<f32>, color: vec3<f32>) -> DistanceColor {\n"
        "    let shaft_length = 0.6 * uniforms.size.x;\n"
        "    let shaft_radius = 0.02 * uniforms.size.x;\n"
        "    let head_length = 0.4 * uniforms.size.x;\n"
        "    let head_radius = 0.15 * uniforms.size.x;\n"
        "    let local_pos = pos - center;\n"
        "    let axis = normalize(direction);\n"
        "    let rotated_pos = rotate_to_axis(local_pos, axis);\n"
        "    // Cylinder for shaft (Y axis)\n"
        "    let shaft = sdf_cylinder(rotated_pos - vec3<f32>(0.0, shaft_length / 2.0, 0.0), vec3<f32>(0.0), shaft_radius, shaft_length / 2.0, color, -2);\n"
        "    // Cone for head (Y axis)\n"
        "    let cone_angle = vec2<f32>(head_radius, head_length);\n"
        "    let head = sdf_cone(rotated_pos - vec3<f32>(0.0, shaft_length + head_length / 2.0, 0.0), vec3<f32>(0.0), cone_angle, head_length / 2.0, color, -2);\n"
        "    // Combine with union\n"
        "    let arrow = opUnion(shaft, head);\n"
        "    return arrow;\n"
        "}\n"
        "\n"
        "fn sdf_gizmo_plane(pos: vec3<f32>, center: vec3<f32>, normal1: vec3<f32>, normal2: vec3<f32>, color: vec3<f32>) -> DistanceColor {\n"
        "    let size = 0.2 * uniforms.size.x;\n"
        "    let thickness = 0.01 * uniforms.size.x;\n"
        "    let local_pos = pos - center;\n"
        "    // Build a rotation matrix from normal1 and normal2\n"
        "    let n1 = normalize(normal1);\n"
        "    let n2 = normalize(normal2);\n"
        "    let n3 = normalize(cross(n1, n2));\n"
        "    let rot = mat3x3<f32>(n1, n2, n3);\n"
        "    // Transform local_pos into box space\n"
        "    let box_pos = transpose(rot) * local_pos;\n"
        "    let box_size = vec3<f32>(size, size, thickness);\n"
        "    return sdf_box(box_pos, vec3<f32>(0.0), box_size, color, -2);\n"
        "}\n"
        "\n"
        "fn sdf_gizmos(pos: vec3<f32>) -> DistanceColor {\n"
        "    let gizmo_center = vec3<f32>(uniforms.position.x, uniforms.position.y, uniforms.position.z);\n"
        "    let offset = -0.50 * uniforms.size.x;\n"
        "    \n"
        "    var result = DistanceColor(1e6, vec3<f32>(0.0), -1);\n"
        "    \n"
        "    // X Arrow (Red)\n"
        "    let x_arrow = sdf_gizmo_arrow(pos, gizmo_center, vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(1.0, 0.0, 0.0));\n"
        "    if (x_arrow.distance < result.distance) { result = x_arrow; }\n"
        "    \n"
        "    // Y Arrow (Green)\n"
        "    let y_arrow = sdf_gizmo_arrow(pos, gizmo_center, vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 1.0, 0.0));\n"
        "    if (y_arrow.distance < result.distance) { result = y_arrow; }\n"
        "    \n"
        "    // Z Arrow (Blue)\n"
        "    let z_arrow = sdf_gizmo_arrow(pos, gizmo_center, vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(0.0, 0.0, 1.0));\n"
        "    if (z_arrow.distance < result.distance) { result = z_arrow; }\n"
        "    \n"
        "    // XY Plane (Yellow) - offset along Z\n"
        "    let xy_plane = sdf_gizmo_plane(pos, gizmo_center + vec3<f32>(0.0, 0.0, offset), vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 1.0, 0.0));\n"
        "    if (xy_plane.distance < result.distance) { result = xy_plane; }\n"
        "    \n"
        "    // XZ Plane (Magenta) - offset along Y\n"
        "    let xz_plane = sdf_gizmo_plane(pos, gizmo_center + vec3<f32>(0.0, offset, 0.0), vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(1.0, 0.0, 0.0));\n"
        "    if (xz_plane.distance < result.distance) { result = xz_plane; }\n"
        "    \n"
        "    // YZ Plane (Cyan) - offset along X\n"
        "    let yz_plane = sdf_gizmo_plane(pos, gizmo_center + vec3<f32>(offset, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0), vec3<f32>(0.0, 0.0, 1.0));\n"
        "    if (yz_plane.distance < result.distance) { result = yz_plane; }\n"
        "    \n"
        "    return result;\n"
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
        "fn sdf_cone(pos: vec3<f32>, center: vec3<f32>, angle: vec2<f32>, height: f32, color: vec3<f32>, id: i32) -> DistanceColor {\n"
        "    let local_pos = pos - center;\n"
        "    let q = height * vec2(angle.x / angle.y, -1.0);\n"
        "    let w = vec2(length(local_pos.xz), local_pos.y);\n"
        "    let a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );\n"
        "    let b = w - q*vec2(clamp(w.x/q.x, 0.0, 1.0), 1.0);\n"
        "    let k = sign(q.y);\n"
        "    let d = min(dot(a, a), dot(b, b));\n"
        "    let s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );\n"
        "    return DistanceColor(sqrt(d)*sign(s), color, id);\n"
        "}\n"
        "fn sdf_cylinder(pos: vec3<f32>, center: vec3<f32>, radius: f32, height: f32, color: vec3<f32>, id: i32) -> DistanceColor {\n"
        "    let local_pos = pos - center;\n"
        "    let d = abs(vec2(length(local_pos.xz), local_pos.y)) - vec2(radius, height);\n"
        "    let dist = min(max(d.x, d.y), 0.0) + length(max(d, vec2<f32>(0.0)));\n"
        "    return DistanceColor(dist, color, id);\n"
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
        "    let dc = ray_march_picking(ro, rd);\n"
        "    return dc.id;\n"
        "}\n"
        "fn ray_march_picking(ro: vec3<f32>, rd: vec3<f32>) -> DistanceColor {\n"
        "    var total_distance: f32 = 0.0;\n"
        "    for (var i: i32 = 0; i < MAX_MARCHING_STEPS && total_distance < MAX_DISTANCE; i++) {\n"
        "        let pos = ro + rd * total_distance;\n"
        "        let dc = sdf_picking(pos);\n"
        "        if (dc.distance < MIN_DISTANCE) {\n"
        "            return dc;\n"
        "        }\n"
        "        total_distance += dc.distance;\n"
        "    }\n"
        "    return DistanceColor(1e6, vec3<f32>(0.0), -1);\n"
        "}\n"
        ;
}

void Coder::addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation) {
    std::string type = "sphere";
    objects.push_back({objectIdCounter, x, y, z, r, g, b, std::vector<float>{size}, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation) {
    std::string type = "box";
    objects.push_back({objectIdCounter, x, y, z, r, g, b, size, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::addCone(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation) {
    std::string type = "cone";
    objects.push_back({objectIdCounter, x, y, z, r, g, b, size, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::addCylinder(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation) {
    std::string type = "cylinder";
    objects.push_back({objectIdCounter, x, y, z, r, g, b, size, type, operation});
    objectIdCounter++;
    renderer->pipelineDirty = true;
}

void Coder::clearObjects() {
    objects.clear();
    renderer->pipelineDirty = true;
}


bool Coder::saveScene(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        printf("Failed to open file for writing: %s\n", filename.c_str());
        return false;
    }

    file << "# Copper Scene File v1.0\n";
    file << "objects " << objects.size() << "\n";

    for (const auto& obj : objects) {
        file << obj.type << " ";
        file << obj.x << " " << obj.y << " " << obj.z << " ";
        
        if (obj.type == "sphere") {
            file << obj.size[0] << " ";
        } else if (obj.type == "box") {
            file << obj.size[0] << " " << obj.size[1] << " " << obj.size[2] << " ";
        }
        
        file << obj.r << " " << obj.g << " " << obj.b << " ";
        file << obj.operation << " " << obj.id << "\n";
    }

    file.close();
    printf("Scene saved to: %s\n", filename.c_str());
    return true;
}

bool Coder::loadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        printf("Failed to open file for reading: %s\n", filename.c_str());
        return false;
    }

    clearObjects();

    std::string line;
    while (std::getline(file, line)) {

        std::stringstream ss(line);
        std::string first_word;
        ss >> first_word;

        if (first_word == "objects") {
            int count;
            ss >> count;
            continue;
        }

        std::string type = first_word;
        float x, y, z, r, g, b;
        std::string operation;
        int id;

        if (type == "sphere") {
            float radius;
            if (ss >> x >> y >> z >> radius >> r >> g >> b >> operation >> id) {
                addSphere(x, y, z, radius, r, g, b, operation);
                if (!objects.empty()) {
                    objects.back().id = id;
                    objectIdCounter = std::max(objectIdCounter, id + 1);
                }
            }
        } else if (type == "box") {
            float size_x, size_y, size_z;
            if (ss >> x >> y >> z >> size_x >> size_y >> size_z >> r >> g >> b >> operation >> id) {
                addBox(x, y, z, {size_x, size_y, size_z}, r, g, b, operation);
                if (!objects.empty()) {
                    objects.back().id = id;
                    objectIdCounter = std::max(objectIdCounter, id + 1);
                }
            }
        }
    }

    file.close();
    printf("Scene loaded from: %s\n", filename.c_str());
    return true;
}


// "fn sdf_gizmo_plane(pos: vec3<f32>, center: vec3<f32>, normal1: vec3<f32>, normal2: vec3<f32>, color: vec3<f32>) -> DistanceColor {\n"
// "    let size = 0.2;\n"
// "    let thickness = 0.01;\n"
// "    \n"
// "    let local_pos = pos - center;\n"
// "    let u = dot(local_pos, normal1);\n"
// "    let v = dot(local_pos, normal2);\n"
// "    let w = dot(local_pos, cross(normal1, normal2));\n"
// "    \n"
// "    let plane_dist = abs(w) - thickness;\n"
// "    let bound_dist = max(abs(u) - size, abs(v) - size);\n"
// "    \n"
// "    var dist: f32;\n"
// "    if (u > 0.0 && v > 0.0 && u < size && v < size) {\n"
// "        dist = plane_dist;\n"
// "    } else {\n"
// "        dist = max(plane_dist, bound_dist);\n"
// "    }\n"
// "    \n"
// "    return DistanceColor(dist, color, -2);\n"
// "}\n"