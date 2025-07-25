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
#include "./Camera.h"
#include "./CameraController.h"

using namespace wgpu;

struct Uniforms {
    glm::mat4 mvp_matrix; // Model-View-Projection matrix
    glm::vec3 light_position; // Position of the light source
    float aspect_ratio;
    //glm::vec3 size; // Size of the object
    //glm::vec3 color; // Color of the object
    //glm::vec3 position; // Position of the object in world space
    glm::vec3 _padding; // Padding to align the struct size to 16 bytes
};


class Renderer {
public:
    bool Init(Window *window);

    void InitGraphics();

    //void UpdateUniforms(float time, float mouseX, float mouseY, float lightX, float lightY, float lightZ, float viewX, float viewY, float viewZ, float fov, float windowWidth, float windowHeight);

    void ConfigureSurface();

    void CreateRenderPipeline();

    void Render();

    void Release();

    void OnResize();

    void setLightPosition(float x, float y, float z) {
        uniformsData.light_position = glm::vec3(x, y, z);
        pipelineDirty = true;
    }

    bool pipelineDirty = false;

    Window *window;

    Camera *camera;

    CameraController *cameraController;

private:

    Coder* coder;
    
    Uniforms uniformsData;
   
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
    Buffer uniformsBuffer;
    BindGroup aspectRatioBindGroup;

    Texture depthStencilTexture;

};



#endif //RENDERER_H