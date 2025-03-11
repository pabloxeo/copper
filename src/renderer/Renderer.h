//
// Created by pabloxeo on 3/11/25.
//

#ifndef COPPER_RENDERER_H
#define COPPER_RENDERER_H

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include "../ui/Window.h"

using namespace wgpu;

class Renderer {
public:
    bool Initialize(Window nwindow);

    void ReconfigureSurface(float newWidth, float newHeight);

    // Render a single frame
    void RenderFrame();

    void UpdateBuffers();

    void UpdateWindow();

    void release() {
        // Release all resources
        pipeline.release();
        surface.unconfigure();
        queue.release();
        surface.release();
        device.release();
    }


private:
    Device device;
    Queue queue;
    Surface surface;
    Window window;

    std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;
    TextureFormat surfaceFormat = TextureFormat::Undefined;

    RenderPipeline pipeline;
    BindGroup bindGroup;         // Bind group for uniform bindings
    BindGroupLayout bindGroupLayout;

    float lightPos[3] = {0.0f, 5.0f, 0.0f};  // Initialize light position

    Buffer uTimeBuffer;         // Buffer for time uniform
    Buffer uMouseBuffer;        // Buffer for mouse position uniform
    Buffer uFovBuffer;          // Buffer for field of view uniform
    Buffer uLightPositionBuffer;// Buffer for light position uniform
    Buffer uViewPositionBuffer; // Buffer for view position uniform
    Buffer uWindowSizeBuffer;   // Buffer for window size uniform


    void CreateBindGroupLayoutAndBuffers();

    void InitializePipeline();

    TextureView GetNextSurfaceTextureView();

};



#endif //COPPER_RENDERER_H
