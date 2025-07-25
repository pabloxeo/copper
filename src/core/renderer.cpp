//
// Created by pabloxeo on 3/11/25.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "./renderer.h"
#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>
#include "../utils/wgpu-util.h"



#ifndef RESOURCE_DIR
#define RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
#endif

using namespace wgpu;


bool Renderer::Init(Window *nwindow) {
    this->window = nwindow;
    
    // Set window user pointer to this renderer instance
    window->SetWindowUserPointer(this);
    
    // Set resize callback
    glfwSetFramebufferSizeCallback(window->getWindow(), Window::FramebufferSizeCallback);
    
    InstanceDescriptor instanceDesc{};
    instanceDesc.capabilities.timedWaitAnyEnable = true;
    instance = CreateInstance(&instanceDesc);
    
    GetAdapter([this](Adapter a) {
        adapter = a;
        GetDevice([this](Device d) {
            device = d;
            coder = new Coder(this);
            InitGraphics();
        }, adapter, instance, device);
    }, adapter, instance);

    this->camera = new Camera();
    this->cameraController = new CameraController(this->camera, nwindow->getWindow());


    return true;
}


void Renderer::InitGraphics() {
    ConfigureSurface();
    // Create aspect ratio buffer
    uniformsData.aspect_ratio = float(window->getWindowWidth()) / float(window->getWindowHeight());
    uniformsData.mvp_matrix = glm::mat4(1.0f);
    uniformsData.light_position = glm::vec3(0.0f, 5.0f, 0.0f); // Default light position
    BufferDescriptor bufferDesc{};
    bufferDesc.size = sizeof(Uniforms);
    bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
    uniformsBuffer = device.CreateBuffer(&bufferDesc);
    device.GetQueue().WriteBuffer(uniformsBuffer, 0, &uniformsData, sizeof(Uniforms));

    // Create bind group layout and bind group
    BindGroupLayoutEntry entry{};
    entry.binding = 0;
    entry.visibility = ShaderStage::Fragment;
    entry.buffer.type = BufferBindingType::Uniform;
    entry.buffer.minBindingSize = sizeof(Uniforms);

    BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &entry;
    bindGroupLayout = device.CreateBindGroupLayout(&layoutDesc);

    BindGroupEntry bgEntry{};
    bgEntry.binding = 0;
    bgEntry.buffer = uniformsBuffer;
    bgEntry.offset = 0;
    bgEntry.size = sizeof(Uniforms);

    BindGroupDescriptor bgDesc{};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;
    aspectRatioBindGroup = device.CreateBindGroup(&bgDesc);

    CreateRenderPipeline();
    gui.initGui(*window, device, format);
    gui.setCoderAndRenderer(coder, this);

} 

void Renderer::ConfigureSurface() {
    surface = glfw::CreateSurfaceForWindow(instance, window->getWindow());
    SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);
    format = capabilities.formats[0];

    SurfaceConfiguration config{
        .device = device,
        .format = format,
        .width = window->getWindowWidth(),
        .height = window->getWindowHeight()};
    config.presentMode = PresentMode::Fifo;
    surface.Configure(&config);
}

void Renderer::CreateRenderPipeline() {
    ShaderModuleWGSLDescriptor wgslDesc{};
    coder->generateShaderCode();
    std::string shaderSource = coder->getShaderCode(); // or from file
    wgslDesc.code = shaderSource.c_str();

    ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgslDesc };
    ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    ColorTargetState colorTargetState{.format = format};

    BlendState blendState{};
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;
    blendState.alpha.srcFactor = wgpu::BlendFactor::One;
    blendState.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.alpha.operation = wgpu::BlendOperation::Add;

    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = wgpu::ColorWriteMask::All;

    FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fragmentMain";  // Specify the fragment shader entry point
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    TextureDescriptor depthDesc{};
    depthDesc.size.width = window->getWindowWidth();
    depthDesc.size.height = window->getWindowHeight();
    depthDesc.size.depthOrArrayLayers = 1;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;
    depthDesc.dimension = TextureDimension::e2D;
    depthDesc.format = TextureFormat::Depth24Plus;
    depthDesc.usage = TextureUsage::RenderAttachment;

    depthStencilTexture = device.CreateTexture(&depthDesc);

    DepthStencilState depthStencilState{};
    depthStencilState.format = TextureFormat::Depth24Plus;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = CompareFunction::Less;

    RenderPipelineDescriptor descriptor{
        .vertex = {.module = shaderModule,
                    .entryPoint = "vertexMain"},
        .fragment = &fragmentState};
    descriptor.primitive.topology = PrimitiveTopology::TriangleList;
    descriptor.primitive.stripIndexFormat = IndexFormat::Undefined;
	descriptor.primitive.frontFace = FrontFace::CCW;
	descriptor.primitive.cullMode = CullMode::None;
    descriptor.depthStencil = &depthStencilState;

    PipelineLayoutDescriptor pipelineLayoutDesc{};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    auto pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    descriptor.layout = pipelineLayout;

    pipeline = device.CreateRenderPipeline(&descriptor);

}

