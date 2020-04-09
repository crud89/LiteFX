#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Vector1f.
// ------------------------------------------------------------------------------------------------

#pragma region Vector1f

Vector1f::Vector(const scalar_type& x) noexcept : m_elements{ x } { }
Vector1f::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0] } { }
Vector1f::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]) } { }
const Vector1f::scalar_type* Vector1f::elements() const noexcept { return m_elements; }
Vector1f::scalar_type* Vector1f::elements() noexcept { return m_elements; }
const Vector1f::scalar_type& Vector1f::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector1f::scalar_type& Vector1f::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector1f::scalar_type& Vector1f::x() const { return m_elements[0]; }
Vector1f::scalar_type& Vector1f::x() { return m_elements[0]; }

#if defined(BUILD_ENABLE_GLM)
Vector1f::Vector(const glm::f32vec1& v) noexcept : m_elements{ v.x } { }
Vector1f::operator glm::f32vec1() noexcept { return glm::f32vec1(m_elements[0]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector1f::Vector(const DirectX::XMVECTOR& v) noexcept {
	scalar_type e;
	DirectX::XMStoreFloat(&e, v);
	m_elements[0] = e;
}

Vector1f::operator DirectX::XMVECTOR() noexcept { return DirectX::XMLoadFloat(m_elements); }
Vector1f::operator Vector1f::scalar_type() noexcept { return m_elements[0]; }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector1u
// ------------------------------------------------------------------------------------------------

#pragma region Vector1u

Vector1u::Vector(const scalar_type& x) noexcept : m_elements{ x } { }
Vector1u::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0] } { }
Vector1u::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]) } { }
const Vector1u::scalar_type* Vector1u::elements() const noexcept { return m_elements; }
Vector1u::scalar_type* Vector1u::elements() noexcept { return m_elements; }
const Vector1u::scalar_type& Vector1u::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector1u::scalar_type& Vector1u::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector1u::scalar_type& Vector1u::x() const { return m_elements[0]; }
Vector1u::scalar_type& Vector1u::x() { return m_elements[0]; }

#if defined(BUILD_ENABLE_GLM)
Vector1u::Vector(const glm::u32vec1& v) noexcept : m_elements{ v.x } { }
Vector1u::operator glm::u32vec1() noexcept { return glm::u32vec1(m_elements[0]); }
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector1u::Vector(const DirectX::XMVECTOR& v) noexcept {
	UInt32 e;
	DirectX::XMStoreInt(&e, v);
	m_elements[0] = e;
}

Vector1u::operator DirectX::XMVECTOR() noexcept { return DirectX::XMLoadInt(m_elements); }
Vector1u::operator Vector1u::scalar_type() noexcept { return m_elements[0]; }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector2f.
// ------------------------------------------------------------------------------------------------

#pragma region Vector2f

Vector2f::Vector(const scalar_type& v) noexcept : m_elements{ v, v } { }
Vector2f::Vector(const scalar_type& x, const scalar_type& y) noexcept : m_elements{ x, y } { }
Vector2f::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1] } { }
Vector2f::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]) } { }
const Vector2f::scalar_type* Vector2f::elements() const noexcept { return m_elements; }
Vector2f::scalar_type* Vector2f::elements() noexcept { return m_elements; }
const Vector2f::scalar_type& Vector2f::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector2f::scalar_type& Vector2f::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector2f::scalar_type& Vector2f::x() const { return m_elements[0]; }
Vector2f::scalar_type& Vector2f::x() { return m_elements[0]; }
const Vector2f::scalar_type& Vector2f::y() const { return m_elements[1]; }
Vector2f::scalar_type& Vector2f::y() { return m_elements[1]; }

#if defined(BUILD_ENABLE_GLM)
Vector2f::Vector(const glm::f32vec2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2f::operator glm::f32vec2() noexcept { return glm::f32vec2(m_elements[0], m_elements[1]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector2f::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT2 e;
	DirectX::XMStoreFloat2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

Vector2f::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMFLOAT2();
	return DirectX::XMLoadFloat2(&e); 
}

Vector2f::Vector(const DirectX::XMFLOAT2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2f::operator DirectX::XMFLOAT2() noexcept { return DirectX::XMFLOAT2(m_elements[0], m_elements[1]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector2u.
// ------------------------------------------------------------------------------------------------

#pragma region Vector2u

Vector2u::Vector(const scalar_type& v) noexcept : m_elements{ v, v } { }
Vector2u::Vector(const scalar_type& x, const scalar_type& y) noexcept : m_elements{ x, y } { }
Vector2u::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1] } { }
Vector2u::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]) } { }
const Vector2u::scalar_type* Vector2u::elements() const noexcept { return m_elements; }
Vector2u::scalar_type* Vector2u::elements() noexcept { return m_elements; }
const Vector2u::scalar_type& Vector2u::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector2u::scalar_type& Vector2u::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector2u::scalar_type& Vector2u::x() const { return m_elements[0]; }
Vector2u::scalar_type& Vector2u::x() { return m_elements[0]; }
const Vector2u::scalar_type& Vector2u::y() const { return m_elements[1]; }
Vector2u::scalar_type& Vector2u::y() { return m_elements[1]; }

