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

@group(0) @binding(0) var<uniform> uTime: f32;

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
	var output: VertexOutput;
	var p = vec2f(0.0, 0.0);
    var offset = vec2f(-0.6875, -0.463);
    offset += 0.3 * vec2f(cos(uTime), sin(uTime));
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
    uv = (uv * 2.0) - 1.0;
	uv.y = -(uv.y * 720.0/1080.0);
    let ro = vec3(0.0, 0.0, 0.0);
    let rd = normalize(vec3(uv.x, uv.y, 1.0));
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

    let out = vec3(clamp(log(total_distance_traveled) * 0.3, 0.0, 1.0));


    return vec4(out, 1.0);
}

fn sdf(pos: vec3<f32>) -> f32 {
    //return min(sdf_sphere(pos), sdf_plane(pos));
    return min(opSmoothIntersect(sdf_sphere(pos),sdf_plane(pos),1.5),opSmoothIntersect(sdf_sphere(pos), sdf_sphere2(pos), 1.0));
}




fn opSmoothIntersect(d1: f32, d2: f32, k: f32) -> f32 {
  let h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.);
  return mix(d2, d1, h) - k * h * (1. - h);
}

fn sdf_sphere(pos: vec3<f32>) -> f32 {
    let animated_position = vec3(sin(uTime) * 1.0, -0.5, cos(uTime) * 1.0 + 5.0);
    return length(pos - animated_position) - 1.0;
}
fn sdf_sphere2(pos: vec3<f32>) -> f32 {
    return length(pos - vec3(0.5, 1.5, 5.0)) - 1.0;
}
fn sdf_plane(pos: vec3<f32>) -> f32 {
    return pos.y + 1.0;
}
)";


bool App::Initialize() {
	// Open window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1080, 720, "Learn WebGPU", nullptr, nullptr);

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
    BindGroupLayoutEntry bglEntry = {};
    bglEntry.binding = 0;
    bglEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment; // Vertex and Fragment shader visibility
    bglEntry.buffer.type = BufferBindingType::Uniform; // Type is Uniform buffer
    bglEntry.buffer.minBindingSize = sizeof(float); // Size of the float

    BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    bindGroupLayout = device.createBindGroupLayout(bglDesc);

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
    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;
    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = FrontFace::CCW;
    pipelineDesc.primitive.cullMode = CullMode::None;

    FragmentState fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

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
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.depthStencil = nullptr;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    // Create a pipeline layout descriptor to reference the bind group layout
    PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = reinterpret_cast<WGPUBindGroupLayoutImpl* const*>(&bindGroupLayout);


    // Create the pipeline layout and assign it to the pipeline descriptor
    PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);
    pipelineDesc.layout = pipelineLayout;

    pipeline = device.createRenderPipeline(pipelineDesc);

    // Release pipeline layout after it is no longer needed
    pipelineLayout.release();

    // Create the buffer for uTime
    BufferDescriptor bufferDesc = {};
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    bufferDesc.mappedAtCreation = false;
    uTimeBuffer = device.createBuffer(bufferDesc);

    BindGroupEntry bgEntry = {};
    bgEntry.binding = 0;
    bgEntry.buffer = uTimeBuffer;
    bgEntry.offset = 0;
    bgEntry.size = sizeof(float);

    BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;

    bindGroup = device.createBindGroup(bgDesc);

    // Release shader module as it's no longer needed
    shaderModule.release();
}
