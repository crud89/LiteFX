#pragma once

#include <litefx/graphics_api.hpp>
#include <litefx/graphics_formatters.hpp>
#include <litefx/math.hpp>

namespace LiteFX::Graphics {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    struct LITEFX_GRAPHICS_API Vertex {
    public:
        Vector3f Position;
        Vector4f Color;
        Vector2f TextureCoordinate0;
    };

}