#if defined(BUILD_ENABLE_GLM)
Vector2u::Vector(const glm::u32vec2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2u::operator glm::u32vec2() noexcept { return glm::u32vec2(m_elements[0], m_elements[1]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector2u::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT2 e;
	DirectX::XMStoreUInt2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

Vector2u::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMUINT2();
	return DirectX::XMLoadUInt2(&e);
}

Vector2u::Vector(const DirectX::XMUINT2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2u::operator DirectX::XMUINT2() noexcept { return DirectX::XMUINT2(m_elements[0], m_elements[1]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector2i.
// ------------------------------------------------------------------------------------------------

#pragma region Vector2i

Vector2i::Vector(const scalar_type& v) noexcept : m_elements{ v, v } { }
Vector2i::Vector(const scalar_type& x, const scalar_type& y) noexcept : m_elements{ x, y } { }
Vector2i::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1] } { }
Vector2i::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]) } { }
const Vector2i::scalar_type* Vector2i::elements() const noexcept { return m_elements; }
Vector2i::scalar_type* Vector2i::elements() noexcept { return m_elements; }
const Vector2i::scalar_type& Vector2i::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector2i::scalar_type& Vector2i::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector2i::scalar_type& Vector2i::x() const { return m_elements[0]; }
Vector2i::scalar_type& Vector2i::x() { return m_elements[0]; }
const Vector2i::scalar_type& Vector2i::y() const { return m_elements[1]; }
Vector2i::scalar_type& Vector2i::y() { return m_elements[1]; }

#if defined(BUILD_ENABLE_GLM)
Vector2i::Vector(const glm::i32vec2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2i::operator glm::i32vec2() noexcept { return glm::i32vec2(m_elements[0], m_elements[1]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector2i::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT2 e;
	DirectX::XMStoreSInt2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

Vector2i::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMINT2();
	return DirectX::XMLoadSInt2(&e);
}

Vector2i::Vector(const DirectX::XMINT2& v) noexcept : m_elements{ v.x, v.y } { }
Vector2i::operator DirectX::XMINT2() noexcept { return DirectX::XMINT2(m_elements[0], m_elements[1]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector3f.
// ------------------------------------------------------------------------------------------------

#pragma region Vector3f

Vector3f::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v } { }
Vector3f::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept : m_elements{ x, y, z } { }
Vector3f::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2] } { }
Vector3f::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]) } { }
const Vector3f::scalar_type* Vector3f::elements() const noexcept { return m_elements; }
Vector3f::scalar_type* Vector3f::elements() noexcept { return m_elements; }
const Vector3f::scalar_type& Vector3f::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector3f::scalar_type& Vector3f::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector3f::scalar_type& Vector3f::x() const { return m_elements[0]; }
Vector3f::scalar_type& Vector3f::x() { return m_elements[0]; }
const Vector3f::scalar_type& Vector3f::y() const { return m_elements[1]; }
Vector3f::scalar_type& Vector3f::y() { return m_elements[1]; }
const Vector3f::scalar_type& Vector3f::z() const { return m_elements[2]; }
Vector3f::scalar_type& Vector3f::z() { return m_elements[2]; }

#if defined(BUILD_ENABLE_GLM)
Vector3f::Vector(const glm::f32vec3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3f::operator glm::f32vec3() noexcept { return glm::f32vec3(m_elements[0], m_elements[1], m_elements[2]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector3f::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT3 e;
	DirectX::XMStoreFloat3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

Vector3f::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMFLOAT3();
	return DirectX::XMLoadFloat3(&e);
}

Vector3f::Vector(const DirectX::XMFLOAT3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3f::operator DirectX::XMFLOAT3() noexcept { return DirectX::XMFLOAT3(m_elements[0], m_elements[1], m_elements[2]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector3u.
// ------------------------------------------------------------------------------------------------

#pragma region Vector3u

Vector3u::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v } { }
Vector3u::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept : m_elements{ x, y, z } { }
Vector3u::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2] } { }
Vector3u::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]) } { }
const Vector3u::scalar_type* Vector3u::elements() const noexcept { return m_elements; }
Vector3u::scalar_type* Vector3u::elements() noexcept { return m_elements; }
const Vector3u::scalar_type& Vector3u::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector3u::scalar_type& Vector3u::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector3u::scalar_type& Vector3u::x() const { return m_elements[0]; }
Vector3u::scalar_type& Vector3u::x() { return m_elements[0]; }
const Vector3u::scalar_type& Vector3u::y() const { return m_elements[1]; }
Vector3u::scalar_type& Vector3u::y() { return m_elements[1]; }
const Vector3u::scalar_type& Vector3u::z() const { return m_elements[2]; }
Vector3u::scalar_type& Vector3u::z() { return m_elements[2]; }