void Renderer::Render() {

    this->cameraController->update_camera(this->window->getWindow());

    if (pipelineDirty) {
        ConfigureSurface(); 
        CreateRenderPipeline();
        pipelineDirty = false;
    }
    // Check if surface is valid before proceeding
    if (!surface) return;
    
    SurfaceTexture surfaceTexture;
    try {
        surface.GetCurrentTexture(&surfaceTexture);
    } catch (...) {
        return;
    }
    uint32_t actualWidth = surfaceTexture.texture.GetWidth();
    uint32_t actualHeight = surfaceTexture.texture.GetHeight();
    // Update aspect ratio buffer with actual render target size
    
    
    // device.GetQueue().WriteBuffer(uniformsBuffer, 0, &aspect, sizeof(float));

    uniformsData.mvp_matrix = this->camera->get_view_matrix();
    uniformsData.aspect_ratio = float(actualWidth) / float(actualHeight);


    device.GetQueue().WriteBuffer(uniformsBuffer, 0, &uniformsData, sizeof(Uniforms));

    RenderPassColorAttachment colorAttachment{
        .view = surfaceTexture.texture.CreateView(),
        .loadOp = LoadOp::Clear,
        .storeOp = StoreOp::Store,
        .clearValue = {0.1f, 0.1f, 0.1f, 1.0f}
    };

    RenderPassDepthStencilAttachment depthAttachment{
        .view = depthStencilTexture.CreateView(),
        .depthLoadOp = LoadOp::Clear,
        .depthStoreOp = StoreOp::Store,
        .depthClearValue = 1.0f,
    };

    RenderPassDescriptor renderpass{
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment,
        .depthStencilAttachment = &depthAttachment
    };

    CommandEncoder encoder = device.CreateCommandEncoder();
    RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);

    // Always use actualWidth/actualHeight from surfaceTexture for viewport/scissor
    uint32_t scissorWidth = std::min(actualWidth, actualWidth);
    uint32_t scissorHeight = std::min(actualHeight, actualHeight);

    pass.SetViewport(0, 0, float(scissorWidth), float(scissorHeight), 0.0f, 1.0f);
    pass.SetScissorRect(0, 0, scissorWidth, scissorHeight);

    // Render main content
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, aspectRatioBindGroup); // Add this line
    pass.Draw(6, 1, 0, 0);


    gui.updateGui(pass);

    pass.End();

    CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    surface.Present();
    instance.ProcessEvents();

}

void Renderer::Release(){
    window = nullptr;
    gui.terminateGui();
}

void Renderer::OnResize() {
    // Ensure the device is valid before proceeding
    if (!device) return;
    
    // Ensure all GPU operations are finished
    device.GetQueue().Submit(0, nullptr);
    
    // Get the new framebuffer size
    int width, height;
    glfwGetFramebufferSize(window->getWindow(), &width, &height);
    
    // Configure surface with new dimensions
    SurfaceConfiguration config{};
    config.device = device;
    config.format = format;
    config.width = static_cast<uint32_t>(width);
    config.height = static_cast<uint32_t>(height);
    config.presentMode = PresentMode::Fifo;
    surface.Configure(&config);
    
    // Recreate depth texture
    TextureDescriptor depthDesc{};
    depthDesc.size.width = static_cast<uint32_t>(width);
    depthDesc.size.height = static_cast<uint32_t>(height);
    depthDesc.size.depthOrArrayLayers = 1;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;
    depthDesc.dimension = TextureDimension::e2D;
    depthDesc.format = TextureFormat::Depth24Plus;
    depthDesc.usage = TextureUsage::RenderAttachment;
    
    // First destroy the old texture (explicitly release it)
    depthStencilTexture = {};
    
    // Then create a new one
    depthStencilTexture = device.CreateTexture(&depthDesc);
    
}