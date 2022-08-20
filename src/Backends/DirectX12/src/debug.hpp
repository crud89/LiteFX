#pragma once

#include <litefx/string.hpp>
#include <litefx/math.hpp>
#include <d3d12.h>
#include <fmt/format.h>
#include <array>
#include <comdef.h>
#include <wrl.h>

using namespace Microsoft::WRL;

#ifndef NDEBUG

// Definitions as defined in WinPixEventRuntime
#define D3D12_EVENT_METADATA               0x02

#define EVENT_TIMESTAMP_WRITE_MASK         0x00000FFFFFFFFFFF
#define EVENT_TYPE_WRITE_MASK              0x00000000000003FF
#define EVENT_TIMESTAMP_BIT_SHIFT          20
#define EVENT_TYPE_BIT_SHIFT               10
#define EVENT_TYPE_END_EVENT               0x00
#define EVENT_TYPE_BEGIN_EVENT_VARARGS     0x01
#define EVENT_TYPE_BEGIN_EVENT_NOARGS      0x02

consteval UInt64 EncodeEventInfo(UInt64 timestamp, UInt64 eventType)
{
    return ((timestamp & EVENT_TIMESTAMP_WRITE_MASK) << EVENT_TIMESTAMP_BIT_SHIFT) | ((eventType & EVENT_TYPE_WRITE_MASK) << EVENT_TYPE_BIT_SHIFT);
}

#define EVENT_STRING_ALIGNMENT_WRITE_MASK  0x000000000000000F
#define EVENT_STRING_ALIGNMENT_BIT_SHIFT   60
#define EVENT_STRING_COPY_CHUNK_SIZE_WRITE_MASK 0x000000000000001F
#define EVENT_STRING_COPY_CHUNK_SIZE_BIT_SHIFT 55
#define EVENT_STRING_IS_ANSI_WRITE_MASK    0x0000000000000001
#define EVENT_STRING_IS_ANSI_BIT_SHIFT     54
#define EVENT_STRING_IS_SHORTCUT_WRITE_MASK 0x0000000000000001
#define EVENT_STRING_IS_SHORTCUT_BIT_SHIFT 53

consteval UInt64 EncodeStringInfo(UInt64 alignment, UInt64 copyChunkSize, BOOL isANSI, BOOL isShortcut)
{
    return ((alignment & EVENT_STRING_ALIGNMENT_WRITE_MASK) << EVENT_STRING_ALIGNMENT_BIT_SHIFT) | ((copyChunkSize & EVENT_STRING_COPY_CHUNK_SIZE_WRITE_MASK) << EVENT_STRING_COPY_CHUNK_SIZE_BIT_SHIFT) | 
        ((static_cast<UInt64>(isANSI) & EVENT_STRING_IS_ANSI_WRITE_MASK) << EVENT_STRING_IS_ANSI_BIT_SHIFT) | ((static_cast<UInt64>(isShortcut) & EVENT_STRING_IS_SHORTCUT_WRITE_MASK) << EVENT_STRING_IS_SHORTCUT_BIT_SHIFT);
}

static void CopyEventName(const String str, UInt64*& buffer, UInt64* end)
{
    *buffer++ = ::EncodeStringInfo(0, 8, FALSE, FALSE);
    auto argument = str.c_str();

    while (buffer < end)
    {
        UInt64 c = static_cast<UInt16>(argument[0]);
        if (!c)
        {
            *buffer++ = 0;
            return;
        }

        UInt64 x = c;
        c = static_cast<UInt16>(argument[1]);

        if (!c)
        {
            *buffer++ = x;
            return;
        }

        x |= c << 16;
        c = static_cast<UInt16>(argument[2]);

        if (!c)
        {
            *buffer++ = x;
            return;
        }

        x |= c << 32;
        c = static_cast<UInt16>(argument[3]);

        if (!c)
        {
            *buffer++ = x;
            return;
        }

        x |= c << 48;
        *buffer++ = x;
        argument += 4;
    }
}