#if defined(BUILD_ENABLE_GLM)
Vector3u::Vector(const glm::u32vec3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3u::operator glm::u32vec3() noexcept { return glm::u32vec3(m_elements[0], m_elements[1], m_elements[2]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector3u::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT3 e;
	DirectX::XMStoreUInt3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

Vector3u::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMUINT3();
	return DirectX::XMLoadUInt3(&e);
}

Vector3u::Vector(const DirectX::XMUINT3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3u::operator DirectX::XMUINT3() noexcept { return DirectX::XMUINT3(m_elements[0], m_elements[1], m_elements[2]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector3i.
// ------------------------------------------------------------------------------------------------

#pragma region Vector3i

Vector3i::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v } { }
Vector3i::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z) noexcept : m_elements{ x, y, z } { }
Vector3i::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2] } { }
Vector3i::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]) } { }
const Vector3i::scalar_type* Vector3i::elements() const noexcept { return m_elements; }
Vector3i::scalar_type* Vector3i::elements() noexcept { return m_elements; }
const Vector3i::scalar_type& Vector3i::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector3i::scalar_type& Vector3i::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector3i::scalar_type& Vector3i::x() const { return m_elements[0]; }
Vector3i::scalar_type& Vector3i::x() { return m_elements[0]; }
const Vector3i::scalar_type& Vector3i::y() const { return m_elements[1]; }
Vector3i::scalar_type& Vector3i::y() { return m_elements[1]; }
const Vector3i::scalar_type& Vector3i::z() const { return m_elements[2]; }
Vector3i::scalar_type& Vector3i::z() { return m_elements[2]; }

#if defined(BUILD_ENABLE_GLM)
Vector3i::Vector(const glm::i32vec3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3i::operator glm::i32vec3() noexcept { return glm::i32vec3(m_elements[0], m_elements[1], m_elements[2]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector3i::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT3 e;
	DirectX::XMStoreSInt3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

Vector3i::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMINT3();
	return DirectX::XMLoadSInt3(&e);
}

Vector3i::Vector(const DirectX::XMINT3& v) noexcept : m_elements{ v.x, v.y, v.z } { }
Vector3i::operator DirectX::XMINT3() noexcept { return DirectX::XMINT3(m_elements[0], m_elements[1], m_elements[2]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector4f.
// ------------------------------------------------------------------------------------------------

#pragma region Vector4f

Vector4f::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v, v } { }
Vector4f::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept : m_elements{ x, y, z, w } { }
Vector4f::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2], _other.m_elements[3] } { }
Vector4f::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]), std::move(_other.m_elements[3]) } { }
const Vector4f::scalar_type* Vector4f::elements() const noexcept { return m_elements; }
Vector4f::scalar_type* Vector4f::elements() noexcept { return m_elements; }
const Vector4f::scalar_type& Vector4f::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector4f::scalar_type& Vector4f::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector4f::scalar_type& Vector4f::x() const { return m_elements[0]; }
Vector4f::scalar_type& Vector4f::x() { return m_elements[0]; }
const Vector4f::scalar_type& Vector4f::y() const { return m_elements[1]; }
Vector4f::scalar_type& Vector4f::y() { return m_elements[1]; }
const Vector4f::scalar_type& Vector4f::z() const { return m_elements[2]; }
Vector4f::scalar_type& Vector4f::z() { return m_elements[2]; }
const Vector4f::scalar_type& Vector4f::w() const { return m_elements[3]; }
Vector4f::scalar_type& Vector4f::w() { return m_elements[3]; }

#if defined(BUILD_ENABLE_GLM)
Vector4f::Vector(const glm::f32vec4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4f::operator glm::f32vec4() noexcept { return glm::f32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector4f::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT4 e;
	DirectX::XMStoreFloat4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

Vector4f::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMFLOAT4();
	return DirectX::XMLoadFloat4(&e);
}

Vector4f::Vector(const DirectX::XMFLOAT4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4f::operator DirectX::XMFLOAT4() noexcept { return DirectX::XMFLOAT4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector4u.
// ------------------------------------------------------------------------------------------------

#pragma region Vector4u

Vector4u::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v, v } { }
Vector4u::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept : m_elements{ x, y, z, w } { }
Vector4u::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2], _other.m_elements[3] } { }
Vector4u::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]), std::move(_other.m_elements[3]) } { }
const Vector4u::scalar_type* Vector4u::elements() const noexcept { return m_elements; }
Vector4u::scalar_type* Vector4u::elements() noexcept { return m_elements; }
const Vector4u::scalar_type& Vector4u::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector4u::scalar_type& Vector4u::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector4u::scalar_type& Vector4u::x() const { return m_elements[0]; }
Vector4u::scalar_type& Vector4u::x() { return m_elements[0]; }
const Vector4u::scalar_type& Vector4u::y() const { return m_elements[1]; }
Vector4u::scalar_type& Vector4u::y() { return m_elements[1]; }
const Vector4u::scalar_type& Vector4u::z() const { return m_elements[2]; }
Vector4u::scalar_type& Vector4u::z() { return m_elements[2]; }
const Vector4u::scalar_type& Vector4u::w() const { return m_elements[3]; }
Vector4u::scalar_type& Vector4u::w() { return m_elements[3]; }

