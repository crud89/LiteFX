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