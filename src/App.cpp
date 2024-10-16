//
// Created by pablo on 20/07/2024.
//

#include "App.h"

using namespace wgpu;

const char* shaderSource = R"(

struct VertexOutput {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) uv: vec2<f32>
}
struct DistanceColor {
    distance: f32, // Distance from the ray to the surface
    color: vec3<f32> // Color of the object
}

@group(0) @binding(0) var<uniform> uTime: f32;                  // Time for animations
@group(0) @binding(1) var<uniform> uMousePosition: vec2<f32>;  // Mouse position
@group(0) @binding(2) var<uniform> uLightPosition: vec3<f32>;  // Light source position
@group(0) @binding(3) var<uniform> uViewPosition: vec3<f32>;   // Camera position

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
    var output: VertexOutput;
    var p = vec2f(0.0, 0.0);

    // Vertex positions and UVs for a quad
    switch (in_vertex_index) {
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
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    var uv: vec2<f32> = in.uv;
    uv = (uv * 2.0) - 1.0; // Convert UV coordinates to range [-1, 1]
    uv.y = -(uv.y * 1080.0 / 720.0); // Adjust for aspect ratio

    let aspect_ratio = 1080.0 / 720.0;
    let ro = uViewPosition; // Camera position
    let rd = normalize(vec3(uv.x * aspect_ratio, uv.y, 1.0)); // Ray direction

    return ray_march(ro, rd); // Perform ray marching
}

fn blinn_phong_lighting(normal: vec3<f32>, view_dir: vec3<f32>, light_dir: vec3<f32>, current_pos: vec3<f32>) -> vec3<f32> {
    // Material properties
    let ambient_strength = 0.2; // Ambient light strength
    let diffuse_strength = 0.8; // Diffuse light strength
    let specular_strength = 1.4; // Specular light strength
    let shininess = 32.0; // Shininess factor for highlights

    // Ambient component
    let ambient = ambient_strength * vec3(1.0, 1.0, 1.0);

    // Diffuse component
    let diff = max(dot(normal, light_dir), 0.0);
    let diffuse = diffuse_strength * diff * vec3(1.0, 1.0, 1.0);

    // Specular component (Blinn-Phong)
    let halfway_dir = normalize(light_dir + view_dir);
    let spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    let specular = specular_strength * spec * vec3(1.0, 1.0, 1.0);

    // Shadow calculation
    let shadow_factor = calculate_shadow(current_pos, light_dir, normal); // Check for shadow
    return (ambient + diffuse * (1.0 - shadow_factor) + specular * (1.0 - shadow_factor)); // Combine lighting components
}



