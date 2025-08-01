#ifndef WGPU_UTIL_H
#define WGPU_UTIL_H
\
#include <dawn/webgpu_cpp.h>
#include <iostream>
#include <functional>

void GetAdapter(const std::function<void(wgpu::Adapter)>& callback, wgpu::Adapter adapter, wgpu::Instance instance) {
    void *userdata = &adapter;
    wgpu::RequestAdapterOptions options{};
    instance.WaitAny(instance.RequestAdapter(
        &options, wgpu::CallbackMode::WaitAnyOnly,
        [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter,
            const char* message, void* userdata) {
            if (status != wgpu::RequestAdapterStatus::Success) {
                printf("RequestAdapter failed: %s\n", message);
                exit(0);
            }
            *static_cast<wgpu::Adapter *>(userdata) = adapter;
        }, 
    userdata), UINT64_MAX);
    callback(adapter);
}

void GetDevice(const std::function<void(wgpu::Device)>& callback, wgpu::Adapter adapter, wgpu::Instance instance, wgpu::Device device) {
    void *userdata = &device;
    wgpu::Limits requiredLimits = {};
    requiredLimits.maxBindGroups = 2;
    wgpu::DeviceDescriptor deviceDescriptor{};
    deviceDescriptor.requiredLimits = &requiredLimits;
    deviceDescriptor.SetUncapturedErrorCallback(
        []([[maybe_unused]] const wgpu::Device &device,[[maybe_unused]] wgpu::ErrorType type,wgpu::StringView message) {
            std::cout << "Error message: " << std::string(message.data, message.length);
        });
    deviceDescriptor.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        []([[maybe_unused]] const wgpu::Device&,[[maybe_unused]] wgpu::DeviceLostReason reason,
            wgpu::StringView message) {
            std::cout << "Device lost: " << std::string(message.data, message.length);
        });
    
    instance.WaitAny(adapter.RequestDevice(
        &deviceDescriptor, wgpu::CallbackMode::WaitAnyOnly,
        [](wgpu::RequestDeviceStatus status, wgpu::Device device, const char* message, void* userdata) {
            if (status != wgpu::RequestDeviceStatus::Success) {
                printf("RequestDevice failed: %s\n", message);
                exit(0);
            }
            *static_cast<wgpu::Device *>(userdata) = device;
        }, userdata), UINT64_MAX);
        callback(device);  
}

#endif