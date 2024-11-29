#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Rect.
// ------------------------------------------------------------------------------------------------

Rect::Rect() noexcept : Vector<scalar_type, vec_size>() {}
Rect::Rect(const Rect& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Rect::Rect(Rect&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

Rect::Rect(const Vector<size_t, 2>& pos, size_t w, size_t h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = pos.x();
	this->y() = pos.y();
	this->z() = w;
	this->w() = h;
}

Rect::Rect(size_t x, size_t y, size_t w, size_t h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = x;
	this->y() = y;
	this->z() = w;
	this->w() = h;
}

Rect& Rect::Rect::operator=(const Rect& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
	return *this;
}

Rect& Rect::operator=(Rect&& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
	return *this;
}

Vector<size_t, 2> Rect::position() const noexcept {
	auto p = Vector<size_t, 2>();
	p.x() = this->x();
	p.y() = this->y();
	return p;
}

Size2d Rect::extent() const noexcept {
	return { this->z(), this->w() };
}

size_t Rect::width() const noexcept {
	return this->z();
}

size_t& Rect::width() noexcept {
	return this->z();
}

size_t Rect::height() const noexcept {
	return this->w();
}

size_t& Rect::height() noexcept {
	return this->w();
}

// ------------------------------------------------------------------------------------------------
// RectI.
// ------------------------------------------------------------------------------------------------

RectI::RectI() noexcept : Vector<scalar_type, vec_size>() {}
RectI::RectI(const RectI& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
RectI::RectI(RectI&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

RectI::RectI(const Vector<Int32, 2>& pos, Int32 w, Int32 h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = pos.x();
	this->y() = pos.y();
	this->z() = w;
	this->w() = h;
}

RectI::RectI(Int32 x, Int32 y, Int32 w, Int32 h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = x;
	this->y() = y;
	this->z() = w;
	this->w() = h;
}

RectI& RectI::RectI::operator=(const RectI& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
	return *this;
}

RectI& RectI::operator=(RectI&& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
	return *this;
}

Vector<Int32, 2> RectI::position() const noexcept {
	return Vector2i(this->x(), this->y());
}

Size2d RectI::extent() const noexcept {
	return { static_cast<size_t>(this->z()), static_cast<size_t>(this->w()) };
}

Int32 RectI::width() const noexcept {
	return this->z();
}

Int32& RectI::width() noexcept {
	return this->z();
}

Int32 RectI::height() const noexcept {
	return this->w();
}

Int32& RectI::height() noexcept {
	return this->w();
}

// ------------------------------------------------------------------------------------------------
// RectF.
// ------------------------------------------------------------------------------------------------

RectF::RectF() noexcept : Vector<scalar_type, vec_size>() {}
RectF::RectF(const RectF& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
RectF::RectF(RectF&& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>&&>(_other)) {}

RectF::RectF(const Vector<Float, 2>& pos, Float w, Float h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = pos.x();
	this->y() = pos.y();
	this->z() = w;
	this->w() = h;
}

RectF::RectF(Float x, Float y, Float w, Float h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = x;
	this->y() = y;
	this->z() = w;
	this->w() = h;
}

RectF& RectF::RectF::operator=(const RectF& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>>(_other));
	return *this;
}

RectF& RectF::operator=(RectF&& _other) noexcept {
	Vector<scalar_type, vec_size>::operator=(static_cast<Vector<scalar_type, vec_size>&&>(_other));
	return *this;
}

Vector<Float, 2> RectF::position() const noexcept {
	return Vector2f(this->x(), this->y());
}

Size2d RectF::extent() const noexcept {
	return { static_cast<size_t>(this->z()), static_cast<size_t>(this->w()) };
}

Float RectF::width() const noexcept {
	return this->z();
}

Float& RectF::width() noexcept {
	return this->z();
}

Float RectF::height() const noexcept {
	return this->w();
}

Float& RectF::height() noexcept {
	return this->w();
}