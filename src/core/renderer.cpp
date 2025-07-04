//
// Created by pabloxeo on 3/11/25.
//

#include "../ui/window.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include "./renderer.h"
#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>
#include "../utils/wgpu-util.h"
#include "renderer.h"


#ifndef RESOURCE_DIR
#define RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
#endif

using namespace wgpu;


bool Renderer::Init(Window *nwindow) {
    this->window = nwindow;
    
    // Set window user pointer to this renderer instance
    window->SetWindowUserPointer(this);
    
    // Set resize callback
    glfwSetFramebufferSizeCallback(window->GetWindow(), Window::FramebufferSizeCallback);
    
    InstanceDescriptor instanceDesc{};
    instanceDesc.capabilities.timedWaitAnyEnable = true;
    instance = CreateInstance(&instanceDesc);
    
    GetAdapter([this](Adapter a) {
        adapter = a;
        GetDevice([this](Device d) {
            device = d;
            InitGraphics();
        }, adapter, instance, device);
    }, adapter, instance);


    return true;
}

void Renderer::InitGraphics() {
    ConfigureSurface();
    // Create aspect ratio buffer
    float aspect = float(window->getWindowWidth()) / float(window->getWindowHeight());
    wgpu::BufferDescriptor bufferDesc{};
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    aspectRatioBuffer = device.CreateBuffer(&bufferDesc);
    device.GetQueue().WriteBuffer(aspectRatioBuffer, 0, &aspect, sizeof(float));

    // Create bind group layout and bind group
    wgpu::BindGroupLayoutEntry entry{};
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry.buffer.minBindingSize = sizeof(float);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &entry;
    bindGroupLayout = device.CreateBindGroupLayout(&layoutDesc);

    wgpu::BindGroupEntry bgEntry{};
    bgEntry.binding = 0;
    bgEntry.buffer = aspectRatioBuffer;
    bgEntry.offset = 0;
    bgEntry.size = sizeof(float);

    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;
    aspectRatioBindGroup = device.CreateBindGroup(&bgDesc);

    CreateRenderPipeline();
    gui.initGui(*window, device, format);
    gui.setCoderAndRenderer(&shaderCode, this);
} 

void Renderer::ConfigureSurface() {
    surface = glfw::CreateSurfaceForWindow(instance, window->GetWindow());
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
    std::string shaderSource = shaderCode.getShaderCode(); // or from file
    wgslDesc.code = shaderSource.c_str();

    ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgslDesc };
    ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    ColorTargetState colorTargetState{.format = format};

    FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fragmentMain";  // Specify the fragment shader entry point
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::TextureDescriptor depthDesc{};
    depthDesc.size.width = window->getWindowWidth();
    depthDesc.size.height = window->getWindowHeight();
    depthDesc.size.depthOrArrayLayers = 1;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;
    depthDesc.dimension = wgpu::TextureDimension::e2D;
    depthDesc.format = wgpu::TextureFormat::Depth24Plus;
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;

    depthStencilTexture = device.CreateTexture(&depthDesc);

    wgpu::DepthStencilState depthStencilState{};
    depthStencilState.format = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;

    RenderPipelineDescriptor descriptor{
        .vertex = {.module = shaderModule,
                    .entryPoint = "vertexMain"},
        .fragment = &fragmentState};
    descriptor.primitive.topology = PrimitiveTopology::TriangleList;
    descriptor.primitive.stripIndexFormat = IndexFormat::Undefined;
	descriptor.primitive.frontFace = FrontFace::CCW;
	descriptor.primitive.cullMode = CullMode::None;
    descriptor.depthStencil = &depthStencilState;

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    auto pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    descriptor.layout = pipelineLayout;

    pipeline = device.CreateRenderPipeline(&descriptor);

}

void Renderer::Render() {
    // Always update aspect ratio buffer before rendering
    float aspect = float(window->getWindowWidth()) / float(window->getWindowHeight());
    device.GetQueue().WriteBuffer(aspectRatioBuffer, 0, &aspect, sizeof(float));

    if (pipelineDirty) {
        ConfigureSurface();      // <-- Add this line
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
    aspect = float(actualWidth) / float(actualHeight);
    device.GetQueue().WriteBuffer(aspectRatioBuffer, 0, &aspect, sizeof(float));

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

void Renderer::UpdateWindow() {
    //float windowSizeData[2] = { window->getWindowWidth(), window->getWindowHeight() };
    // Update any buffers or resources that depend on the window size
    //queue.WriteBuffer(uWindowSizeBuffer, 0, windowSizeData, sizeof(windowSizeData));
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
    glfwGetFramebufferSize(window->GetWindow(), &width, &height);
    
    // Make sure dimensions are valid
    if (width < 64) width = 64;
    if (height < 64) height = 64;
    
    // Configure surface with new dimensions
    SurfaceConfiguration config{};
    config.device = device;
    config.format = format;
    config.width = static_cast<uint32_t>(width);
    config.height = static_cast<uint32_t>(height);
    config.presentMode = PresentMode::Fifo;
    surface.Configure(&config);
    
    // Recreate depth texture
    wgpu::TextureDescriptor depthDesc{};
    depthDesc.size.width = static_cast<uint32_t>(width);
    depthDesc.size.height = static_cast<uint32_t>(height);
    depthDesc.size.depthOrArrayLayers = 1;
    depthDesc.mipLevelCount = 1;
    depthDesc.sampleCount = 1;
    depthDesc.dimension = wgpu::TextureDimension::e2D;
    depthDesc.format = wgpu::TextureFormat::Depth24Plus;
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
    
    // First destroy the old texture (explicitly release it)
    depthStencilTexture = {};
    
    // Then create a new one
    depthStencilTexture = device.CreateTexture(&depthDesc);
    
    // Update aspect ratio buffer
    float aspect = float(width) / float(height);
    device.GetQueue().WriteBuffer(aspectRatioBuffer, 0, &aspect, sizeof(float));
    
    // Log the resize operation
    //std::cout << "Window resized to " << width << "x" << height << std::endl;
}