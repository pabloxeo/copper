#ifndef INTERFAZ_H
#define INTERFAZ_H

class Renderer;

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <dawn/webgpu_cpp.h>
#include "../core/Coder.h"
#include "./Window.h"


class Interfaz{
public:
    void setCoderAndRenderer(Coder* coder, Renderer* renderer);
    bool initGui(Window window, wgpu::Device device, wgpu::TextureFormat format);
    void terminateGui();
    void updateGui(wgpu::RenderPassEncoder renderPass);
private:
    Coder* coder = nullptr;
    Renderer* renderer = nullptr;
};

#endif