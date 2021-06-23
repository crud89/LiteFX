---
title: "CMake Integration"
weight: 4
---

LiteFX already uses CMake for building. On top of that, it also exports the helpers it internally already uses to build its samples, so you can also use them in your application to automate building shaders or integrate assets into your build process. The CMake library allows you to define *shader module targets* and link them to your application just as you would with normal library targets. Shader modules can be written in HLSL and GLSL and can be compiled using [DXC](https://github.com/microsoft/DirectXShaderCompiler) and [GLSLC](https://github.com/google/shaderc/tree/main/glslc). Furthermore, it allows you to define a directory hierarchy for static assets.

```cmake
ADD_SHADER_MODULE(MyVertexShader SOURCE "vs.hlsl" 
  LANGUAGE HLSL TYPE VERTEX SHADER_MODEL 6_3 
  COMPILE_AS SPIRV COMPILER DXC)
TARGET_LINK_SHADERS(MyLiteFXApp SHADERS MyVertexShader)

TARGET_ADD_ASSET_DIRECTORY(MyLiteFXApp
    NAME "textures"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}"
    ASSETS "assets/texture.tga"
)
```