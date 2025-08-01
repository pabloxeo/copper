//
// Created by pabloxeo on 3/11/25.
//

#include "./Renderer.h"
#include "../utils/wgpu-util.h"

using namespace wgpu;


bool Renderer::Init(Window *nwindow) {
    this->window = nwindow;
    
    // Set window user pointer to this renderer instance
    window->SetWindowUserPointer(this);
    
    // Set resize callback
    glfwSetFramebufferSizeCallback(window->getWindow(), Window::FramebufferSizeCallback);

    glfwSetMouseButtonCallback(window->getWindow(), Window::MouseButtonCallback);
    
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
    uniformsData.floor = 1;
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

    int width, height;
    glfwGetFramebufferSize(window->getWindow(), &width, &height);

    SurfaceConfiguration config{
        .device = device,
        .format = format,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),};
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
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;
    blendState.alpha.srcFactor = BlendFactor::One;
    blendState.alpha.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.alpha.operation = BlendOperation::Add;

    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = ColorWriteMask::All;

    FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fragmentMain";  // Specify the fragment shader entry point
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    int width, height;
    glfwGetFramebufferSize(window->getWindow(), &width, &height);

    TextureDescriptor depthDesc{};
    depthDesc.size.width = static_cast<uint32_t>(width);
    depthDesc.size.height = static_cast<uint32_t>(height);
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
        //CreatePickingPipeline();
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

    uniformsData.mvp_matrix = this->camera->get_view_matrix();
    uniformsData.aspect_ratio = float(actualWidth) / float(actualHeight);
    auto selectedObjectProperties = coder->getSelectedObjectProperties(coder->getSelectedObjectId());
    uniformsData.size = selectedObjectProperties.size;
    uniformsData.position = selectedObjectProperties.position;
    uniformsData.color = selectedObjectProperties.color;
    double mouseX, mouseY;
    glfwGetCursorPos(window->getWindow(), &mouseX, &mouseY);
    float normalizedX = static_cast<float>(mouseX) / actualWidth;
    float normalizedY = static_cast<float>(mouseY) / actualHeight;
    uniformsData.mouse_position = glm::vec2((float)normalizedX, (float)normalizedY);
    uniformsData.picked_id = coder->getSelectedObjectId();
    uniformsData.floor = this->floor ? 1 : 0;
    //std::cout << "Floor: " << (uniformsData.floor ? "Enabled" : "Disabled") << std::endl;
    //std::cout << "Mouse Position: " << uniformsData.mouse_position.x << ", " << uniformsData.mouse_position.y << std::endl;
    

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

