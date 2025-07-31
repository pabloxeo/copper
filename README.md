# Copper

**Copper** es un programa de diseño 3D escrito en C++ que utiliza [Dawn](https://dawn.googlesource.com/dawn/) (una implementación de WebGPU) como backend de renderizado. Este proyecto explora técnicas de ray marching con **SDFs (Signed Distance Functions)** para construir y visualizar geometría en tiempo real.

## ✨ Características

- Renderizado en tiempo real con WebGPU (vía Dawn)
- Modelado basado en SDFs (funciones de distancia)
- Operaciones de combinación: unión, intersección, resta y suavizado
- Interfaz gráfica con ImGui
- Gestión de ventana usando GLFW
- Shaders escritos en WGSL

## 📸 Capturas

> 

## 🧰 Requisitos

### Herramientas básicas

- CMake 3.15 o superior
- Compilador C++17 o superior
- GLFW
- ImGui

### Dependencias necesarias para compilar Dawn (nombres en Ubuntu)

Para compilar Dawn desde el código fuente, necesitarás instalar los siguientes paquetes:

```bash
sudo apt-get install \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    mesa-common-dev \
    libx11-xcb-dev \
    pkg-config \
    nodejs \
    npm
