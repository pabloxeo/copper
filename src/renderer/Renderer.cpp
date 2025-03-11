//
// Created by pabloxeo on 3/11/25.
//

#include "Renderer.h"
#include "../ui/Window.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
#endif

bool Renderer::Initialize(Window nwindow){

    this->window = nwindow;

    Instance instance = wgpuCreateInstance(nullptr);

    std::cout << "Requesting adapter..." << std::endl;
    surface = glfwGetWGPUSurface(instance, window.GetWindow());
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
    config.width = window.getWindowWidth();
    config.height = window.getWindowHeight();
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

    // Create GPU resources (buffers, bind group layouts, etc.)
    CreateBindGroupLayoutAndBuffers();

    // Initialize pipeline
    InitializePipeline();

    return true;
}

void Renderer::CreateBindGroupLayoutAndBuffers()
{
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

    BindGroupLayoutEntry uFovLayoutEntry = {};
    uFovLayoutEntry.binding = 4;
    uFovLayoutEntry.visibility = ShaderStage::Fragment;
    uFovLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uFovLayoutEntry.buffer.minBindingSize = sizeof(float);

    BindGroupLayoutEntry uWindowSizeLayoutEntry = {};
    uWindowSizeLayoutEntry.binding = 5;
    uWindowSizeLayoutEntry.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    uWindowSizeLayoutEntry.buffer.type = BufferBindingType::Uniform;
    uWindowSizeLayoutEntry.buffer.minBindingSize = 2 * sizeof(float);


    // Set up layout entries array and descriptor
    BindGroupLayoutEntry layoutEntries[] = {
            uTimeLayoutEntry,
            uMousePositionLayoutEntry,
            uLightPositionLayoutEntry,
            uViewPositionLayoutEntry,
            uFovLayoutEntry,
            uWindowSizeLayoutEntry  // Add the new layout entry
    };
    BindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
    bindGroupLayoutDescriptor.entryCount = 6;
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

    BufferDescriptor uFovBufferDesc = {};
    uFovBufferDesc.size = sizeof(float);
    uFovBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uFovBuffer = device.createBuffer(uFovBufferDesc);

    BufferDescriptor uWindowSizeBufferDesc = {};
    uWindowSizeBufferDesc.size = 2 * sizeof(float);     // Size for vec2<f32>
    uWindowSizeBufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uWindowSizeBuffer = device.createBuffer(uWindowSizeBufferDesc);


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

    BindGroupEntry fovBufferEntry = {};
    fovBufferEntry.binding = 4;
    fovBufferEntry.buffer = uFovBuffer;
    fovBufferEntry.offset = 0;
    fovBufferEntry.size = sizeof(float);

    BindGroupEntry windowSizeBufferEntry = {};
    windowSizeBufferEntry.binding = 5;                  // Corresponds to `@binding(5)` in the shader
    windowSizeBufferEntry.buffer = uWindowSizeBuffer;   // Assign the buffer created above
    windowSizeBufferEntry.offset = 0;                   // Offset is zero for the entire buffer
    windowSizeBufferEntry.size = 2 * sizeof(float);     // Size matches vec2<f32>

    BindGroupEntry bindGroupEntries[] = {
            timeBufferEntry,
            mouseBufferEntry,
            lightBufferEntry,
            viewBufferEntry,
            fovBufferEntry,
            windowSizeBufferEntry // Add this new entry
    };
    BindGroupDescriptor bindGroupDesc = {};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 6;
    bindGroupDesc.entries = bindGroupEntries;

    // Create the bind group
    bindGroup = device.createBindGroup(bindGroupDesc);
}

void Renderer::InitializePipeline() {
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
    std::ifstream file(RESOURCE_DIR "/shaders/shader.wgsl");
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);
    shaderCodeDesc.code = shaderSource.c_str();
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

void Renderer::RenderFrame()
{
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

    queue.submit(1, &command);
    command.release();
    // At the end of the frame
    targetView.release();
#ifndef __EMSCRIPTEN__
    surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
    device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
    device.poll(false);
#endif


}

void Renderer::UpdateBuffers() {
        // Example of updating uTime each frame
        float currentTime = static_cast<float>(glfwGetTime());
        queue.writeBuffer(uTimeBuffer, 0, &currentTime, sizeof(float));
    
        double xpos, ypos;
        glfwGetCursorPos(window.GetWindow(), &xpos, &ypos);
    
        // Convert to normalized coordinates (-1 to 1)
        float mousePos[2] = {
                static_cast<float>((xpos / window.getWindowWidth()) * 2.0 - 1.0),
                static_cast<float>((1.0 - (ypos / window.getWindowHeight())) * 2.0 - 1.0)
        };
    
        queue.writeBuffer(uLightPositionBuffer, 0, &lightPos, sizeof(lightPos));
    
        float viewPos[3] = {0.0f, 0.0f, 0.0f};  // Sample view position, update as needed
        queue.writeBuffer(uViewPositionBuffer, 0, &viewPos, sizeof(viewPos));
    
        float fov = 1.;
        queue.writeBuffer(uFovBuffer, 0, &fov, sizeof(float));
    
        // Update buffer
        queue.writeBuffer(uMouseBuffer, 0, &mousePos, sizeof(mousePos));
}

void Renderer::UpdateWindow(){
    float windowSizeData[2] = { window.getWindowWidth(), window.getWindowHeight() };
        queue.writeBuffer(uWindowSizeBuffer, 0, windowSizeData, sizeof(windowSizeData));
}

void Renderer::ReconfigureSurface(float newWidth, float newHeight) {

    SurfaceConfiguration config = {};
    config.width = newWidth;
    config.height = newHeight;
    config.usage = TextureUsage::RenderAttachment;
    config.format = surfaceFormat;
    config.presentMode = PresentMode::Fifo;
    config.alphaMode = CompositeAlphaMode::Auto;
    config.device = device;

    surface.configure(config);
}

TextureView Renderer::GetNextSurfaceTextureView() {
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