#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Vector1f.
// ------------------------------------------------------------------------------------------------

Vector1f::Vector1f() noexcept : Vector<Float, 1>() { }
Vector1f::Vector1f(const Float& v) noexcept : Vector<Float, 1>(v) { }
Vector1f::Vector1f(const Vector1f& _v) noexcept : Vector<Float, 1>(_v) { }
Vector1f::Vector1f(const Vector<Float, 1>& _v) noexcept : Vector<Float, 1>(_v) { }
Vector1f::Vector1f(Vector1f&& _v) noexcept : Vector<Float, 1>(_v) { }
Vector1f::Vector1f(Vector<Float, 1>&& _v) noexcept : Vector<Float, 1>(_v) { }

Vector1f& Vector1f::operator=(const Vector<Float, 1>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector1f& Vector1f::operator=(Vector<Float, 1>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector1f& Vector1f::operator=(const Array<Float>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector1f& Vector1f::operator=(const Vector1f& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector1f& Vector1f::operator=(Vector1f&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Float& Vector1f::operator[](const unsigned int& i) const noexcept {
    return Vector<Float, 1>::operator[](i);
}

Float& Vector1f::operator[](const unsigned int& i) noexcept {
    return Vector<Float, 1>::operator[](i);
}

Vector1f::operator Array<Float>() noexcept {
    Array<Float> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
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

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector1f::Vector1f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMStoreFloat(m_elements, v);
}

Vector1f::Vector1f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMStoreFloat(m_elements, std::move(v));
}

Vector1f::operator DirectX::XMVECTOR() const noexcept {
    return DirectX::XMLoadFloat(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector1u.
// ------------------------------------------------------------------------------------------------

Vector1u::Vector1u() noexcept : Vector<UInt32, 1>() { }
Vector1u::Vector1u(const UInt32& v) noexcept : Vector<UInt32, 1>(v) { }
Vector1u::Vector1u(const Vector1u& _v) noexcept : Vector<UInt32, 1>(_v) { }
Vector1u::Vector1u(const Vector<UInt32, 1>& _v) noexcept : Vector<UInt32, 1>(_v) { }
Vector1u::Vector1u(Vector1u&& _v) noexcept : Vector<UInt32, 1>(_v) { }
Vector1u::Vector1u(Vector<UInt32, 1>&& _v) noexcept : Vector<UInt32, 1>(_v) { }

Vector1u& Vector1u::operator=(const Vector<UInt32, 1>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector1u& Vector1u::operator=(Vector<UInt32, 1>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector1u& Vector1u::operator=(const Array<UInt32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector1u& Vector1u::operator=(const Vector1u& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector1u& Vector1u::operator=(Vector1u&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const UInt32& Vector1u::operator[](const unsigned int& i) const noexcept {
    return Vector<UInt32, 1>::operator[](i);
}

UInt32& Vector1u::operator[](const unsigned int& i) noexcept {
    return Vector<UInt32, 1>::operator[](i);
}

Vector1u::operator Array<UInt32>() noexcept {
    Array<UInt32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
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

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector1u::Vector1u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMStoreInt(m_elements, v);
}

Vector1u::Vector1u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMStoreInt(m_elements, std::move(v));
}

Vector1u::operator DirectX::XMVECTOR() const noexcept {
    return DirectX::XMLoadInt(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2f.
// ------------------------------------------------------------------------------------------------

Vector2f::Vector2f() noexcept : Vector<Float, 2>() { }
Vector2f::Vector2f(const Float& v) noexcept : Vector<Float, 2>(v) { }
Vector2f::Vector2f(const Float& x, const Float& y) noexcept : Vector<Float, 2>() { 
    this->x() = x;
    this->y() = y;
}

Vector2f::Vector2f(const Vector2f& _v) noexcept : Vector<Float, 2>(_v) { }
Vector2f::Vector2f(const Vector<Float, 2>& _v) noexcept : Vector<Float, 2>(_v) { }
Vector2f::Vector2f(Vector2f&& _v) noexcept : Vector<Float, 2>(_v) { }
Vector2f::Vector2f(Vector<Float, 2>&& _v) noexcept : Vector<Float, 2>(_v) { }

Vector2f& Vector2f::operator=(const Vector<Float, 2>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2f& Vector2f::operator=(Vector<Float, 2>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector2f& Vector2f::operator=(const Array<Float>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2f& Vector2f::operator=(const Vector2f& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector2f& Vector2f::operator=(Vector2f&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Float& Vector2f::operator[](const unsigned int& i) const noexcept {
    return Vector<Float, 2>::operator[](i);
}

Float& Vector2f::operator[](const unsigned int& i) noexcept {
    return Vector<Float, 2>::operator[](i);
}

Vector2f::operator Array<Float>() noexcept {
    Array<Float> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector2f::Vector2f(const glm::f32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2f::Vector2f(glm::f32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2f::operator glm::f32vec2() const noexcept {
    return glm::f32vec2(m_elements[0], m_elements[1]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector2f::Vector2f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT2 e;
    DirectX::XMStoreFloat2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2f::Vector2f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT2 e;
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
    return DirectX::XMFLOAT2(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2u.
// ------------------------------------------------------------------------------------------------

Vector2u::Vector2u() noexcept : Vector<UInt32, 2>() { }
Vector2u::Vector2u(const UInt32& v) noexcept : Vector<UInt32, 2>(v) { }
Vector2u::Vector2u(const UInt32& x, const UInt32& y) noexcept : Vector<UInt32, 2>() {
    this->x() = x;
    this->y() = y;
}

Vector2u::Vector2u(const Vector2u& _v) noexcept : Vector<UInt32, 2>(_v) { }
Vector2u::Vector2u(const Vector<UInt32, 2>& _v) noexcept : Vector<UInt32, 2>(_v) { }
Vector2u::Vector2u(Vector2u&& _v) noexcept : Vector<UInt32, 2>(_v) { }
Vector2u::Vector2u(Vector<UInt32, 2>&& _v) noexcept : Vector<UInt32, 2>(_v) { }

Vector2u& Vector2u::operator=(const Vector<UInt32, 2>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2u& Vector2u::operator=(Vector<UInt32, 2>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector2u& Vector2u::operator=(const Array<UInt32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2u& Vector2u::operator=(const Vector2u& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector2u& Vector2u::operator=(Vector2u&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const UInt32& Vector2u::operator[](const unsigned int& i) const noexcept {
    return Vector<UInt32, 2>::operator[](i);
}

UInt32& Vector2u::operator[](const unsigned int& i) noexcept {
    return Vector<UInt32, 2>::operator[](i);
}

Vector2u::operator Array<UInt32>() noexcept {
    Array<UInt32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector2u::Vector2u(const glm::u32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2u::Vector2u(glm::u32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2u::operator glm::u32vec2() const noexcept {
    return glm::u32vec2(m_elements[0], m_elements[1]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector2u::Vector2u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT2 e;
    DirectX::XMStoreUInt2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2u::Vector2u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT2 e;
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
    return DirectX::XMUINT2(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector2i.
// ------------------------------------------------------------------------------------------------

Vector2i::Vector2i() noexcept : Vector<Int32, 2>() { }
Vector2i::Vector2i(const Int32& v) noexcept : Vector<Int32, 2>(v) { }
Vector2i::Vector2i(const Int32& x, const Int32& y) noexcept : Vector<Int32, 2>() {
    this->x() = x;
    this->y() = y;
}

Vector2i::Vector2i(const Vector2i& _v) noexcept : Vector<Int32, 2>(_v) { }
Vector2i::Vector2i(const Vector<Int32, 2>& _v) noexcept : Vector<Int32, 2>(_v) { }
Vector2i::Vector2i(Vector2i&& _v) noexcept : Vector<Int32, 2>(_v) { }
Vector2i::Vector2i(Vector<Int32, 2>&& _v) noexcept : Vector<Int32, 2>(_v) { }

Vector2i& Vector2i::operator=(const Vector<Int32, 2>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2i& Vector2i::operator=(Vector<Int32, 2>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector2i& Vector2i::operator=(const Array<Int32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector2i& Vector2i::operator=(const Vector2i& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector2i& Vector2i::operator=(Vector2i&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Int32& Vector2i::operator[](const unsigned int& i) const noexcept {
    return Vector<Int32, 2>::operator[](i);
}

Int32& Vector2i::operator[](const unsigned int& i) noexcept {
    return Vector<Int32, 2>::operator[](i);
}

Vector2i::operator Array<Int32>() noexcept {
    Array<Int32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector2i::Vector2i(const glm::i32vec2& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector2i::Vector2i(glm::i32vec2&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector2i::operator glm::i32vec2() const noexcept {
    return glm::i32vec2(m_elements[0], m_elements[1]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector2i::Vector2i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT2 e;
    DirectX::XMStoreSInt2(&e, v);

    this->x() = e.x;
    this->y() = e.y;
}

Vector2i::Vector2i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT2 e;
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
    return DirectX::XMINT2(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3f.
// ------------------------------------------------------------------------------------------------

Vector3f::Vector3f() noexcept : Vector<Float, 3>() { }
Vector3f::Vector3f(const Float& v) noexcept : Vector<Float, 3>(v) { }
Vector3f::Vector3f(const Float& x, const Float& y, const Float& z) noexcept : Vector<Float, 3>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
}

Vector3f::Vector3f(const Vector3f& _v) noexcept : Vector<Float, 3>(_v) { }
Vector3f::Vector3f(const Vector<Float, 3>& _v) noexcept : Vector<Float, 3>(_v) { }
Vector3f::Vector3f(Vector3f&& _v) noexcept : Vector<Float, 3>(_v) { }
Vector3f::Vector3f(Vector<Float, 3>&& _v) noexcept : Vector<Float, 3>(_v) { }

Vector3f& Vector3f::operator=(const Vector<Float, 3>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3f& Vector3f::operator=(Vector<Float, 3>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector3f& Vector3f::operator=(const Array<Float>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3f& Vector3f::operator=(const Vector3f& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector3f& Vector3f::operator=(Vector3f&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Float& Vector3f::operator[](const unsigned int& i) const noexcept {
    return Vector<Float, 3>::operator[](i);
}

Float& Vector3f::operator[](const unsigned int& i) noexcept {
    return Vector<Float, 3>::operator[](i);
}

Vector3f::operator Array<Float>() noexcept {
    Array<Float> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector3f::Vector3f(const glm::f32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3f::Vector3f(glm::f32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3f::operator glm::f32vec3() const noexcept {
    return glm::f32vec3(m_elements[0], m_elements[1], m_elements[2]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector3f::Vector3f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT3 e;
    DirectX::XMStoreFloat3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3f::Vector3f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT3 e;
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
    return DirectX::XMFLOAT3(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3u.
// ------------------------------------------------------------------------------------------------

Vector3u::Vector3u() noexcept : Vector<UInt32, 3>() { }
Vector3u::Vector3u(const UInt32& v) noexcept : Vector<UInt32, 3>(v) { }
Vector3u::Vector3u(const UInt32& x, const UInt32& y, const UInt32& z) noexcept : Vector<UInt32, 3>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
}

Vector3u::Vector3u(const Vector3u& _v) noexcept : Vector<UInt32, 3>(_v) { }
Vector3u::Vector3u(const Vector<UInt32, 3>& _v) noexcept : Vector<UInt32, 3>(_v) { }
Vector3u::Vector3u(Vector3u&& _v) noexcept : Vector<UInt32, 3>(_v) { }
Vector3u::Vector3u(Vector<UInt32, 3>&& _v) noexcept : Vector<UInt32, 3>(_v) { }

Vector3u& Vector3u::operator=(const Vector<UInt32, 3>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3u& Vector3u::operator=(Vector<UInt32, 3>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector3u& Vector3u::operator=(const Array<UInt32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3u& Vector3u::operator=(const Vector3u& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector3u& Vector3u::operator=(Vector3u&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const UInt32& Vector3u::operator[](const unsigned int& i) const noexcept {
    return Vector<UInt32, 3>::operator[](i);
}

UInt32& Vector3u::operator[](const unsigned int& i) noexcept {
    return Vector<UInt32, 3>::operator[](i);
}

Vector3u::operator Array<UInt32>() noexcept {
    Array<UInt32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector3u::Vector3u(const glm::u32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3u::Vector3u(glm::u32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3u::operator glm::u32vec3() const noexcept {
    return glm::u32vec3(m_elements[0], m_elements[1], m_elements[2]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector3u::Vector3u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT3 e;
    DirectX::XMStoreUInt3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3u::Vector3u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT3 e;
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
    return DirectX::XMUINT3(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector3i.
// ------------------------------------------------------------------------------------------------

Vector3i::Vector3i() noexcept : Vector<Int32, 3>() { }
Vector3i::Vector3i(const Int32& v) noexcept : Vector<Int32, 3>(v) { }
Vector3i::Vector3i(const Int32& x, const Int32& y, const Int32& z) noexcept : Vector<Int32, 3>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
}

Vector3i::Vector3i(const Vector3i& _v) noexcept : Vector<Int32, 3>(_v) { }
Vector3i::Vector3i(const Vector<Int32, 3>& _v) noexcept : Vector<Int32, 3>(_v) { }
Vector3i::Vector3i(Vector3i&& _v) noexcept : Vector<Int32, 3>(_v) { }
Vector3i::Vector3i(Vector<Int32, 3>&& _v) noexcept : Vector<Int32, 3>(_v) { }

Vector3i& Vector3i::operator=(const Vector<Int32, 3>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3i& Vector3i::operator=(Vector<Int32, 3>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector3i& Vector3i::operator=(const Array<Int32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector3i& Vector3i::operator=(const Vector3i& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector3i& Vector3i::operator=(Vector3i&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Int32& Vector3i::operator[](const unsigned int& i) const noexcept {
    return Vector<Int32, 3>::operator[](i);
}

Int32& Vector3i::operator[](const unsigned int& i) noexcept {
    return Vector<Int32, 3>::operator[](i);
}

Vector3i::operator Array<Int32>() noexcept {
    Array<Int32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector3i::Vector3i(const glm::i32vec3& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector3i::Vector3i(glm::i32vec3&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector3i::operator glm::i32vec3() const noexcept {
    return glm::i32vec3(m_elements[0], m_elements[1], m_elements[2]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector3i::Vector3i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT3 e;
    DirectX::XMStoreSInt3(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
}

Vector3i::Vector3i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT3 e;
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
    return DirectX::XMINT3(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4f.
// ------------------------------------------------------------------------------------------------

Vector4f::Vector4f() noexcept : Vector<Float, 4>() { }
Vector4f::Vector4f(const Float& v) noexcept : Vector<Float, 4>(v) { }
Vector4f::Vector4f(const Float& x, const Float& y, const Float& z, const Float& w) noexcept : Vector<Float, 4>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
    this->w() = w;
}

Vector4f::Vector4f(const Vector4f& _v) noexcept : Vector<Float, 4>(_v) { }
Vector4f::Vector4f(const Vector<Float, 4>& _v) noexcept : Vector<Float, 4>(_v) { }
Vector4f::Vector4f(Vector4f&& _v) noexcept : Vector<Float, 4>(_v) { }
Vector4f::Vector4f(Vector<Float, 4>&& _v) noexcept : Vector<Float, 4>(_v) { }

Vector4f& Vector4f::operator=(const Vector<Float, 4>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4f& Vector4f::operator=(Vector<Float, 4>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector4f& Vector4f::operator=(const Array<Float>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4f& Vector4f::operator=(const Vector4f& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector4f& Vector4f::operator=(Vector4f&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Float& Vector4f::operator[](const unsigned int& i) const noexcept {
    return Vector<Float, 4>::operator[](i);
}

Float& Vector4f::operator[](const unsigned int& i) noexcept {
    return Vector<Float, 4>::operator[](i);
}

Vector4f::operator Array<Float>() noexcept {
    Array<Float> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector4f::Vector4f(const glm::f32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4f::Vector4f(glm::f32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4f::operator glm::f32vec4() const noexcept {
    return glm::f32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector4f::Vector4f(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMFLOAT4 e;
    DirectX::XMStoreFloat4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4f::Vector4f(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMFLOAT4 e;
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
    return DirectX::XMFLOAT4(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4u.
// ------------------------------------------------------------------------------------------------

Vector4u::Vector4u() noexcept : Vector<UInt32, 4>() { }
Vector4u::Vector4u(const UInt32& v) noexcept : Vector<UInt32, 4>(v) { }
Vector4u::Vector4u(const UInt32& x, const UInt32& y, const UInt32& z, const UInt32& w) noexcept : Vector<UInt32, 4>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
    this->w() = w;
}

Vector4u::Vector4u(const Vector4u& _v) noexcept : Vector<UInt32, 4>(_v) { }
Vector4u::Vector4u(const Vector<UInt32, 4>& _v) noexcept : Vector<UInt32, 4>(_v) { }
Vector4u::Vector4u(Vector4u&& _v) noexcept : Vector<UInt32, 4>(_v) { }
Vector4u::Vector4u(Vector<UInt32, 4>&& _v) noexcept : Vector<UInt32, 4>(_v) { }

Vector4u& Vector4u::operator=(const Vector<UInt32, 4>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4u& Vector4u::operator=(Vector<UInt32, 4>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector4u& Vector4u::operator=(const Array<UInt32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4u& Vector4u::operator=(const Vector4u& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector4u& Vector4u::operator=(Vector4u&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const UInt32& Vector4u::operator[](const unsigned int& i) const noexcept {
    return Vector<UInt32, 4>::operator[](i);
}

UInt32& Vector4u::operator[](const unsigned int& i) noexcept {
    return Vector<UInt32, 4>::operator[](i);
}

Vector4u::operator Array<UInt32>() noexcept {
    Array<UInt32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector4u::Vector4u(const glm::u32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4u::Vector4u(glm::u32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4u::operator glm::u32vec4() const noexcept {
    return glm::u32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector4u::Vector4u(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMUINT4 e;
    DirectX::XMStoreUInt4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4u::Vector4u(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMUINT4 e;
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
    return DirectX::XMUINT4(m_elements);
}
#endif

// ------------------------------------------------------------------------------------------------
// Vector4i.
// ------------------------------------------------------------------------------------------------

Vector4i::Vector4i() noexcept : Vector<Int32, 4>() { }
Vector4i::Vector4i(const Int32& v) noexcept : Vector<Int32, 4>(v) { }
Vector4i::Vector4i(const Int32& x, const Int32& y, const Int32& z, const Int32& w) noexcept : Vector<Int32, 4>() {
    this->x() = x;
    this->y() = y;
    this->z() = z;
    this->w() = w;
}

Vector4i::Vector4i(const Vector4i& _v) noexcept : Vector<Int32, 4>(_v) { }
Vector4i::Vector4i(const Vector<Int32, 4>& _v) noexcept : Vector<Int32, 4>(_v) { }
Vector4i::Vector4i(Vector4i&& _v) noexcept : Vector<Int32, 4>(_v) { }
Vector4i::Vector4i(Vector<Int32, 4>&& _v) noexcept : Vector<Int32, 4>(_v) { }

Vector4i& Vector4i::operator=(const Vector<Int32, 4>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4i& Vector4i::operator=(Vector<Int32, 4>&& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return std::move(_other[i++]);
    });

    return *this;
}

Vector4i& Vector4i::operator=(const Array<Int32>& _other) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable {
        return _other[i++];
    });

    return *this;
}

Vector4i& Vector4i::operator=(const Vector4i& _other) noexcept {
    return this->operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
}

Vector4i& Vector4i::operator=(Vector4i&& _other) noexcept {
    return this->operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
}

const Int32& Vector4i::operator[](const unsigned int& i) const noexcept {
    return Vector<Int32, 4>::operator[](i);
}

Int32& Vector4i::operator[](const unsigned int& i) noexcept {
    return Vector<Int32, 4>::operator[](i);
}

Vector4i::operator Array<Int32>() noexcept {
    Array<Int32> v(vec_size);
    std::generate(std::begin(v), std::end(v), [this, i = 0]() mutable { return m_elements[i++]; });
    return v;
}

#if defined(BUILD_WITH_GLM)
Vector4i::Vector4i(const glm::i32vec4& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

Vector4i::Vector4i(glm::i32vec4&& v) noexcept {
    std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

Vector4i::operator glm::i32vec4() const noexcept {
    return glm::i32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]);
}
#endif

#if defined(BUILD_WITH_DIRECTX_MATH)
Vector4i::Vector4i(const DirectX::XMVECTOR& v) noexcept {
    DirectX::XMINT4 e;
    DirectX::XMStoreSInt4(&e, v);

    this->x() = e.x;
    this->y() = e.y;
    this->z() = e.z;
    this->w() = e.w;
}

Vector4i::Vector4i(DirectX::XMVECTOR&& v) noexcept {
    DirectX::XMINT4 e;
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
    return DirectX::XMINT4(m_elements);
}
#endif