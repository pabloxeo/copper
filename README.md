# Copper

**Copper** es un programa de diseño 3D escrito en C++ que utiliza [Dawn](https://dawn.googlesource.com/dawn/) (una implementación de WebGPU) como backend de renderizado. Este proyecto explora técnicas de ray marching con **SDFs (Signed Distance Functions)** para construir y visualizar geometría en tiempo real.

## ✨ Características

- Renderizado en tiempo real con WebGPU (vía Dawn)
- Modelado basado en SDFs (funciones de distancia)
- Operaciones de combinación: unión, intersección y resta.
- Interfaz gráfica con ImGui
- Gestión de ventana usando GLFW
- Shaders escritos en WGSL

## 📸 Capturas

> *Próximamente - Las capturas de pantalla se añadirán cuando el proyecto esté más avanzado*

## 🧰 Requisitos

### Herramientas básicas

- CMake 3.15 o superior
- Compilador C++17 o superior
- GLFW
- ImGui

### Dependencias necesarias para compilar Dawn (nombres en Ubuntu)

Para compilar Dawn desde el código fuente, necesitarás instalar los siguientes paquetes:

```bash
sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev mesa-common-dev libx11-xcb-dev pkg-config nodejs npm
```

## 🚀 Compilación

### Clonando el repositorio
```bash
git clone https://github.com/tu-usuario/copper.git
cd copper
git submodule update --init
```

### Compilación en Linux
```bash
cmake -B build
cmake --build build -j$(nproc)
```

## 🎮 Uso

### Ejecutando Copper
Desde la carpeta del proyecto:
```bash
./build/copper
```

### Controles básicos
- **Click izquierdo + arrastre**: Rotar la cámara
- **Rueda del ratón**: Zoom in/out
- **Click derecho + arrastre**: Pan (desplazar vista)

### Interfaz de usuario

>No es definitiva


## 🛠️ Roadmap

- [x] Configuración básica del proyecto
- [x] Integración con Dawn/WebGPU
- [x] Ray marching
- [x] SDFs primitivas (esfera, caja)
- [x] Interfaz ImGui básica
- [x] Sistema de cámara completo
- [ ] Picking de objetos
- [x] Operaciones boolenas
- [x] Iluminación y sombras
- [ ] Exportación en malla de triángulos
- [ ] Build estática

## 📚 Recursos útiles

### SDFs y Ray Marching
- [Inigo Quilez - SDF Functions](https://iquilezles.org/articles/distfunctions/)
- [Ray Marching Tutorial](https://michaelwalczyk.com/blog-ray-marching.html)
- [Shadertoy - SDF Examples](https://www.shadertoy.com/results?query=sdf)

### WebGPU y Dawn
- [WebGPU Specification](https://gpuweb.github.io/gpuweb/)
- [Dawn Documentation](https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/)
- [Learn WebGPU](https://eliemichel.github.io/LearnWebGPU/)



