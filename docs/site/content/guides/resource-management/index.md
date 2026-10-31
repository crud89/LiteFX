# Resource management

Buffers, textures and samplers hold the data your shaders work with. This guide covers their life from creation to the point where a shader
accesses them:

<div class="grid cards" markdown>

-   :material-database-outline:{ .lg .middle } __Buffers, textures and samplers__

    ---

    Create resources, choose where their memory lives, and get data into and out of them.

    [:octicons-arrow-right-24: Read the guide](buffers-textures-samplers.md)

-   :material-link-variant:{ .lg .middle } __Descriptor bindings__

    ---

    Make resources available to shaders: describing bindings, distributing buffer elements across descriptors, and the rules for each kind of
    descriptor.

    [:octicons-arrow-right-24: Read the guide](descriptor-bindings.md)

</div>

How to order accesses to resources on the GPU, for example between an upload and a draw call, is covered by the [synchronization
guide](../synchronization/index.md).