#if defined(BUILD_ENABLE_GLM)
Vector4u::Vector(const glm::u32vec4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4u::operator glm::u32vec4() noexcept { return glm::u32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector4u::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT4 e;
	DirectX::XMStoreUInt4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

Vector4u::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMUINT4();
	return DirectX::XMLoadUInt4(&e);
}

Vector4u::Vector(const DirectX::XMUINT4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4u::operator DirectX::XMUINT4() noexcept { return DirectX::XMUINT4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Vector4i.
// ------------------------------------------------------------------------------------------------

#pragma region Vector4i

Vector4i::Vector(const scalar_type& v) noexcept : m_elements{ v, v, v, v } { }
Vector4i::Vector(const scalar_type& x, const scalar_type& y, const scalar_type& z, const scalar_type& w) noexcept : m_elements{ x, y, z, w } { }
Vector4i::Vector(const vec_type& _other) noexcept : m_elements{ _other.m_elements[0], _other.m_elements[1], _other.m_elements[2], _other.m_elements[3] } { }
Vector4i::Vector(vec_type&& _other) noexcept : m_elements{ std::move(_other.m_elements[0]), std::move(_other.m_elements[1]), std::move(_other.m_elements[2]), std::move(_other.m_elements[3]) } { }
const Vector4i::scalar_type* Vector4i::elements() const noexcept { return m_elements; }
Vector4i::scalar_type* Vector4i::elements() noexcept { return m_elements; }
const Vector4i::scalar_type& Vector4i::operator[](const int& i) const noexcept { return m_elements[i]; }
Vector4i::scalar_type& Vector4i::operator[](const int& i) noexcept { return m_elements[i]; }
const Vector4i::scalar_type& Vector4i::x() const { return m_elements[0]; }
Vector4i::scalar_type& Vector4i::x() { return m_elements[0]; }
const Vector4i::scalar_type& Vector4i::y() const { return m_elements[1]; }
Vector4i::scalar_type& Vector4i::y() { return m_elements[1]; }
const Vector4i::scalar_type& Vector4i::z() const { return m_elements[2]; }
Vector4i::scalar_type& Vector4i::z() { return m_elements[2]; }
const Vector4i::scalar_type& Vector4i::w() const { return m_elements[3]; }
Vector4i::scalar_type& Vector4i::w() { return m_elements[3]; }

#if defined(BUILD_ENABLE_GLM)
Vector4i::Vector(const glm::i32vec4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4i::operator glm::i32vec4() noexcept { return glm::i32vec4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif 

#if defined(BUILD_ENABLE_DIRECTX_MATH)
Vector4i::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT4 e;
	DirectX::XMStoreSInt4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

Vector4i::operator DirectX::XMVECTOR() noexcept {
	auto e = this->operator DirectX::XMINT4();
	return DirectX::XMLoadSInt4(&e);
}

Vector4i::Vector(const DirectX::XMINT4& v) noexcept : m_elements{ v.x, v.y, v.z, v.w } { }
Vector4i::operator DirectX::XMINT4() noexcept { return DirectX::XMINT4(m_elements[0], m_elements[1], m_elements[2], m_elements[3]); }
#endif
#pragma endregion

// ------------------------------------------------------------------------------------------------
// Specialize classes so that they get exported.
// ------------------------------------------------------------------------------------------------

class LITEFX_MATH_API CVector1f : public Vector1f { };
class LITEFX_MATH_API CVector1u : public Vector1u { };
class LITEFX_MATH_API CVector2f : public Vector2f { };
class LITEFX_MATH_API CVector2u : public Vector2u { };
class LITEFX_MATH_API CVector2i : public Vector2i { };
class LITEFX_MATH_API CVector3f : public Vector3f { };
class LITEFX_MATH_API CVector3u : public Vector3u { };
class LITEFX_MATH_API CVector3i : public Vector3i { };
class LITEFX_MATH_API CVector4f : public Vector4f { };
class LITEFX_MATH_API CVector4u : public Vector4u { };
class LITEFX_MATH_API CVector4i : public Vector4i { };