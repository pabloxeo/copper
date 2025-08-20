//
// Created by pabloxeo on 3/11/25.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <fstream>
#include <cstring>
#include <filesystem>
#include "../ui/Window.h"
#include "./Coder.h"
#include "./Camera.h"
#include "./CameraController.h"
#include "../ui/Interfaz.h"



using namespace wgpu;

struct Uniforms {
    glm::mat4 mvp_matrix; 
    glm::vec3 light_position;
    float aspect_ratio;
    glm::vec3 size;
    float padding;
    glm::vec3 color;
    float padding2;
    glm::vec3 position;
    float padding3;
    glm::vec2 mouse_position;
    int picked_id;
    float padd;
};


class Renderer {
public:
    bool Init(Window *window);

    void Render();

    void Release();

    void OnResize();

    void setLightPosition(float x, float y, float z) {
        uniformsData.light_position = glm::vec3(x, y, z);
    }

    glm::vec2 getMouseUv();
    float getAspectRatio();

    void OnMouseButton(int button, int action);

    void updateSelectedId();

    bool pipelineDirty = false;

    bool floor = true;

    Window *window;

    Camera *camera;

    CameraController *cameraController;

private:

    Coder* coder;
    
    Uniforms uniformsData;
   
    Interfaz gui;

    Instance instance;
    Adapter adapter;
    Device device;
    Queue queue;
    Surface surface;
    TextureFormat format;

    RenderPipeline pipeline;
    RenderPipeline pickingPipeline;

    BindGroup bindGroup;
    BindGroupLayout bindGroupLayout;
    Buffer uniformBuffer;
    Buffer uniformsBuffer;
    BindGroup aspectRatioBindGroup;

    Texture depthStencilTexture;
    Texture pickingTexture;

    TextureView pickingTextureView;


    void InitGraphics();

    //void UpdateUniforms(float time, float mouseX, float mouseY, float lightX, float lightY, float lightZ, float viewX, float viewY, float viewZ, float fov, float windowWidth, float windowHeight);

    void ConfigureSurface();

    void CreateRenderPipeline();

    void CreatePickingPipeline();


};



#endif //RENDERER_H