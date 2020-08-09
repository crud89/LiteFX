#include <litefx/math.hpp>

using namespace LiteFX::Math;

// ------------------------------------------------------------------------------------------------
// Rect.
// ------------------------------------------------------------------------------------------------

Rect::Rect() noexcept : Vector<scalar_type, vec_size>() {}
Rect::Rect(const Rect& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
Rect::Rect(Rect&& _other) noexcept : Vector<scalar_type, vec_size>(std::move(static_cast<Vector<scalar_type, vec_size>>(_other))) {}

Rect::Rect(const Vector<size_t, 2>& pos, const size_t& w, const size_t& h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = pos.x();
	this->y() = pos.y();
	this->z() = w;
	this->w() = h;
}

Rect::Rect(const size_t& x, const size_t& y, const size_t& w, const size_t& h) noexcept : Vector<scalar_type, vec_size>() {
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
	Vector<scalar_type, vec_size>::operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
	return *this;
}

Vector<size_t, 2> Rect::position() const noexcept {
	auto p = Vector<size_t, 2>();
	p.x() = this->x();
	p.y() = this->y();
	return p;
}

Size2d Rect::extent() const noexcept {
	return Size2d(this->z(), this->w());
}

const size_t& Rect::width() const noexcept {
	return this->z();
}

size_t& Rect::width() noexcept {
	return this->z();
}

const size_t& Rect::height() const noexcept {
	return this->w();
}

size_t& Rect::height() noexcept {
	return this->w();
}

// ------------------------------------------------------------------------------------------------
// RectF.
// ------------------------------------------------------------------------------------------------

RectF::RectF() noexcept : Vector<scalar_type, vec_size>() {}
RectF::RectF(const RectF& _other) noexcept : Vector<scalar_type, vec_size>(static_cast<Vector<scalar_type, vec_size>>(_other)) {}
RectF::RectF(RectF&& _other) noexcept : Vector<scalar_type, vec_size>(std::move(static_cast<Vector<scalar_type, vec_size>>(_other))) {}

RectF::RectF(const Vector<Float, 2>& pos, const Float& w, const Float& h) noexcept : Vector<scalar_type, vec_size>() {
	this->x() = pos.x();
	this->y() = pos.y();
	this->z() = w;
	this->w() = h;
}

RectF::RectF(const Float& x, const Float& y, const Float& w, const Float& h) noexcept : Vector<scalar_type, vec_size>() {
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
	Vector<scalar_type, vec_size>::operator=(std::move(static_cast<Vector<scalar_type, vec_size>>(_other)));
	return *this;
}

Vector<Float, 2> RectF::position() const noexcept {
	return Vector2f(this->x(), this->y());
}

Size2d RectF::extent() const noexcept {
	return Size2d(this->z(), this->w());
}

const Float& RectF::width() const noexcept {
	return this->z();
}

Float& RectF::width() noexcept {
	return this->z();
}

const Float& RectF::height() const noexcept {
	return this->w();
}

Float& RectF::height() noexcept {
	return this->w();
}