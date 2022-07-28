---
title: "Render Backends"
weight: 3
---

The core of LiteFX is a flexible computer graphics engine with support for modern graphics APIs. It is implemented for two backends: DirectX 12 ❎ and Vulkan 🌋. Though currently not officially implemented, it is also possible to write custom backends for older graphics APIs and still benefit from a possible performance improvements, since the abstraction layer follows principles that are also used by [*Approaching Zero Driver Overhead (AZDO)*](https://www.gdcvault.com/play/1020791).