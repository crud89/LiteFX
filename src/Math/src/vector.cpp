#include <litefx/math.hpp>

using namespace LiteFX::Math;

// NOLINTBEGIN(performance-move-const-arg)

// ------------------------------------------------------------------------------------------------
// Vector1f.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector1f::Vector1f(const glm::f32vec1& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector1f::Vector1f(glm::f32vec1&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector1f::operator glm::f32vec1() const noexcept {
    return glm::f32vec1(m_elements[0]);
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector1f::Vector1f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMStoreFloat(m_elements.data(), v);
}

Vector1f::Vector1f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMStoreFloat(m_elements.data(), std::move(v));
}

Vector1f::operator DirectX::XMVECTOR() const noexcept {
    return DirectX::XMLoadFloat(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector1u.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector1u::Vector1u(const glm::u32vec1& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector1u::Vector1u(glm::u32vec1&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector1u::operator glm::u32vec1() const noexcept {
    return glm::u32vec1(m_elements[0]);
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector1u::Vector1u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMStoreInt(m_elements.data(), v);
}

Vector1u::Vector1u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMStoreInt(m_elements.data(), std::move(v));
}

Vector1u::operator DirectX::XMVECTOR() const noexcept {
    return DirectX::XMLoadInt(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2f.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector2f::Vector2f(const glm::f32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2f::Vector2f(glm::f32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2f::operator glm::f32vec2() const noexcept {
    return { m_elements[0], m_elements[1] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector2f::Vector2f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT2 e{ };
    DirectX::XMStoreFloat2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2f::Vector2f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT2 e{ };
    DirectX::XMStoreFloat2(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
}

Vector2f::Vector2f(const DirectX::XMFLOAT2& v) noexcept : Vector<Float, 2>() {
    this->x() = v.x;
    this->y() = v.y;
}

Vector2f::Vector2f(DirectX::XMFLOAT2&& v) noexcept : Vector<Float, 2>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
}

Vector2f::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMFLOAT2();
    return DirectX::XMLoadFloat2(&buffer);
}

Vector2f::operator DirectX::XMFLOAT2() const noexcept {
    return DirectX::XMFLOAT2(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2u.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector2u::Vector2u(const glm::u32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2u::Vector2u(glm::u32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2u::operator glm::u32vec2() const noexcept {
    return { m_elements[0], m_elements[1] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector2u::Vector2u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT2 e{ };
    DirectX::XMStoreUInt2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2u::Vector2u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT2 e{ };
    DirectX::XMStoreUInt2(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
}

Vector2u::Vector2u(const DirectX::XMUINT2& v) noexcept : Vector<UInt32, 2>() {
    this->x() = v.x;
    this->y() = v.y;
}

Vector2u::Vector2u(DirectX::XMUINT2&& v) noexcept : Vector<UInt32, 2>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
}

Vector2u::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMUINT2();
    return DirectX::XMLoadUInt2(&buffer);
}

Vector2u::operator DirectX::XMUINT2() const noexcept {
    return DirectX::XMUINT2(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2i.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector2i::Vector2i(const glm::i32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2i::Vector2i(glm::i32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2i::operator glm::i32vec2() const noexcept {
    return { m_elements[0], m_elements[1] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector2i::Vector2i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT2 e{ };
    DirectX::XMStoreSInt2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2i::Vector2i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT2 e{ };
    DirectX::XMStoreSInt2(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
}

Vector2i::Vector2i(const DirectX::XMINT2& v) noexcept : Vector<Int32, 2>() {
    this->x() = v.x;
    this->y() = v.y;
}

Vector2i::Vector2i(DirectX::XMINT2&& v) noexcept : Vector<Int32, 2>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
}

Vector2i::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMINT2();
    return DirectX::XMLoadSInt2(&buffer);
}

Vector2i::operator DirectX::XMINT2() const noexcept {
    return DirectX::XMINT2(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3f.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector3f::Vector3f(const glm::f32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3f::Vector3f(glm::f32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3f::operator glm::f32vec3() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector3f::Vector3f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT3 e{ };
    DirectX::XMStoreFloat3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3f::Vector3f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT3 e{ };
    DirectX::XMStoreFloat3(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3f::Vector3f(const DirectX::XMFLOAT3& v) noexcept : Vector<Float, 3>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
}

Vector3f::Vector3f(DirectX::XMFLOAT3&& v) noexcept : Vector<Float, 3>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
}

Vector3f::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMFLOAT3();
    return DirectX::XMLoadFloat3(&buffer);
}

Vector3f::operator DirectX::XMFLOAT3() const noexcept {
    return DirectX::XMFLOAT3(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3u.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector3u::Vector3u(const glm::u32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3u::Vector3u(glm::u32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3u::operator glm::u32vec3() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector3u::Vector3u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT3 e{ };
    DirectX::XMStoreUInt3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3u::Vector3u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT3 e{ };
    DirectX::XMStoreUInt3(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3u::Vector3u(const DirectX::XMUINT3& v) noexcept : Vector<UInt32, 3>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
}

Vector3u::Vector3u(DirectX::XMUINT3&& v) noexcept : Vector<UInt32, 3>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
}

Vector3u::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMUINT3();
    return DirectX::XMLoadUInt3(&buffer);
}

Vector3u::operator DirectX::XMUINT3() const noexcept {
    return DirectX::XMUINT3(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3i.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector3i::Vector3i(const glm::i32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3i::Vector3i(glm::i32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3i::operator glm::i32vec3() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector3i::Vector3i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT3 e{ };
    DirectX::XMStoreSInt3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3i::Vector3i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT3 e{ };
    DirectX::XMStoreSInt3(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3i::Vector3i(const DirectX::XMINT3& v) noexcept : Vector<Int32, 3>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
}

Vector3i::Vector3i(DirectX::XMINT3&& v) noexcept : Vector<Int32, 3>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
}

Vector3i::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMINT3();
    return DirectX::XMLoadSInt3(&buffer);
}

Vector3i::operator DirectX::XMINT3() const noexcept {
    return DirectX::XMINT3(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4f.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector4f::Vector4f(const glm::f32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4f::Vector4f(glm::f32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4f::operator glm::f32vec4() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2], m_elements[3] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector4f::Vector4f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT4 e{ };
    DirectX::XMStoreFloat4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4f::Vector4f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT4 e{ };
    DirectX::XMStoreFloat4(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4f::Vector4f(const DirectX::XMFLOAT4& v) noexcept : Vector<Float, 4>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
    this->w() = v.w;
}

Vector4f::Vector4f(DirectX::XMFLOAT4&& v) noexcept : Vector<Float, 4>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
    this->w() = std::move(v.w);
}

Vector4f::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMFLOAT4();
    return DirectX::XMLoadFloat4(&buffer);
}

Vector4f::operator DirectX::XMFLOAT4() const noexcept {
    return DirectX::XMFLOAT4(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4u.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector4u::Vector4u(const glm::u32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4u::Vector4u(glm::u32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4u::operator glm::u32vec4() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2], m_elements[3] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector4u::Vector4u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT4 e{ };
    DirectX::XMStoreUInt4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4u::Vector4u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT4 e{ };
    DirectX::XMStoreUInt4(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4u::Vector4u(const DirectX::XMUINT4& v) noexcept : Vector<UInt32, 4>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
    this->w() = v.w;
}

Vector4u::Vector4u(DirectX::XMUINT4&& v) noexcept : Vector<UInt32, 4>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
    this->w() = std::move(v.w);
}

Vector4u::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMUINT4();
    return DirectX::XMLoadUInt4(&buffer);
}

Vector4u::operator DirectX::XMUINT4() const noexcept {
    return DirectX::XMUINT4(m_elements.data());
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4i.
// ------------------------------------------------------------------------------------------------

#if defined(LITEFX_BUILD_WITH_GLM)
Vector4i::Vector4i(const glm::i32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4i::Vector4i(glm::i32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4i::operator glm::i32vec4() const noexcept {
    return { m_elements[0], m_elements[1], m_elements[2], m_elements[3] };
}
#endif

#if defined(LITEFX_BUILD_WITH_DIRECTX_MATH)
Vector4i::Vector4i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT4 e{ };
    DirectX::XMStoreSInt4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4i::Vector4i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT4 e{ };
    DirectX::XMStoreSInt4(&e, std::move(v));

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4i::Vector4i(const DirectX::XMINT4& v) noexcept : Vector<Int32, 4>() {
    this->x() = v.x;
    this->y() = v.y;
    this->z() = v.z;
    this->w() = v.w;
}

Vector4i::Vector4i(DirectX::XMINT4&& v) noexcept : Vector<Int32, 4>() {
    this->x() = std::move(v.x);
    this->y() = std::move(v.y);
    this->z() = std::move(v.z);
    this->w() = std::move(v.w);
}

Vector4i::operator DirectX::XMVECTOR() const noexcept {
    auto buffer = this->operator DirectX::XMINT4();
    return DirectX::XMLoadSInt4(&buffer);
}

Vector4i::operator DirectX::XMINT4() const noexcept {
    return DirectX::XMINT4(m_elements.data());
}
#endif

// NOLINTEND(performance-move-const-arg)