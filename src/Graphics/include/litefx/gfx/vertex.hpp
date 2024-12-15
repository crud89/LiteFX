#pragma once

#include <litefx/graphics_api.hpp>
#include <litefx/math.hpp>

namespace LiteFX::Graphics {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// <summary>
    /// Default definition for a simple vertex.
    /// </summary>
    struct Vertex {
    public:
        /// <summary>
        /// The position of the vertex.
        /// </summary>
        Vector3f Position;

        /// <summary>
        /// The color of the vertex.
        /// </summary>
        Vector4f Color;

        /// <summary>
        /// The normal vector of the vertex.
        /// </summary>
        Vector3f Normal;

        /// <summary>
        /// The texture coordinate of the vertex.
        /// </summary>
        Vector2f TextureCoordinate0;
    };

}