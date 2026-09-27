#pragma once

#include <litefx/graphics_api.hpp>
#include <litefx/math.hpp>

namespace LiteFX::Graphics {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// @brief Default definition for a simple vertex.
    struct Vertex {
    public:
        /// @brief The position of the vertex.
        Vector3f Position;

        /// @brief The color of the vertex.
        Vector4f Color;

        /// @brief The normal vector of the vertex.
        Vector3f Normal;

        /// @brief The texture coordinate of the vertex.
        Vector2f TextureCoordinate0;
    };

}