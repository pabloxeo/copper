struct VertexOutput {
	@builtin(position) clip_pos: vec4<f32>,
	@location(0) uv: vec2<f32>
}

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
	var output: VertexOutput;
	var p = vec2f(0.0, 0.0);
	switch in_vertex_index {
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
    output.clip_pos = vec4f(p, 0.0, 1.0);
	return output;
}

const MAX_MARCHING_STEPS: i32 = 100;
const MIN_DIST: f32 = 0.0001;
const MAX_DIST: f32 = 1000.0;


@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    var uv: vec2<f32> = in.uv;
	uv.y = uv.y * 480.0/640.0;
    let ro = vec3(0.0, 0.0, 0.0);
    let rd = vec3(uv.x, -uv.y, 1.0);

	return ray_march(ro, rd);
}
fn ray_march(ro: vec3<f32>, rd: vec3<f32>) -> vec4f {

    var total_distance_traveled: f32 = 0.0;

    for(var i = 0; i < MAX_MARCHING_STEPS; i++) {
        let current_pos = ro + total_distance_traveled * rd;
        let distance_to_closest = sdf(current_pos);

        total_distance_traveled += distance_to_closest;

        if (total_distance_traveled > MAX_DIST || distance_to_closest < MIN_DIST) {
            break;
        }
    }

    let out = vec3(clamp(log(abs(total_distance_traveled)) * 0.2, 0.0, 1.0));



    return vec4(1.0 - out, 1.0);
}

fn sdf(pos: vec3<f32>) -> f32 {
    return min(sdf_plane(pos), sdf_plane(pos));
}
fn sdf_sphere(pos: vec3<f32>) -> f32 {
    return length(pos - vec3(3.0, -3.0, 10.0)) - 2.0;
}
fn sdf_plane(pos: vec3<f32>) -> f32 {
    return dot(pos, vec3(0.0, 1.0, 0.0)) + -2.0;
}