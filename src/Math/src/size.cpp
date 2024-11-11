#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Size2d.
// ------------------------------------------------------------------------------------------------

Size2d::Size2d() noexcept : Vector<scalar_type, vec_size>() {}
Size2d::Size2d(size_t v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size2d::Size2d(size_t w, size_t h) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
}

Size2d::Size2d(const Size2d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size2d::Size2d(Size2d&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

Size2d& Size2d::operator=(const Size2d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size2d& Size2d::operator=(Size2d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
    return *this;
}

Size2d::operator Size3d() const noexcept { return Size3d{ this->width(), this->height(), 1 }; }
Size2d::operator Size4d() const noexcept { return Size4d{ this->width(), this->height(), 1, 1 }; }
Size2d Size2d::operator/(size_t s) noexcept { return Size2d{ this->width() / s, this->height() / s }; }
Size2d& Size2d::operator/=(size_t s) noexcept { this->width() /= s; this->height() /= s; return *this; }
Size2d Size2d::operator*(size_t s) noexcept { return Size2d{ this->width() * s, this->height() * s }; }
Size2d& Size2d::operator*=(size_t s) noexcept { this->width() *= s; this->height() *= s; return *this; }
Size2d Size2d::operator+(const Size2d& s) noexcept { return Size2d{ this->width() + s.width(), this->height() + s.height() }; }
Size2d& Size2d::operator+=(const Size2d& s) noexcept { this->width() += s.width(); this->height() += s.height(); return *this; }
Size2d Size2d::operator-(const Size2d& s) noexcept { return Size2d{ this->width() - s.width(), this->height() - s.height() }; }
Size2d& Size2d::operator-=(const Size2d& s) noexcept { this->width() -= s.width(); this->height() -= s.height(); return *this; }

size_t Size2d::width() const noexcept{ return this->x(); }
size_t& Size2d::width() noexcept { return this->x(); }
size_t Size2d::height() const noexcept { return this->y(); }
size_t& Size2d::height() noexcept { return this->y(); }

// ------------------------------------------------------------------------------------------------
// Size3d.
// ------------------------------------------------------------------------------------------------

Size3d::Size3d() noexcept : Vector<scalar_type, vec_size>() {}
Size3d::Size3d(size_t v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size3d::Size3d(size_t w, size_t h, size_t d) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
    this->z() = d;
}

Size3d::Size3d(const Size3d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size3d::Size3d(Size3d&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

Size3d& Size3d::operator=(const Size3d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size3d& Size3d::operator=(Size3d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
    return *this;
}

Size3d::operator Size4d() const noexcept { return Size4d{ this->width(), this->height(), this->depth(), 1 }; }
Size3d Size3d::operator/(size_t s) noexcept { return Size3d{ this->width() / s, this->height() / s, this->depth() / s }; }
Size3d& Size3d::operator/=(size_t s) noexcept { this->width() /= s; this->height() /= s; this->depth() /= s; return *this; }
Size3d Size3d::operator*(size_t s) noexcept { return Size3d{ this->width() * s, this->height() * s, this->depth() * s }; }
Size3d& Size3d::operator*=(size_t s) noexcept { this->width() *= s; this->height() *= s; this->depth() *= s; return *this; }
Size3d Size3d::operator+(const Size3d& s) noexcept { return Size3d{ this->width() + s.width(), this->height() + s.height(), this->depth() + s.depth() }; }
Size3d& Size3d::operator+=(const Size3d& s) noexcept { this->width() += s.width(); this->height() += s.height(); this->depth() += s.depth() ; return *this; }
Size3d Size3d::operator-(const Size3d& s) noexcept { return Size3d{ this->width() - s.width(), this->height() - s.height(), this->depth() - s.depth() }; }
Size3d& Size3d::operator-=(const Size3d& s) noexcept { this->width() -= s.width(); this->height() -= s.height(); this->depth() -= s.depth(); return *this; }
size_t Size3d::width() const noexcept { return this->x(); }
size_t& Size3d::width() noexcept { return this->x(); }
size_t Size3d::height() const noexcept { return this->y(); }
size_t& Size3d::height() noexcept { return this->y(); }
size_t Size3d::depth() const noexcept { return this->z(); }
size_t& Size3d::depth() noexcept { return this->z(); }

// ------------------------------------------------------------------------------------------------
// Size4d.
// ------------------------------------------------------------------------------------------------

Size4d::Size4d() noexcept : Vector<scalar_type, vec_size>() {}
Size4d::Size4d(size_t v) noexcept : Vector<scalar_type, vec_size>(v) {}
Size4d::Size4d(size_t w, size_t h, size_t d, size_t a) noexcept : Vector<scalar_type, vec_size>() {
    this->x() = w;
    this->y() = h;
    this->z() = d;
    this->w() = a;
}

Size4d::Size4d(const Size4d& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Size4d::Size4d(Size4d&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

Size4d& Size4d::operator=(const Size4d& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
    return *this;
}

Size4d& Size4d::operator=(Size4d&& _other) noexcept {
    Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
    return *this;
}

Size4d Size4d::operator/(size_t s) noexcept { return Size4d{ this->width() / s, this->height() / s, this->depth() / s, this->alpha() / s }; }
Size4d& Size4d::operator/=(size_t s) noexcept { this->width() /= s; this->height() /= s; this->depth() /= s; this->alpha() /= s; return *this; }
Size4d Size4d::operator*(size_t s) noexcept { return Size4d{ this->width() * s, this->height() * s, this->depth() * s, this->alpha() * s }; }
Size4d& Size4d::operator*=(size_t s) noexcept { this->width() *= s; this->height() *= s; this->depth() *= s; this->alpha() *= s; return *this; }
Size4d Size4d::operator+(const Size4d& s) noexcept { return Size4d{ this->width() + s.width(), this->height() + s.height(), this->depth() + s.depth(), this->alpha() + s.alpha() }; }
Size4d& Size4d::operator+=(const Size4d& s) noexcept { this->width() += s.width(); this->height() += s.height(); this->depth() += s.depth(); this->alpha() += s.alpha(); return *this; }
Size4d Size4d::operator-(const Size4d& s) noexcept { return Size4d{ this->width() - s.width(), this->height() - s.height(), this->depth() - s.depth(), this->alpha() - s.alpha() }; }
Size4d& Size4d::operator-=(const Size4d& s) noexcept { this->width() -= s.width(); this->height() -= s.height(); this->depth() -= s.depth(); this->alpha() -= s.alpha(); return *this; }

size_t Size4d::width() const noexcept { return this->x(); }
size_t& Size4d::width() noexcept { return this->x(); }
size_t Size4d::height() const noexcept { return this->y(); }
size_t& Size4d::height() noexcept { return this->y(); }
size_t Size4d::depth() const noexcept { return this->z(); }
size_t& Size4d::depth() noexcept { return this->z(); }
size_t Size4d::alpha() const noexcept { return this->w(); }
size_t& Size4d::alpha() noexcept { return this->w(); }