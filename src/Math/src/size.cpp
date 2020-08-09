#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Size2d.
// ------------------------------------------------------------------------------------------------

Size2d::Size2d() noexcept : Vector<scalar_type, vec_size>() {}
Size2d::Size2d(const size_t& v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size2d::Size2d(const size_t& w, const size_t& h) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
}

Size2d::Size2d(const Size2d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size2d::Size2d(Size2d&& _other) noexcept : Vector<scalar_type, vec_size>(std::move(static_cast<Vector<scalar_type, vec_size>>(_other))) {}

Size2d& Size2d::operator=(const Size2d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size2d& Size2d::operator=(Size2d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
    return *this;
}

const size_t& Size2d::width() const noexcept{ return this->x(); }
size_t& Size2d::width() noexcept { return this->x(); }
const size_t& Size2d::height() const noexcept { return this->y(); }
size_t& Size2d::height() noexcept { return this->y(); }

// ------------------------------------------------------------------------------------------------
// Size3d.
// ------------------------------------------------------------------------------------------------

Size3d::Size3d() noexcept : Vector<scalar_type, vec_size>() {}
Size3d::Size3d(const size_t& v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size3d::Size3d(const size_t& w, const size_t& h, const size_t& d) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
    this->z() = d;
}

Size3d::Size3d(const Size3d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size3d::Size3d(Size3d&& _other) noexcept : Vector<scalar_type, vec_size>(std::move(static_cast<Vector<scalar_type, vec_size>>(_other))) {}

Size3d& Size3d::operator=(const Size3d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size3d& Size3d::operator=(Size3d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
    return *this;
}

const size_t& Size3d::width() const noexcept { return this->x(); }
size_t& Size3d::width() noexcept { return this->x(); }
const size_t& Size3d::height() const noexcept { return this->y(); }
size_t& Size3d::height() noexcept { return this->y(); }
const size_t& Size3d::depth() const noexcept { return this->z(); }
size_t& Size3d::depth() noexcept { return this->z(); }

// ------------------------------------------------------------------------------------------------
// Size4d.
// ------------------------------------------------------------------------------------------------

Size4d::Size4d() noexcept : Vector<scalar_type, vec_size>() {}
Size4d::Size4d(const size_t& v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size4d::Size4d(const size_t& w, const size_t& h, const size_t& d, const size_t& a) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
    this->z() = d;
    this->w() = a;
}

Size4d::Size4d(const Size4d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size4d::Size4d(Size4d&& _other) noexcept : Vector<scalar_type, vec_size>(std::move(static_cast<Vector<scalar_type, vec_size>>(_other))) {}

Size4d& Size4d::operator=(const Size4d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size4d& Size4d::operator=(Size4d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
    return *this;
}

const size_t& Size4d::width() const noexcept { return this->x(); }
size_t& Size4d::width() noexcept { return this->x(); }
const size_t& Size4d::height() const noexcept { return this->y(); }
size_t& Size4d::height() noexcept { return this->y(); }
const size_t& Size4d::depth() const noexcept { return this->z(); }
size_t& Size4d::depth() noexcept { return this->z(); }
const size_t& Size4d::alpha() const noexcept { return this->w(); }
size_t& Size4d::alpha() noexcept { return this->w(); }