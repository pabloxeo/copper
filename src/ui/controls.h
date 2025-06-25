#ifndef CONTROLS_H
#define CONTROLS_H

// Forward declarations
class Window;
class Renderer;
class Coder;

#include <webgpu/webgpu_cpp.h>

// No need to include window.h or coder.h here

class Controls {
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