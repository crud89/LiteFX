# Shaders

*Shaders* are programs that run on the GPU. Modern GPUs have a programmable *graphics pipeline*, consisting of several *stages*. Some of them
run shaders, while others are *fixed-function* stages, which are configured through settings. A set of shaders that work together is called
a *shader program*. A graphics shader program needs at least two of them:

- **A vertex shader**, which runs once for each vertex of the geometry.
- **A fragment shader** (called *pixel shader* in DirectX), which runs once for each pixel covered by the geometry and computes its color.

!!! tip "Shader stage names"

    For the most part shader stage names are used as interchangeable synonyms throughout the engine. For example, the `TARGET_ADD_SHADER_PROGRAM`
    (and other CMake helpers) use the `PIXEL` or `FRAGMENT` keywords to refer to the same shader stage. The engine maps the following names to the 
    same concepts:

    - *Pixel* and *Fragment*
    - *Hull* and *Tessellation Evaluation*
    - *Domain* and *Tessellation Control*
    - *Amplification* and *Task*

    If a synonym is not defined explicitly, it may be assumed.

## Writing the shaders

Graphics APIs expect shaders as pre-compiled *byte code*, in their own format: *SPIR-V* for Vulkan and *DXIL* for DirectX 12. This tutorial
writes its shaders in *HLSL*, which can be compiled into both formats. That way, both backends use the same shader source.

Create a `shaders` directory in your project and add the file `tutorial.hlsl`. It contains both shaders, each in its own function:

```hlsl title="shaders/tutorial.hlsl"
struct VertexInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};

struct VertexData
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

struct FragmentData
{
    float4 Color : SV_TARGET;
};

VertexData VSMain(in VertexInput input)
{
    VertexData vertex;
    vertex.Position = input.Position;
    vertex.Color = input.Color;
    return vertex;
}

FragmentData PSMain(in VertexData input)
{
    FragmentData fragment;
    fragment.Color = input.Color;
    return fragment;
}
```

**The vertex shader** (`VSMain`) receives a vertex in the layout of the `Vertex` structure of your application. The identifiers after the
fields, such as `POSITION` and `COLOR`, are *semantics*: they tell the GPU what a field contains. The shader passes the position and color on
without changes. The `SV_POSITION` semantic marks the position the GPU uses to place the vertex on screen.

**The fragment shader** (`PSMain`) receives the output of the vertex shader. For pixels inside the triangle, the GPU *interpolates* between the
values of the three vertices, so that the colors blend smoothly across the triangle. The shader returns this color. The `SV_TARGET` semantic
tells the GPU to write it into the (first) render target.

## Compiling the shaders

The engine provides the `TARGET_ADD_SHADER_PROGRAM` CMake helper, which compiles a shader program for all backends of the engine and copies
the results next to your application. Add it to your `CMakeLists.txt`, after the application:

```cmake title="CMakeLists.txt"
TARGET_ADD_SHADER_PROGRAM(MyApp
    SOURCE "shaders/tutorial.hlsl"
    VERTEX VSMain
    PIXEL  PSMain
    SHADER_MODEL 6_5
)
```

`SOURCE` names the shader file, and each stage names the function that implements it (its *entry point*). The helper compiles each stage
into its own file, named after the source file and the stage. When you build the project, you find them in the `shaders` directory next to
your application:

| File | Contains |
|---|---|
| `tutorial_vs.spv` | The vertex shader for Vulkan (SPIR-V) |
| `tutorial_fs.spv` | The fragment shader for Vulkan (SPIR-V) |
| `tutorial_vs.dxi` | The vertex shader for DirectX 12 (DXIL) |
| `tutorial_fs.dxi` | The fragment shader for DirectX 12 (DXIL) |

!!! tip

    Shaders can also be written in separate files, one per stage. In that case, pass the files to the stages instead of `SOURCE`, e.g.
    `VERTEX "shaders/tutorial_vs.hlsl"`. See the documentation in `Shaders.cmake` for all options.

## Loading the shader program

Back in the start handler, load the shader program for the backend that is being started. Both backends use a different byte code format,
so the file extension is chosen based on the backend:

```cpp title="main.cpp"
auto startCallback = [this]<typename TBackend>(TBackend* backend) {
    // Alias type names for improved readability.
    using RenderPass = TBackend::render_pass_type;
    using FrameBuffer = TBackend::frame_buffer_type;
    using ShaderProgram = TBackend::shader_program_type;

    // ...

    // Create the frame buffers.
    // ...

    // Select the shader byte code for the backend.
    const String extension = std::is_same_v<TBackend, VulkanBackend> ? "spv" : "dxi";

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule(std::format("shaders/tutorial_vs.{}", extension), "VSMain")
        .withFragmentShaderModule(std::format("shaders/tutorial_fs.{}", extension), "PSMain");

    // ...

    m_device = device;
    return true;
};
```

Each shader module is loaded from its file, together with the name of its entry point.

!!! note "Working directory"

    The shader files are loaded relative to the working directory, so the application must be started from the directory that contains it.
    If you start it from the command line, change into the `binaries` directory of your build first.

The shader program describes the programmable stages of the pipeline. In the [next part](pipeline.md), you configure the fixed-function stages
and create the render pipeline.