void Renderer::updateSelectedId(){

    RenderPassColorAttachment pickingAttachment{
        .view = pickingTextureView,
        .loadOp = LoadOp::Clear,
        .storeOp = StoreOp::Store,
        .clearValue = {-1.0f, 0.0f, 0.0f, 0.0f}
    };
    RenderPassDescriptor pickingRenderpass{
        .colorAttachmentCount = 1,
        .colorAttachments = &pickingAttachment
    };
    CommandEncoder pickingEncoder = device.CreateCommandEncoder();
    RenderPassEncoder pickingPass = pickingEncoder.BeginRenderPass(&pickingRenderpass);
    
    uint32_t actualWidth = pickingTexture.GetWidth();
    uint32_t actualHeight = pickingTexture.GetHeight();

    uint32_t scissorWidth = std::min(actualWidth, actualWidth);
    uint32_t scissorHeight = std::min(actualHeight, actualHeight);


    pickingPass.SetViewport(0, 0, float(scissorWidth), float(scissorHeight), 0.0f, 1.0f);
    pickingPass.SetScissorRect(0, 0, scissorWidth, scissorHeight);

    pickingPass.SetPipeline(pickingPipeline);
    pickingPass.SetBindGroup(0, aspectRatioBindGroup); // Add this line
    pickingPass.Draw(6, 1, 0, 0);
    
    BufferDescriptor readbackDesc = {};
    readbackDesc.usage = BufferUsage::MapRead | BufferUsage::CopyDst;
    readbackDesc.size = 256;
    readbackDesc.mappedAtCreation = false;
    Buffer readbackBuffer = device.CreateBuffer(&readbackDesc);
    
    
    TexelCopyTextureInfo source = {};
    source.texture = pickingTexture;
    source.origin = {0, 0, 0};
    source.mipLevel = 0;
    source.aspect = TextureAspect::All;
    
    TexelCopyBufferInfo destination = {};
    destination.buffer = readbackBuffer;
    destination.layout.offset = 0;
    destination.layout.bytesPerRow = 256; // 4 bytes for R32Sint format
    destination.layout.rowsPerImage = 1;
    
    Extent3D extent = {1, 1, 1};
    
    
    pickingPass.End();
    pickingEncoder.CopyTextureToBuffer(
        &source,
        &destination,
        &extent
    );

    struct MapUserData {
        Buffer buffer;
        Coder *coder;
        Renderer *renderer;
    };

    MapUserData* userdata = new MapUserData{readbackBuffer, coder, this}; // manage lifetime properly

    WGPUBufferMapCallbackInfo mappedReadyCallback = {
        .nextInChain = nullptr,
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = [](WGPUMapAsyncStatus status, WGPUStringView, void* userdata, void*) {
            MapUserData* data = reinterpret_cast<MapUserData*>(userdata);
            if (status == WGPUMapAsyncStatus_Success) {
                const int8_t* mapped = reinterpret_cast<const int8_t*>(data->buffer.GetConstMappedRange());
                int value = *reinterpret_cast<const int*>(mapped);
                if(data->coder && value > 0) {
                    data->coder->setSelectedObjectId(value);
                    data->renderer->pipelineDirty = true;
                }else if(data->coder) {
                    data->coder->setSelectedObjectId(-1);
                    data->renderer->pipelineDirty = true;
                }

                //std::cout << "Selected Object ID: " << value << std::endl;
                data->buffer.Unmap();
            } else {
                std::cerr << "Buffer mapping failed with status: " << status << "\n";
            }
            delete data; // clean up userdata
        },
        .userdata1 = userdata,
        .userdata2 = nullptr
    };

    CommandBuffer pickingCommands = pickingEncoder.Finish();
    device.GetQueue().Submit(1, &pickingCommands);

    wgpuBufferMapAsync(readbackBuffer.MoveToCHandle(), WGPUMapMode_Read, 0, 256, mappedReadyCallback);
}


void Renderer::CreatePickingPipeline(){
    ShaderModuleWGSLDescriptor wgslDesc{};
    coder->generateShaderCode();
    std::string shaderSource = coder->getShaderCode(); // or from file
    wgslDesc.code = shaderSource.c_str();

    ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgslDesc };
    ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);


    TextureDescriptor pickingTextureDesc{};
    pickingTextureDesc.size.width = 1;
    pickingTextureDesc.size.height = 1;
    pickingTextureDesc.size.depthOrArrayLayers = 1;
    pickingTextureDesc.mipLevelCount = 1;
    pickingTextureDesc.sampleCount = 1;
    pickingTextureDesc.dimension = TextureDimension::e2D;
    pickingTextureDesc.format = TextureFormat::R32Sint;
    pickingTextureDesc.usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc;

    pickingTexture = device.CreateTexture(&pickingTextureDesc);

    pickingTextureView = pickingTexture.CreateView();

    ColorTargetState pickingColorTargetState{};
    pickingColorTargetState.format = TextureFormat::R32Sint;
    pickingColorTargetState.writeMask = ColorWriteMask::All;


    FragmentState pickingFragmentState{};
    pickingFragmentState.module = shaderModule;
    pickingFragmentState.entryPoint = "fragmentPickingMain";  // Specify the picking fragment
    pickingFragmentState.targetCount = 1;
    pickingFragmentState.targets = &pickingColorTargetState;

    RenderPipelineDescriptor pickingPipelineDesc{};
    pickingPipelineDesc.vertex.module = shaderModule;
    pickingPipelineDesc.vertex.entryPoint = "vertexMain";
    pickingPipelineDesc.fragment = &pickingFragmentState;
    pickingPipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;

    PipelineLayoutDescriptor pickingPipelineLayoutDesc{};
    pickingPipelineLayoutDesc.bindGroupLayoutCount = 1;
    pickingPipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
    auto pickingPipelineLayout = device.CreatePipelineLayout(&pickingPipelineLayoutDesc);

    pickingPipelineDesc.layout = pickingPipelineLayout;

    pickingPipeline = device.CreateRenderPipeline(&pickingPipelineDesc);


}

void Renderer::OnMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        CreatePickingPipeline();
        updateSelectedId();
    }
}