fn calculate_shadow(current_pos: vec3<f32>, light_dir: vec3<f32>, normal: vec3<f32>) -> f32 {
    let shadow_ray_origin = current_pos + normal * 0.001; // Offset position to avoid self-shadowing
    let shadow_ray_dir = normalize(light_dir);
    var total_distance: f32 = 0.0;

    // Perform ray marching towards the light
    for (var i = 0; i < MAX_MARCHING_STEPS; i++) {
        let shadow_pos = shadow_ray_origin + total_distance * shadow_ray_dir; // Current position in shadow ray
        let dc = sdf(shadow_pos); // Evaluate distance and color

        if (dc.distance < MIN_DIST) { // If close to a surface
            return 1.0; // In shadow
        }

        total_distance += dc.distance; // Increment distance traveled

        if (total_distance > length(uLightPosition - current_pos)) { // If light is reached
            break; // Exit loop
        }
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

fn sdf(pos: vec3<f32>) -> DistanceColor {
    let d1 = sdf_sphere(pos); // SDF for sphere
    let d2 = sdf_plane(pos);   // SDF for plane
    let d3 = sdf_sphere2(pos);

    // Use smooth union or intersection with colors
    return opSmoothUnion(d3, opSmoothUnion(d1, d2, 0.3), 0.3); // Use smooth union
    // return opSmoothIntersect(d1, d2, 0.1); // Uncomment this for smooth intersection
}

fn ray_march(ro: vec3<f32>, rd: vec3<f32>) -> vec4<f32> {
    var total_distance_traveled: f32 = 0.0;
    var surface_color: vec3<f32> = vec3(0.0, 0.0, 0.0); // Default surface color
    var hit_color: vec3<f32> = vec3(0.0, 0.0, 0.0); // Color at hit point

    // Ray marching loop
    for (var i = 0; i < MAX_MARCHING_STEPS; i++) {
        let current_pos = ro + total_distance_traveled * rd; // Current position in space
        let dc = sdf(current_pos); // Distance and color evaluation
        total_distance_traveled += dc.distance; // Increment distance traveled

        if (dc.distance < MIN_DIST) { // If close to a surface
            // Calculate normal at the current position
            let normal = calculate_normal(current_pos);

            // Calculate lighting based on the light position and view direction
            let light_dir = normalize(uLightPosition - current_pos);
            let view_dir = normalize(ro - current_pos); // View direction from surface to camera

            // Get the color at the hit point
            hit_color = dc.color;

            // Calculate final color using Blinn-Phong lighting
            let color = blinn_phong_lighting(normal, view_dir, light_dir, current_pos); // Calculate lighting
            surface_color = hit_color * color; // Combine surface color with lighting
            break; // Exit the loop upon hitting a surface
        }

        if (total_distance_traveled > MAX_DIST) { // If maximum distance exceeded
            break; // Exit the loop
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


// Update the SDF functions to return DistanceColor
fn sdf_sphere(pos: vec3<f32>) -> DistanceColor {
    let animated_position = vec3(sin(uTime) * 1.0, 0.0, cos(uTime) * 1.0 + 4.0);
    let distance = length(pos - animated_position) - 1.0; // Sphere radius of 1.0
    return DistanceColor(distance, vec3(1.0, 0.0, 0.0)); // Red color for the first sphere
}

fn sdf_sphere2(pos: vec3<f32>) -> DistanceColor {
    let animated_position = vec3(uMousePosition.x, uMousePosition.y, 4.0);
    let distance = length(pos - animated_position) - 1.0; // Sphere radius of 1.0
    return DistanceColor(distance, vec3(0.0, 0.0, 1.0)); // Green color for the second sphere
}

fn sdf_plane(pos: vec3<f32>) -> DistanceColor {
    return DistanceColor(pos.y + 1.0, vec3(0.1, 0.1, 0.1)); // Blue color for the plane
}


)";


bool App::Initialize() {
	// Open window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1080, 720, "Copper", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this); // Set the user pointer to access App instance
    glfwSetKeyCallback(window, KeyCallback);

    Instance instance = wgpuCreateInstance(nullptr);

	surface = glfwGetWGPUSurface(instance, window);

	std::cout << "Requesting adapter..." << std::endl;
	surface = glfwGetWGPUSurface(instance, window);
	RequestAdapterOptions adapterOpts = {};
	adapterOpts.compatibleSurface = surface;
	Adapter adapter = instance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	instance.release();

	std::cout << "Requesting device..." << std::endl;
	DeviceDescriptor deviceDesc = {};
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	device = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << device << std::endl;

	uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	});

	queue = device.getQueue();

	// Configure the surface
	SurfaceConfiguration config = {};

	// Configuration of the textures created for the underlying swap chain
	config.width = 1080;
	config.height = 720;
	config.usage = TextureUsage::RenderAttachment;
	surfaceFormat = surface.getPreferredFormat(adapter);
	config.format = surfaceFormat;

	// And we do not need any particular view format:
	config.viewFormatCount = 0;
	config.viewFormats = nullptr;
	config.device = device;
	config.presentMode = PresentMode::Fifo;
	config.alphaMode = CompositeAlphaMode::Auto;

	surface.configure(config);

	// Release the adapter only after it has been fully utilized
	adapter.release();
    // Define the Bind Group Layout for uTime uniform
    // Define uTime layout entry
    BindGroupLayoutEntry uTimeLayoutEntry = {};
    uTimeLayoutEntry.binding = 0;
    uTimeLayoutEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    uTimeLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uTimeLayoutEntry.buffer.minBindingSize = sizeof(float);

// Define uMousePosition layout entry
    BindGroupLayoutEntry uMousePositionLayoutEntry = {};
    uMousePositionLayoutEntry.binding = 1;
    uMousePositionLayoutEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    uMousePositionLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uMousePositionLayoutEntry.buffer.minBindingSize = 2 * sizeof(float);


    // Define uLightPosition layout entry
    BindGroupLayoutEntry uLightPositionLayoutEntry = {};
    uLightPositionLayoutEntry.binding = 2;
    uLightPositionLayoutEntry.visibility = ShaderStage::Fragment;
    uLightPositionLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uLightPositionLayoutEntry.buffer.minBindingSize = 3 * sizeof(float);

    BindGroupLayoutEntry uViewPositionLayoutEntry = {};
    uViewPositionLayoutEntry.binding = 3;
    uViewPositionLayoutEntry.visibility = ShaderStage::Fragment;
    uViewPositionLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uViewPositionLayoutEntry.buffer.minBindingSize = 3 * sizeof(float);

// Set up layout entries array and descriptor
    BindGroupLayoutEntry layoutEntries[] = { uTimeLayoutEntry, uMousePositionLayoutEntry, uLightPositionLayoutEntry, uViewPositionLayoutEntry };
    BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
    bindGroupLayoutDescriptor.entryCount = 4;
    bindGroupLayoutDescriptor.entries = layoutEntries;

// Create bind group layout
    bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDescriptor);

// Buffer for uTime
    BufferDescriptor uTimeBufferDesc = {};
    uTimeBufferDesc.size = sizeof(float);
    uTimeBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uTimeBuffer = device.createBuffer(uTimeBufferDesc);

// Buffer for uMousePosition
    BufferDescriptor uMousePositionBufferDesc = {};
    uMousePositionBufferDesc.size = 2 * sizeof(float);
    uMousePositionBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uMouseBuffer = device.createBuffer(uMousePositionBufferDesc);

    // Buffer for uLightPosition
    BufferDescriptor uLightPositionBufferDesc = {};
    uLightPositionBufferDesc.size = 3 * sizeof(float);
    uLightPositionBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uLightPositionBuffer = device.createBuffer(uLightPositionBufferDesc);

// Buffer for uViewPosition
    BufferDescriptor uViewPositionBufferDesc = {};
    uViewPositionBufferDesc.size = 3 * sizeof(float);
    uViewPositionBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uViewPositionBuffer = device.createBuffer(uViewPositionBufferDesc);

// Bind group entries
    BindGroupEntry timeBufferEntry = {};
    timeBufferEntry.binding = 0;
    timeBufferEntry.buffer = uTimeBuffer;
    timeBufferEntry.offset = 0;
    timeBufferEntry.size = sizeof(float);

    BindGroupEntry mouseBufferEntry = {};
    mouseBufferEntry.binding = 1;
    mouseBufferEntry.buffer = uMouseBuffer;
    mouseBufferEntry.offset = 0;
    mouseBufferEntry.size = 2 * sizeof(float);

    BindGroupEntry lightBufferEntry = {};
    lightBufferEntry.binding = 2;
    lightBufferEntry.buffer = uLightPositionBuffer;
    lightBufferEntry.offset = 0;
    lightBufferEntry.size = 3 * sizeof(float);

    BindGroupEntry viewBufferEntry = {};
    viewBufferEntry.binding = 3;
    viewBufferEntry.buffer = uViewPositionBuffer;
    viewBufferEntry.offset = 0;
    viewBufferEntry.size = 3 * sizeof(float);

    BindGroupEntry bindGroupEntries[] = { timeBufferEntry, mouseBufferEntry, lightBufferEntry, viewBufferEntry };
    BindGroupDescriptor bindGroupDesc = {};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 4;
    bindGroupDesc.entries = bindGroupEntries;

// Create the bind group
    bindGroup = device.createBindGroup(bindGroupDesc);

    InitializePipeline();

    return true;
}

void App::Terminate() {
	pipeline.release();
	surface.unconfigure();
	queue.release();
	surface.release();
	device.release();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void App::MainLoop() {
	glfwPollEvents();

	// Get the next target texture view
	TextureView targetView = GetNextSurfaceTextureView();
	if (!targetView) return;

	// Create a command encoder for the draw call
	CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = "My command encoder";
	CommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

	// Create the render pass that clears the screen with our color
	RenderPassDescriptor renderPassDesc = {};

	// The attachment part of the render pass descriptor describes the target texture of the pass
	RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = LoadOp::Clear;
	renderPassColorAttachment.storeOp = StoreOp::Store;
	renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWrites = nullptr;

	RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

	// Select which render pipeline to use
	renderPass.setPipeline(pipeline);
    // Bind the group before the draw call
    renderPass.setBindGroup(0, bindGroup, 0, nullptr);
    // Draw 1 instance of a 3-vertices shape
	renderPass.draw(6, 1, 0, 0);

	renderPass.end();
	renderPass.release();

	// Finally encode and submit the render pass
	CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.label = "Command buffer";
	CommandBuffer command = encoder.finish(cmdBufferDescriptor);
	encoder.release();

	std::cout << "Submitting command..." << std::endl;
	queue.submit(1, &command);
	command.release();
	std::cout << "Command submitted." << std::endl;

	// At the enc of the frame
	targetView.release();
#ifndef __EMSCRIPTEN__
	surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
	device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
	device.poll(false);
#endif
    // Example of updating uTime each frame
    float currentTime = static_cast<float>(glfwGetTime());
    queue.writeBuffer(uTimeBuffer, 0, &currentTime, sizeof(float));

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

// Convert to normalized coordinates (-1 to 1)
    float mousePos[2] = {
            static_cast<float>((xpos / 1080.0) * 2.0 - 1.0),
            static_cast<float>((1.0 - (ypos / 720.0)) * 2.0 - 1.0)
    };

    queue.writeBuffer(uLightPositionBuffer, 0, lightPos, sizeof(lightPos));

    float viewPos[3] = {0.0f, 0.0f, 0.0f};  // Sample view position, update as needed
    queue.writeBuffer(uViewPositionBuffer, 0, &viewPos, sizeof(viewPos));


// Update buffer
    queue.writeBuffer(uMouseBuffer, 0, &mousePos, sizeof(mousePos));

}

bool App::IsRunning() {
	return !glfwWindowShouldClose(window);
}

TextureView App::GetNextSurfaceTextureView() {
	// Get the surface texture
	SurfaceTexture surfaceTexture;
	surface.getCurrentTexture(&surfaceTexture);
	if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
		return nullptr;
	}
	Texture texture = surfaceTexture.texture;

	// Create a view for this surface texture
	TextureViewDescriptor viewDescriptor;
	viewDescriptor.label = "Surface texture view";
	viewDescriptor.format = texture.getFormat();
	viewDescriptor.dimension = TextureViewDimension::_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = TextureAspect::All;
	TextureView targetView = texture.createView(viewDescriptor);

	return targetView;
}

void App::InitializePipeline() {
    // Load the shader module
    ShaderModuleDescriptor shaderDesc;
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif

    ShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderCodeDesc.code = shaderSource;
    ShaderModule shaderModule = device.createShaderModule(shaderDesc);

    RenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.bufferCount = 0; // No vertex buffers needed
    pipelineDesc.vertex.buffers = nullptr;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0; // No vertex constants needed
    pipelineDesc.vertex.constants = nullptr;
    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = FrontFace::CCW;
    pipelineDesc.primitive.cullMode = CullMode::None;

    FragmentState fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0; // No fragment constants needed

    // Configure blending state
    BlendState blendState;
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;
    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;

    ColorTargetState colorTarget;
    colorTarget.format = surfaceFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All;

    // Set up color targets
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.depthStencil = nullptr;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    // Create the pipeline layout descriptor
    PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1; // Should match your BindGroupLayout count
    pipelineLayoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayoutImpl* const*>(&bindGroupLayout); // Note: use the pointer directly

    // Create the pipeline layout
    PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);
    pipelineDesc.layout = pipelineLayout;

    // Create the render pipeline
    pipeline = device.createRenderPipeline(pipelineDesc);

    // Release the pipeline layout after it is no longer needed
    pipelineLayout.release();
}
