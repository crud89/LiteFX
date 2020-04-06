#include <litefx/math_types.hpp>

using namespace LiteFX::Math;

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && D == 1>>
Vector<T, DIM>::Vector(const T& x) noexcept : m_elements({ x }) { }

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && D == 2>>
Vector<T, DIM>::Vector(const T& x, const T& y) noexcept : m_elements({ x, y }) { }

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && D == 3>>
Vector<T, DIM>::Vector(const T& x, const T& y, const T& z) noexcept : m_elements({ x, y, z }) { }

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && D == 4>>
Vector<T, DIM>::Vector(const T& x, const T& y, const T& z, const T& w) noexcept : m_elements({ x, y, z, w }) { }

template <typename T, int DIM>
Vector<T, DIM>::Vector(const vec_type& _other) noexcept
{
	std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return _other.m_elements[i++]; });
}

template <typename T, int DIM>
Vector<T, DIM>::Vector(vec_type&& _other) noexcept
{
	std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(_other.m_elements[i++]); });
}

#if defined(BUILD_ENABLE_GLM)
template <typename T, int DIM>
Vector<T, DIM>::Vector(glm::vec<DIM, T>& v) noexcept
{
	std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return v[i++]; });
}

template <typename T, int DIM>
Vector<T, DIM>::Vector(glm::vec<DIM, T>&& v) noexcept
{
	std::generate(std::begin(m_elements), std::end(m_elements), [&, i = 0]() mutable { return std::move(v[i++]); });
}

template <typename T, int DIM>
template <int D, typename std::enable_if_t<D == DIM && D == 1>>
Vector<T, DIM>::operator glm::vec<DIM, T>() noexcept 
{
	// TODO:
	//return glm::tvec1<T>(m_elements.x);
	return glm::tvec1<T>();
}

template <typename T, int DIM>
template <int D, typename std::enable_if_t<D == DIM && D == 2>>
Vector<T, DIM>::operator glm::vec<DIM, T>() noexcept
{
	return glm::tvec2<T>();
}

template <typename T, int DIM>
template <int D, typename std::enable_if_t<D == DIM && D == 3>>
Vector<T, DIM>::operator glm::vec<DIM, T>() noexcept
{
	return glm::tvec3<T>();
}

template <typename T, int DIM>
template <int D, typename std::enable_if_t<D == DIM && D == 4>>
Vector<T, DIM>::operator glm::vec<DIM, T>() noexcept
{
	return glm::tvec4<T>();
}
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 1>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	T e;
	DirectX::XMStoreFloat(&e, v);
	m_elements[0] = e;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 1>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	T e;
	DirectX::XMStoreInt(&e, v);
	m_elements[0] = e;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT2 e;
	DirectX::XMStoreFloat2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMFLOAT2& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT2 e;
	DirectX::XMStoreUInt2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMUINT2& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT2 e;
	DirectX::XMStoreFloat2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 2>>
Vector<T, DIM>::Vector(const DirectX::XMINT2& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT3 e;
	DirectX::XMStoreFloat3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMFLOAT3& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT3 e;
	DirectX::XMStoreUInt3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMUINT3& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT3 e;
	DirectX::XMStoreFloat3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 3>>
Vector<T, DIM>::Vector(const DirectX::XMINT3& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMFLOAT4 e;
	DirectX::XMStoreFloat4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMFLOAT4& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMUINT4 e;
	DirectX::XMStoreUInt4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMUINT4& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMVECTOR& v) noexcept {
	DirectX::XMINT4 e;
	DirectX::XMStoreFloat4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T, int DIM>
template <typename U, int D, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T> && D == 4>>
Vector<T, DIM>::Vector(const DirectX::XMINT4& e) noexcept {
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}
#endif

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
//typedef TVector2<int16_t> Vector2s;
//typedef TVector2<int32_t> Vector2i;
//typedef TVector2<uint32_t> Vector2u;
//typedef TVector2<float_t> Vector2f;
//
//typedef TVector3<uint8_t> Vector3b;
//typedef TVector3<int16_t> Vector3s;
//typedef TVector3<int32_t> Vector3i;
//typedef TVector3<uint32_t> Vector3u;
//typedef TVector3<float_t> Vector3f;
//
//typedef TVector4<uint8_t> Vector4b;
//typedef TVector4<int16_t> Vector4s;
//typedef TVector4<int32_t> Vector4i;
//typedef TVector4<uint32_t> Vector4u;
//typedef TVector4<float_t> Vector4f;