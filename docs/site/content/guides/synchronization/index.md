# Synchronization

The GPU does not execute commands when you record them. You record commands into *command buffers*, submit those to *command queues*, and
the GPU works through them later, in parallel to your application and to other queues. Whenever two pieces of work touch the same resource,
or the CPU needs the result of GPU work, you have to tell the engine how they are ordered. This guide explains the tools LiteFX gives you for
that, and the rules both backends follow.

You have already used most of them in the [first triangle](../../tutorials/first-triangle/index.md) tutorial: the upload waited for a *fence*, and the
render pass took care of everything else. This guide explains what happened there, and what you need to do once you leave the beaten path,
for example when you add compute work, copy images or use more than one queue.

## Three levels of synchronization

Synchronization in LiteFX happens on three levels. Each one has its own tool:

| Level | Question it answers | Tool |
|---|---|---|
| CPU and GPU | Has the GPU finished the work I submitted, so I can read results or reuse memory? | A *fence* and `ICommandQueue::waitFor(fence)` |
| Queue and queue | Does this queue have to wait for work on another queue? | A *fence* and `ICommandQueue::waitFor(queue, fence)` |
| Within a submission | Does this command have to wait for a previous one, and in which state does it need a resource? | A *barrier* |

The levels build on each other. A fence orders whole submissions, but says nothing about the commands inside them. A barrier orders
commands inside a submission, but cannot wait for another queue or for the CPU. Between submissions to the *same* queue, there is nothing
to do: a queue executes its submissions one after another, so every submission starts with all previous work on that queue completed.

!!! note "Some synchronization is done for you"
    Render passes, the swap chain, mip map generation and defragmentation synchronize their own work. They do, however, expect resources in a
    well-defined state when you hand them over, and they return them in a well-defined state. These contracts are described on the pages of
    this guide, and summarized in [the default state](barriers.md#the-default-state).

## Pages in this guide

| # | Page | Topic |
|:-:|---|---|
| 1 | [Queues and fences](queues-and-fences.md) | Queue types, submitting work, waiting on the CPU and on other queues |
| 2 | [Command buffers](command-buffers.md) | Recording, submitting and reusing command buffers, secondary command buffers, resource lifetime |
| 3 | [Barriers](barriers.md) | Pipeline stages, resource access, image layouts and the default state |
| 4 | [Render passes and frames](render-passes-and-frames.md) | What render passes and the swap chain synchronize, and frames in flight |
| 5 | [Patterns and pitfalls](patterns.md) | Recipes for common situations, typical mistakes and how to debug them |

Read the pages in order the first time. Afterwards, the [patterns](patterns.md) page works as a reference.

## Terms used in this guide

Fence
:   A 64-bit value on a queue's timeline. Every submission advances it, and the queue *passes* a fence value once all work submitted before
    it has finished on the GPU.

Submission
:   One or more command buffers handed to a queue in a single `submit` call. A submission is the unit a fence refers to. Render passes
    submit their own command buffers when they end, so each render pass is a submission of its own.

Barrier
:   A command that makes later commands in the same submission wait for earlier ones, makes memory written by the earlier commands visible to the
    later ones, and changes the layout of images.

Layout
:   The state an image is stored in. Some operations require an image in a specific layout. Buffers have no layout.

Default state
:   The state every engine-managed operation expects resources in, and leaves them in: images in `ImageLayout::Common`.

## Backend differences

LiteFX maps all synchronization primitives to their native counterparts: fences are *timeline semaphores* in Vulkan and `ID3D12Fence`
objects in DirectX 12, and barriers are *synchronization2* barriers in Vulkan and *enhanced barriers* in DirectX 12. The engine requires the
`VK_KHR_unified_image_layouts` extension in Vulkan, which allows it to keep all images in the `GENERAL` layout. As a consequence, image
layouts only have an effect in DirectX 12, while pipeline stages and resource access matter in both backends.

Both backends also execute the submissions of a queue strictly one after another. DirectX 12 guarantees this for each
`ExecuteCommandLists` call. Vulkan does not, so LiteFX lets each submission wait for the previous one on the same queue. This makes the
synchronization rules in this guide identical for both backends.

!!! warning "Test your synchronization on both backends"
    Because Vulkan ignores image layouts, a missing layout transition goes unnoticed there and only fails in DirectX 12. Run your application
    with the Vulkan validation layers *and* the DirectX 12 debug layer enabled while you develop it.
