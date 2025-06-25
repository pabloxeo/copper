//
// Created by pabloxeo on 3/11/25.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>
#include "../ui/window.h"
#include "../ui/controls.h"
#include "./coder.h"

using namespace wgpu;

class Renderer {
public:
    bool Init(Window *window);

    void InitGraphics();

    //void UpdateUniforms(float time, float mouseX, float mouseY, float lightX, float lightY, float lightZ, float viewX, float viewY, float viewZ, float fov, float windowWidth, float windowHeight);

    void ConfigureSurface();

    void CreateRenderPipeline();

    void Render();

    void UpdateWindow();

    void Release();

    void OnResize();

    bool pipelineDirty = false;

private:

    Coder shaderCode;
    
    Window *window;
    Controls gui;

    Instance instance;
    Adapter adapter;
    Device device;
    Queue queue;
    Surface surface;
    TextureFormat format;

    RenderPipeline pipeline;

    BindGroup bindGroup;
    BindGroupLayout bindGroupLayout;
    Buffer uniformBuffer;
    Buffer aspectRatioBuffer;
    BindGroup aspectRatioBindGroup;

    wgpu::Texture depthStencilTexture;

};



#endif //RENDERER_H