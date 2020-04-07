#include <litefx/math_types.hpp>

using namespace LiteFX::Math;

template <typename T>
Vector<T, 1>::Vector(const T& x) noexcept : m_elements({ x }) { }

template <typename T>
Vector<T, 2>::Vector(const T& x, const T& y) noexcept : m_elements({ x, y }) { }

template <typename T>
Vector<T, 3>::Vector(const T& x, const T& y, const T& z) noexcept : m_elements({ x, y, z }) { }

template <typename T>
Vector<T, 4>::Vector(const T& x, const T& y, const T& z, const T& w) noexcept : m_elements({ x, y, z, w }) { }

template <typename T>
Vector<T, 1>::Vector(const vec_type& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
}

template <typename T>
Vector<T, 2>::Vector(const vec_type& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
}

template <typename T>
Vector<T, 3>::Vector(const vec_type& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
	m_elements[2] = std::move(_other.m_elements[2]);
}

template <typename T>
Vector<T, 4>::Vector(const vec_type& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
	m_elements[2] = std::move(_other.m_elements[2]);
	m_elements[3] = std::move(_other.m_elements[3]);
}

template <typename T>
Vector<T, 1>::Vector(vec_type&& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
}

template <typename T>
Vector<T, 2>::Vector(vec_type&& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
}

template <typename T>
Vector<T, 3>::Vector(vec_type&& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
	m_elements[2] = std::move(_other.m_elements[2]);
}

template <typename T>
Vector<T, 4>::Vector(vec_type&& _other) noexcept
{
	m_elements[0] = std::move(_other.m_elements[0]);
	m_elements[1] = std::move(_other.m_elements[1]);
	m_elements[2] = std::move(_other.m_elements[2]);
	m_elements[3] = std::move(_other.m_elements[3]);
}

#if defined(BUILD_ENABLE_GLM)
template <typename T>
Vector<T, 1>::Vector(const glm::tvec1<T>& v) noexcept : m_elements({ v.x }) { }

template <typename T>
Vector<T, 2>::Vector(const glm::tvec2<T>& v) noexcept : m_elements({ v.x, v.y }) { }

template <typename T>
Vector<T, 3>::Vector(const glm::tvec3<T>& v) noexcept : m_elements({ v.x, v.y, v.z }) { }

template <typename T>
Vector<T, 4>::Vector(const glm::tvec4<T>& v) noexcept : m_elements({ v.x, v.y, v.z, v.w }) { }

template <typename T>
Vector<T, 1>::operator glm::tvec1<T>() noexcept
{
	return glm::tvec1<T>(m_elements[0]);
}

template <typename T>
Vector<T, 2>::operator glm::tvec2<T>() noexcept
{
	return glm::tvec2<T>(m_elements[0], m_elements[1]);
}

template <typename T>
Vector<T, 3>::operator glm::tvec3<T>() noexcept
{
	return glm::tvec3<T>(m_elements[0], m_elements[1], m_elements[2]);
}

template <typename T>
Vector<T, 4>::operator glm::tvec4<T>() noexcept
{
	return glm::tvec4<T>(m_elements[0], m_elements[1], m_elements[2], m_elements[3]);
}
#endif

#if defined(BUILD_ENABLE_DIRECTX_MATH)
template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 1>::Vector(const DirectX::XMVECTOR& v) noexcept 
{
	T e;
	DirectX::XMStoreFloat(&e, v);
	m_elements[0] = e;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 1>::Vector(const DirectX::XMVECTOR& v) noexcept 
{
	T e;
	DirectX::XMStoreInt(&e, v);
	m_elements[0] = e;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 1>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadFloat(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 1>::operator Float () noexcept
{
	return m_elements[0];
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 1>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadInt(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 1>::operator UInt32 () noexcept
{
	return m_elements[0];
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 2>::Vector(const DirectX::XMVECTOR& v) noexcept 
{
	DirectX::XMFLOAT2 e;
	DirectX::XMStoreFloat2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 2>::Vector(const DirectX::XMFLOAT2& e) noexcept 
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 2>::Vector(const DirectX::XMVECTOR& v) noexcept 
{
	DirectX::XMUINT2 e;
	DirectX::XMStoreUInt2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 2>::Vector(const DirectX::XMUINT2& e) noexcept 
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 2>::Vector(const DirectX::XMVECTOR& v) noexcept 
{
	DirectX::XMINT2 e;
	DirectX::XMStoreFloat2(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 2>::Vector(const DirectX::XMINT2& e) noexcept 
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 2>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadFloat2(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 2>::operator DirectX::XMFLOAT2 () noexcept
{
	return DirectX::XMFLOAT2(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 2>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadInt2(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 2>::operator DirectX::XMUINT2 () noexcept
{
	return DirectX::XMUINT2(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 2>::operator DirectX::XMVECTOR () noexcept
{
	DirectX::XMINT2 e(&m_elements);
	return DirectX::XMLoadSInt2(&e);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 2>::operator DirectX::XMINT2() noexcept
{
	return DirectX::XMINT2(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 3>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMFLOAT2 e;
	DirectX::XMStoreFloat3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 3>::Vector(const DirectX::XMFLOAT3& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 3>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMUINT2 e;
	DirectX::XMStoreUInt3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 3>::Vector(const DirectX::XMUINT3& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 3>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMINT2 e;
	DirectX::XMStoreFloat3(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 3>::Vector(const DirectX::XMINT3& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 3>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadFloat3(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 3>::operator DirectX::XMFLOAT3 () noexcept
{
	return DirectX::XMFLOAT3(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 3>::operator DirectX::XMVECTOR () noexcept
{
	return DirectX::XMLoadInt3(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 3>::operator DirectX::XMUINT3 () noexcept
{
	return DirectX::XMUINT3(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 3>::operator DirectX::XMVECTOR () noexcept
{
	DirectX::XMINT3 e(&m_elements);
	return DirectX::XMLoadSInt3(&e);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 3>::operator DirectX::XMINT3() noexcept
{
	return DirectX::XMINT3(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 4>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMFLOAT2 e;
	DirectX::XMStoreFloat4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 4>::Vector(const DirectX::XMFLOAT4& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 4>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMUINT2 e;
	DirectX::XMStoreUInt4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 4>::Vector(const DirectX::XMUINT4& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 4>::Vector(const DirectX::XMVECTOR& v) noexcept
{
	DirectX::XMINT2 e;
	DirectX::XMStoreFloat4(&e, v);
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 4>::Vector(const DirectX::XMINT4& e) noexcept
{
	m_elements[0] = e.x;
	m_elements[1] = e.y;
	m_elements[2] = e.z;
	m_elements[3] = e.w;
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 4>::operator DirectX::XMVECTOR() noexcept
{
	return DirectX::XMLoadFloat4(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Float>>>
Vector<T, 4>::operator DirectX::XMFLOAT4() noexcept
{
	return DirectX::XMFLOAT4(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 4>::operator DirectX::XMVECTOR() noexcept
{
	return DirectX::XMLoadInt4(m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, UInt32> && std::is_unsigned_v<T>>>
Vector<T, 4>::operator DirectX::XMUINT4() noexcept
{
	return DirectX::XMUINT4(&m_elements);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 4>::operator DirectX::XMVECTOR() noexcept
{
	DirectX::XMINT4 e(&m_elements);
	return DirectX::XMLoadSInt4(&e);
}

template <typename T>
template <typename U, typename std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<T, Int32> && std::is_signed_v<T>>>
Vector<T, 4>::operator DirectX::XMINT4() noexcept
{
	return DirectX::XMINT4(&m_elements);
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