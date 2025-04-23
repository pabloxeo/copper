struct VertexOutput {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) uv: vec2<f32>
}

struct DistanceColor {
    distance: f32, // Distance from the ray to the surface
    color: vec3<f32> // Color of the object
}

// Constants to replace uniforms
const TIME_FACTOR: f32 = 0.05;       // Used for implicit animation
const LIGHT_POSITION: vec3<f32> = vec3<f32>(5.0, 5.0, 0.0);
const VIEW_POSITION: vec3<f32> = vec3<f32>(0.0, 0.0, -1.0);
const FOV: f32 = 1.0;
const ASPECT_RATIO: f32 = 16.0 / 9.0; // Common 16:9 aspect ratio

@vertex
fn vertexMain(@builtin(vertex_index) i : u32) -> VertexOutput {
    var output: VertexOutput;
    var p = vec2f(0.0, 0.0);

    // Vertex positions and UVs for a quad
    switch (i) {
        case 0u, 4u: {
            p = vec2f(-1.0, -1.0);
            output.uv = vec2f(0.0, 1.0);
        }
        case 1u: {
            p = vec2f(-1.0, 1.0);
            output.uv = vec2f(0.0, 0.0);
        }
        case 2u, 5u: {
            p = vec2f(1.0, 1.0);
            output.uv = vec2f(1.0, 0.0);
        }
        case 3u: {
            p = vec2f(1.0, -1.0);
            output.uv = vec2f(1.0, 1.0);
        }
        default : {
            p = vec2f(0.0, 0.0);
            output.uv = vec2f(0.0, 0.0);
        }
    }

    output.clip_pos = vec4f(p, 0.0, 1.0); // Clip space position
    return output;
}

const MAX_MARCHING_STEPS: i32 = 100; // Maximum number of ray marching steps
const MIN_DIST: f32 = 0.0001;         // Minimum distance to consider
const MAX_DIST: f32 = 1000.0;         // Maximum distance to ray march

@fragment
fn fragmentMain(in: VertexOutput) -> @location(0) vec4f {
    var uv: vec2<f32> = in.uv;
    uv = (uv * 2.0) - 1.0; // Convert UV coordinates to range [-1, 1]
    uv.y = -(uv.y * 1/ASPECT_RATIO); // Adjust for aspect ratio
    
    let ro = VIEW_POSITION; // Fixed camera position
    let rd = normalize(vec3(uv.x, uv.y, FOV)); // Ray direction
    
    return ray_march(ro, rd); // Perform ray marching
}

fn blinn_phong_lighting(normal: vec3<f32>, view_dir: vec3<f32>, light_dir: vec3<f32>, light_col: vec3<f32>, current_pos: vec3<f32>) -> vec3<f32> {
    // Material properties
    let ambient_strength = 0.2; // Ambient light strength
    let diffuse_strength = 0.8; // Diffuse light strength
    let specular_strength = 1.0; // Specular light strength
    let shininess = 32.0; // Shininess factor for highlights

    // Ambient component
    let ambient = ambient_strength * light_col;

    // Diffuse component
    let diff = max(dot(normal, light_dir), 0.0);
    let diffuse = diffuse_strength * diff * light_col;

    // Specular component (Blinn-Phong)
    let halfway_dir = normalize(light_dir + view_dir);
    let spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    let specular = specular_strength * spec * light_col;

    // Shadow calculation
    let shadow_factor = calculate_shadow(current_pos, light_dir, normal); // Check for shadow
    return (ambient + diffuse * (1.0 - shadow_factor) + specular * (1.0 - shadow_factor)); // Combine lighting components
}

fn calculate_shadow(current_pos: vec3<f32>, light_dir: vec3<f32>, normal: vec3<f32>) -> f32 {
    let shadow_ray_origin = current_pos + normal * 0.001; // Offset position to avoid self-shadowing
    let shadow_ray_dir = normalize(light_dir);
    var total_distance: f32 = 0.0;

    // Perform ray marching towards the light
    for (var i = 0; i < MAX_MARCHING_STEPS && total_distance < length(LIGHT_POSITION - current_pos); i++) {
        let shadow_pos = shadow_ray_origin + total_distance * shadow_ray_dir; // Current position in shadow ray
        let dc = sdf(shadow_pos); // Evaluate distance and color

        if (dc.distance < MIN_DIST) { // If close to a surface
            return 1.0; // In shadow
        }

        total_distance += dc.distance; // Increment distance traveled
    }
    return 0.0; // Not in shadow
}

