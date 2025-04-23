#include "./window.h"
#include <webgpu/webgpu_cpp.h> 

#ifndef CONTROLS_H
#define CONTROLS_H

class Controls {
    public:
        bool initGui(Window window, wgpu::Device device, wgpu::TextureFormat format); // called in onInit
        void terminateGui(); // called in onFinish
        void updateGui(wgpu::RenderPassEncoder renderPass); // called in onFrame
    };

#endif