template <typename... TArgs>
static void inline BeginEvent(ID3D12CommandQueue* queue, UInt64 color, StringView format, TArgs... args)
{
    std::array<UInt64, 64> buffer {};
    buffer[0] = ::EncodeEventInfo(0, EVENT_TYPE_BEGIN_EVENT_NOARGS);
    buffer[1] = color;
    UInt64* destination = &buffer[2];
    UInt64* limit = destination + 62;

    // Format the string.
    String str = fmt::format(fmt::runtime(format), std::forward<TArgs>(args)...);
    ::CopyEventName(str, destination, limit);

    // Issue the event.
    queue->BeginEvent(D3D12_EVENT_METADATA, static_cast<void*>(buffer.data()), static_cast<UInt32>(reinterpret_cast<Byte*>(destination) - reinterpret_cast<Byte*>(buffer.data())));
}

template <typename... TArgs>
static void inline BeginEvent(const ComPtr<ID3D12CommandQueue>& queue, UInt64 color, StringView format, TArgs... args)
{
    ::BeginEvent(queue.Get(), color, format, args...);
}

template <typename... TArgs>
static void inline BeginEvent(ID3D12CommandQueue* queue, StringView format, TArgs... args)
{
    ::BeginEvent(queue, 0xDBDBDBFF, format, args...);
}

template <typename... TArgs>
static void inline BeginEvent(const ComPtr<ID3D12CommandQueue>& queue, StringView format, TArgs... args)
{
    ::BeginEvent(queue.Get(), 0xDBDBDBFF, format, args...);
}

static void inline EndEvent(ID3D12CommandQueue* queue)
{
    queue->EndEvent();
}

static void inline EndEvent(const ComPtr<ID3D12CommandQueue>& queue)
{
    queue->EndEvent();
}

#undef D3D12_EVENT_METADATA
#undef EVENT_TIMESTAMP_WRITE_MASK
#undef EVENT_TYPE_WRITE_MASK
#undef EVENT_TIMESTAMP_BIT_SHIFT
#undef EVENT_TYPE_WRITE_BIT_SHIFT
#undef EVENT_TYPE_END_EVENT
#undef EVENT_TYPE_BEGIN_EVENT_VARARGS
#undef EVENT_TYPE_BEGIN_EVENT_NOARGS
#undef EVENT_STRING_ALIGNMENT_WRITE_MASK
#undef EVENT_STRING_ALIGNMENT_BIT_SHIFT
#undef EVENT_STRING_COPY_CHUNK_SIZE_WRITE_MASK
#undef EVENT_STRING_COPY_CHUNK_SIZE_BIT_SHIFT
#undef EVENT_STRING_IS_ANSI_WRITE_MASK
#undef EVENT_STRING_IS_ANSI_BIT_SHIFT
#undef EVENT_STRING_IS_SHORTCUT_WRITE_MASK
#undef EVENT_STRING_IS_SHORTCUT_BIT_SHIFT

#else

template <typename... TArgs>
static void inline BeginEvent(ID3D12CommandQueue* /*queue*/, UInt64 /*color*/, StringView /*format*/, TArgs... /*args*/)
{
}

template <typename... TArgs>
static void inline BeginEvent(const ComPtr<ID3D12CommandQueue>& /*queue*/, UInt64 /*color*/, StringView /*format*/, TArgs... /*args*/)
{
}

template <typename... TArgs>
static void inline BeginEvent(ID3D12CommandQueue* /*queue*/, StringView /*format*/, TArgs... /*args*/)
{
}

template <typename... TArgs>
static void inline BeginEvent(const ComPtr<ID3D12CommandQueue>& /*queue*/, StringView format, TArgs... /*args*/)
{
}

static void inline EndEvent(ID3D12CommandQueue* /*queue*/)
{
}

static void inline EndEvent(const ComPtr<ID3D12CommandQueue>& /*queue*/)
{
}

#endif