fn opSmoothUnion(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {
    let h = clamp(0.5 + 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);
    let blended_distance = mix(d2.distance, d1.distance, h) - k * h * (1.0 - h);
    let blended_color = mix(d2.color, d1.color, h); // Blend colors
    return DistanceColor(blended_distance, blended_color);
}

fn opSmoothIntersect(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {
    let h = clamp(0.5 - 0.5 * (d2.distance - d1.distance) / k, 0.0, 1.0);
    let blended_distance = mix(d2.distance, d1.distance, h) + k * h * (1.0 - h);
    let blended_color = mix(d2.color, d1.color, h); // Blend colors
    return DistanceColor(blended_distance, blended_color);
}

fn opSmoothSubtraction(d1: DistanceColor, d2: DistanceColor, k: f32) -> DistanceColor {
    let h = clamp(0.5 - 0.5 * (d2.distance + d1.distance) / k, 0.0, 1.0);
    let blended_distance = mix(d2.distance, -d1.distance, h) + k * h * (1.0 - h);
    let blended_color = mix(d2.color, d1.color, h); // Blend colors
    return DistanceColor(blended_distance, blended_color);
}

fn sdf(pos: vec3<f32>) -> DistanceColor {
    let d1 = sdf_sphere(pos); // SDF for sphere
    let d2 = sdf_box(pos);    // SDF for box
    let d3 = sdf_sphere2(pos); // SDF for second sphere

    // Use smooth union for all shapes
    return opSmoothUnion(d3, opSmoothUnion(d1, d2, 1.0), 1.0);
}

fn ray_march(ro: vec3<f32>, rd: vec3<f32>) -> vec4<f32> {
    var total_distance_traveled: f32 = 0.0;
    var surface_color: vec3<f32> = vec3(0.0, 0.0, 0.0); // Default surface color
    var hit_color: vec3<f32> = vec3(0.0, 0.0, 0.0); // Color at hit point
    
    // Ray marching loop
    for (var i = 0; i < MAX_MARCHING_STEPS && total_distance_traveled < MAX_DIST; i++) {
        let current_pos = ro + total_distance_traveled * rd; // Current position in space
        let dc = sdf(current_pos); // Distance and color evaluation
        total_distance_traveled += dc.distance; // Increment distance traveled

        if (dc.distance < MIN_DIST) { // If close to a surface
            // Calculate normal at the current position
            let normal = calculate_normal(current_pos);

            // Calculate lighting based on the light position and view direction
            let light_dir = normalize(LIGHT_POSITION - current_pos);
            let view_dir = normalize(ro - current_pos); // View direction from surface to camera

            // Get the color at the hit point
            hit_color = dc.color;

            // Calculate final color using Blinn-Phong lighting
            let color = blinn_phong_lighting(normal, view_dir, light_dir, vec3<f32>(1.0,1.0,1.0), current_pos); // Calculate lighting
            surface_color = hit_color * color; // Combine surface color with lighting
            break; // Exit the loop upon hitting a surface
        }
    }

    return vec4(surface_color, 1.0); // Return the final color with alpha
}

fn calculate_normal(p: vec3<f32>) -> vec3<f32> {
    let epsilon = 0.00001; // Small offset for normal calculation
    let dx = vec3(epsilon, 0.0, 0.0);
    let dy = vec3(0.0, epsilon, 0.0);
    let dz = vec3(0.0, 0.0, epsilon);

    // Get the distance values for normal calculation
    let d1 = sdf(p + dx).distance; // Normal x-component
    let d2 = sdf(p - dx).distance; // Normal x-component
    let d3 = sdf(p + dy).distance; // Normal y-component
    let d4 = sdf(p - dy).distance; // Normal y-component
    let d5 = sdf(p + dz).distance; // Normal z-component
    let d6 = sdf(p - dz).distance; // Normal z-component

    let normal = vec3(
        d1 - d2, // Normal x-component
        d3 - d4, // Normal y-component
        d5 - d6  // Normal z-component
    );

    return normalize(normal); // Normalize the normal vector
}

// Use builtin time calculation
fn get_time() -> f32 {
    // Since there's no builtin time in WGSL, we use UV coordinates to simulate time
    // This is just a placeholder - in practice you'd want to derive something from
    // the fragment coordinates or pass time via a push constant
    return sin(length(VIEW_POSITION)) * 10.0;
}

// Update the SDF functions to use constant values instead of uniforms
fn sdf_sphere(pos: vec3<f32>) -> DistanceColor {
    let t = get_time() * 0.5; // Simulated time for animation
    let animated_position = vec3(sin(t) * 2.0, sin(t) * 2.0, cos(t) * 2.0 + 4.0);
    let distance = length(pos - animated_position) - 0.5; // Sphere radius of 0.5
    return DistanceColor(distance, vec3(1.0, 0.0, 0.0)); // Red color for the first sphere
}

fn sdf_sphere2(pos: vec3<f32>) -> DistanceColor {
    // Fixed position instead of mouse position
    let fixed_position = vec3(1.5, 1.0, 4.0);
    let distance = length(pos - fixed_position) - 1.0; // Sphere radius of 1.0
    return DistanceColor(distance, vec3(0.0, 0.0, 1.0)); // Blue color for the second sphere
}

fn sdf_box(p: vec3<f32>) -> DistanceColor {
    // Define the half-extents of the box
    let box_size = vec3<f32>(1.0, 1.0, 1.0); // Box dimensions

    // Fixed position for the box
    let translated_p = p - vec3<f32>(0.0, -2.0, 4.0);

    // Calculate the distance for the translated point
    let q = abs(translated_p) - box_size;
    let dist = length(max(q, vec3<f32>(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0);

    // Return the distance with color
    return DistanceColor(dist, vec3<f32>(0.8, 0.8, 0.2)); // Yellow-ish box
}