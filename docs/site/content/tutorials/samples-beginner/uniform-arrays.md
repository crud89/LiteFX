# Uniform arrays

!!! abstract "Sample 04"

    [`Samples/UniformArrays`](https://github.com/crud89/LiteFX/tree/main/src/Samples/UniformArrays) · builds on
    [basic rendering](basic-rendering.md).

Data that exists many times over, such as light sources or materials, does not need a buffer and a descriptor each. A buffer can hold an
array of elements that the shader indexes, and a single descriptor binds all of them. The sample lights a cube with eight light sources
passed in this way.

## The array in the shader

The light sources are an array of a constant buffer, bound next to the camera in the same descriptor set:

```hlsl
struct LightData
{
    float4 Position;
    float4 Color;
    float4 Properties;      // x: radius, y: intensity, w: enabled (if > 0.f)
};

ConstantBuffer<CameraData>    camera    : register(b0, space0);
ConstantBuffer<LightData>     lights[8] : register(b1, space0);
```

The fragment shader then accumulates the contribution of each light:

```hlsl
for (int l = 0; l < 8; l++)
{
    // Compute vector P -> Light Source.
    float3 L = lights[l].Position.xyz - P;

    // ... evaluate attenuation, diffuse and specular terms ...
}
```

Since the geometry is lit now, the vertices carry normals, which the input assembler passes on as another attribute:

```cpp
.withAttribute(BufferFormat::XYZ32F, offsetof(Vertex, Normal), AttributeSemantic::Normal)
```

The sample also replaces the tetrahedron with a cube of 24 vertices, so that each face can have its own normals.

## Allocating and filling the array

The buffer is created with the number of elements as its last argument, which makes it an array:

```cpp
constexpr UInt32 LIGHT_SOURCES = 8;

auto& staticBindingLayout = geometryPipeline.layout()->descriptorSet(DescriptorSets::Constant);
auto cameraBuffer = m_device->factory().createBuffer("Camera", staticBindingLayout, 0, ResourceHeap::Resource, 1);
auto lightsBuffer = m_device->factory().createBuffer("Lights", staticBindingLayout, 1, ResourceHeap::Resource, LIGHT_SOURCES);
```

The elements of such a buffer are aligned to the GPU's requirements, so they are not necessarily packed as tightly as in a C++ array.
Therefore the elements are transferred as a list of pointers, with the size of a single element, instead of as one block:

```cpp
commandBuffer->transfer(lights | std::views::transform([](const LightBuffer& light) { return static_cast<const void*>(&light); }) | std::ranges::to<Array<const void*>>(),
    sizeof(LightBuffer), *lightsBuffer, 0);
```

Binding is unchanged: the buffer is passed to `allocate` like a single-element one, and the shader indexes the array itself.

!!! tip "Larger arrays"

    The number of elements is part of the shader here, so it is fixed at compile time. For arrays whose size is only known at runtime, see
    the [bindless](../samples-intermediate/bindless.md) tutorial.
