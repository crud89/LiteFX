---
title: LiteFX ✨
---

{{< blocks/cover image_anchor="top" height="full" >}}
<style type="text/css">
    section#td-cover-block-0 {
        min-height: 100vh;
        background: rgb(2,0,36);
        background: linear-gradient(150deg, rgba(2,0,36,1) 0%, rgba(42,42,79,1) 50%, rgba(60,81,89,1) 100%);
    }

    section.td-box--gradient {
        background: #403f4c;
        background: linear-gradient(180deg, #403f4c 0%,rgb(39, 38, 49) 100%);
    }

    h1.title-gradient, h2.title-gradient {
        font-family: sans-serif;
        letter-spacing: 0.4em;
    }

    h1.title-gradient {
        font-size: clamp(2rem, 18vmin, 8rem);
    }

    h2.title-gradient {
        font-size: clamp(2rem, 18vmin, 2rem);
    }

    .title-gradient {
        --bg-size: 400%;
        --color-one: #84d0e5;
        --color-two: #f3a8f2;
        background: linear-gradient(
            240deg,
            var(--color-one),
            var(--color-two),
            var(--color-one)
        )  0 0 / var(--bg-size) 100%;
        color: transparent;
        background-clip: text;
    }

    @media (prefers-reduced-motion: no-preference) {
        .title-gradient {
            animation: move-bg 20s linear infinite;
        }
        @keyframes move-bg {
            to {
                background-position: var(--bg-size) 0;
            }
        }
    }
</style>

<img src="logo.svg" width="256" height="256" class="mb-5" />

<h1 class="title-gradient mb-3">LiteFX</h1>
<h2 class="title-gradient mb-5">Modern Computer Graphics Engine</h2>

<a class="btn btn-lg btn-primary mt-3 me-3 mb-4" href="https://github.com/crud89/litefx/releases">
  Latest Release <i class="fas fa-circle-down ms-2"></i>
</a>
<a class="btn btn-lg btn-secondary mt-3 me-3 mb-4" href="https://github.com/crud89/litefx">
  GitHub <i class="fab fa-github ms-2 "></i>
</a>
<a class="btn btn-lg btn-secondary mt-3 me-3 mb-4" href="/docs/">
  Documentation <i class="fas fa-book ms-2"></i>
</a>
<a class="btn btn-lg btn-secondary mt-3 me-3 mb-4" href="https://github.com/crud89/litefx/wiki">
  Wiki <i class="fas fa-globe ms-2"></i>
</a>

Welcome to LiteFX ✨ - A modern, flexible computer graphics and rendering engine, written in C++23 with support for Vulkan 🌋 and DirectX 12 ❎. It contains an abstraction layer over both graphics APIs, that allows for seamless switching between them, without re-creating the application window. The engine can be configured through a fluent builder interface, making it possible to easily and quickly express render pipeline configurations. It uses [CMake](https://cmake.org/) and [vcpkg](https://vcpkg.io/) to manage dependencies and make it straightforward to build. It supports custom shader targets (written in HLSL or GLSL), allowing you to setup a basic, fully featured rendering application within seconds.

Interested? You can start right now by heading over to the documentation and read the [project setup](https://litefx.crudolph.io/docs/md_docs_tutorials_project_setup.html) and [quick start](https://litefx.crudolph.io/docs/md_docs_tutorials_quick_start.html) guides. The source code is available on [GitHub](https://github.com/crud89/LiteFX).

{{< blocks/link-down color="info" >}}
{{< /blocks/cover >}}


{{% blocks/lead color="primary" %}}

<h1>Designed to make Modern Graphics APIs accessible.</h1>

{{% /blocks/lead %}}


{{% blocks/section color="dark" type="row" %}}
{{% blocks/feature icon="fa-lightbulb" title="State of the Art" %}}
LiteFX supports many state-of-the-art features of modern graphics APIs, such as **Mesh Shaders**, **Raytracing** and **Indirect Draw**. It utilizes features and extensions from the *DirectX 12 Agility SDK* and *Vulkan 1.3* for a better user experience, such as **Enhanced Barriers** and **Dynamic Rendering**.
{{% /blocks/feature %}}


{{% blocks/feature icon="fa-gears" title="Streamlined API" %}}
LiteFX provides a streamlined, object-oriented API that allows for straightforward state management and to quickly setup render graphs. The optional fluent **Builder API** can be used to configure pipelines, shaders and render passes. **Shader Reflection** can be used to easily derive pipeline interfaces from shader programs.
{{% /blocks/feature %}}


{{% blocks/feature icon="fa-hammer" title="CMake Integration" %}}
LiteFX provides tools to integrate *assets* and *shaders* into your build process, which allows you to manage them next to your sources without leaving your IDE. Shaders can be authored in *GLSL* and *HLSL* and can target both supported APIs from a single source.
{{% /blocks/feature %}}
{{% /blocks/section %}}


{{% blocks/lead color="secondary" %}}

<h1>With support for state of the art rendering technqiues.</h1>

{{% /blocks/lead %}}


{{% blocks/section color="gradient" %}}

<div style="margin: 2em 0em;">
<div style="display: flex;">
<img src="img/bindless.jpg" width="40%" style="margin: 1em; filter: drop-shadow(rgb(22, 19, 46) 1px 1px 10px);">
<div style="margin: 1em; color: white; width: 60%"> 
<h4 class="h3">Bindless Rendering</h2>
<p>LiteFX supports <span style="font-weight: bold;">descriptor indexing</span> in unbounded descriptor arrays, which enables bind-less rendering techniques. Using those, it is straightforward to put object or material information in a common buffer and pass the non-uniform index as part of a push constant, instance index or any other means to the GPU. This makes managing large dynamic scenes accessible. LiteFX takes care of managing the descriptors, so that you can work with the same interface on both sides: in you shaders as well as your application.</p>
</div>
</div>
</div>

<div style="margin: 2em 0em;">
<div style="display: flex;">
<div style="margin: 1em; color: white; width: 60%"> 
<h4 class="h3">Hardware Raytracing</h2>
<p>The engine provides an abstraction layer over hardware-accelerated ray-tracing workflows, including traditional <span style="font-weight: bold;">ray-tracing pipelines</span>, <span style="font-weight: bold;">inline ray-tracing and ray queries</span> as well as for managing and compacting <span style="font-weight: bold;">acceleration structures</span>. As with the other features, LiteFX is agnostic with regards to the graphics API you want to use. Setup rendering once and switch between backends at run-time.</p>
</div>
<img src="img/raytracing.jpg" width="40%" style="margin: 1em; filter: drop-shadow(rgb(22, 19, 46) 1px 1px 10px); margin-left: auto;">
</div>
</div>

<div style="margin: 2em 0em;">
<div style="display: flex;">
<img src="img/indirect.jpg" width="40%" style="margin: 1em; filter: drop-shadow(rgb(22, 19, 46) 1px 1px 10px);">
<div style="margin: 1em; color: white; width: 60%"> 
<h4 class="h3">Indirect Rendering</h2>
<p>Some workloads differ vastly between different graphics APIs. One such example is <span style="font-weight: bold;">indirect rendering</span>. LiteFX provides a comprehensive abstraction for this, using shader model 6.8 to resolve fundamental architectural differences. As with the other features, this makes it easy to write portable applications that can execute on different graphics backends based on the same compatible code base.</p>
</div>
</div>
</div>

{{% /blocks/section %}}

{{% blocks/section %}}

... and there's more! Check out the samples available on [GitHub](https://github.com/crud89/LiteFX/tree/main/src/Samples/)!

{{% /blocks/section %}}