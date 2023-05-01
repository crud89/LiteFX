///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
// NOTICE: This file contains an implementation for `std::generator`, which is in large parts    //
//         based on a draft implementation for the Microsoft Standard Library:                   //
//         https://github.com/CaseyCarter/STL/tree/generator.                                    //
//                                                                                               //
//         Minor differences between the implementations are founded in the compatibility with   // 
//         the project. The source is re-distributed under terms of the Apache License v2.0 with //
//         LLVM Exception (see below).                                                           //
//                                                                                               //
/////////////////////////////////////////////////////////////////////////////////////////////////// 
//                                                                                               //
// Microsoft C++ Standard Library                                                                //
//                                                                                               //
// Copyright(c) Microsoft Corporation                                                            //
//                                                                                               //
// Licensed under the Apache License, Version 2.0 (the "License");                               //
// you may not use this file except in compliance with the License.                              //
// You may obtain a copy of the License at                                                       //
//                                                                                               //
//    http://www.apache.org/licenses/LICENSE-2.0                                                //
//                                                                                               //
// Unless required by applicable law or agreed to in writing, software                           //
// distributed under the License is distributed on an "AS IS" BASIS,                             //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      //
// See the License for the specific language governing permissions and                           //
// limitations under the License.                                                                //
//                                                                                               //
//                                                                                               //
// ----LLVM Exceptions to the Apache 2.0 License----                                             //
//                                                                                               //
// As an exception, if, as a result of your compiling your source code, portions                 //
// of this Software are embedded into an Object form of such source code, you                    //
// may redistribute such embedded portions in such Object form without complying                 //
// with the conditions of Sections 4(a), 4(b) and 4(d)of the License.                            //
//                                                                                               //
// In addition, if you combine or link compiled forms of this Software with                      //
// software that is licensed under the GPLv2("Combined Software") and if a                       //
// court of competent jurisdiction determines that the patent provision(Section                  //
//     3), the indemnity provision(Section 9) or other Section of the License                    //
//     conflicts with the conditions of the GPLv2, you may retroactively and                     //
//     prospectively choose to deem waived or otherwise exclude such Section(s) of               //
//     the License, but only in their entirety and only with respect to the Combined             //
//     Software.                                                                                 //
//                                                                                               //
/////////////////////////////////////////////////////////////////////////////////////////////////// 

#pragma once

#if __has_include(<generator>)
#include <generator>
#else

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <coroutine>
#include <exception>
#include <ranges>

namespace std {

    struct alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__) _Aligned_block {
        unsigned char _Pad[__STDCPP_DEFAULT_NEW_ALIGNMENT__];
    };

    template <class _Alloc>
    concept _Has_real_pointers = same_as<_Alloc, void> || is_pointer_v<typename allocator_traits<_Alloc>::pointer>;

    template <class _Allocator = void>
    class _Promise_allocator { // statically specified allocator type
    private:
        using _Alloc = _Rebind_alloc_t<_Allocator, _Aligned_block>;

        static void* _Allocate(_Alloc _Al, const size_t _Size) {
            if constexpr (default_initializable<_Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // do not store stateless allocator
                const size_t _Count = (_Size + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                return _Al.allocate(_Count);
            }
            else {
                // store stateful allocator
                static constexpr size_t _Align = (_STD max)(alignof(_Alloc), sizeof(_Aligned_block));
                const size_t _Count = (_Size + sizeof(_Alloc) + _Align - 1) / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1) & ~(alignof(_Alloc) - 1);
                ::new (reinterpret_cast<void*>(_Al_address)) _Alloc(_STD move(_Al));
                return _Ptr;
            }
        }

    public:
        static void* operator new(const size_t _Size) requires default_initializable<_Alloc> {
            return _Allocate(_Alloc{}, _Size);
        }

        template <class _Alloc2, class... _Args>
            requires convertible_to<const _Alloc2&, _Allocator>
        static void* operator new(const size_t _Size, allocator_arg_t, const _Alloc2& _Al, const _Args&...) {
            return _Allocate(static_cast<_Alloc>(static_cast<_Allocator>(_Al)), _Size);
        }

        template <class _This, class _Alloc2, class... _Args>
            requires convertible_to<const _Alloc2&, _Allocator>
        static void* operator new(const size_t _Size, const _This&, allocator_arg_t, const _Alloc2& _Al, const _Args&...) {
            return _Allocate(static_cast<_Alloc>(static_cast<_Allocator>(_Al)), _Size);
        }

        static void operator delete(void* const _Ptr, const size_t _Size) noexcept {
            if constexpr (default_initializable<_Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // make stateless allocator
                _Alloc _Al{};
                const size_t _Count = (_Size + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
            }
            else {
                // retrieve stateful allocator
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1) & ~(alignof(_Alloc) - 1);
                auto& _Stored_al = *reinterpret_cast<_Alloc*>(_Al_address);
                _Alloc _Al{ _STD move(_Stored_al) };
                _Stored_al.~_Alloc();

                static constexpr size_t _Align = (_STD max)(alignof(_Alloc), sizeof(_Aligned_block));
                const size_t _Count = (_Size + sizeof(_Alloc) + _Align - 1) / sizeof(_Aligned_block);
                _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
            }
        }
    };

    template <>
    class _Promise_allocator<void> { // type-erased allocator
    private:
        using _Dealloc_fn = void (*)(void*, size_t);

        template <class _ProtoAlloc>
        static void* _Allocate(const _ProtoAlloc& _Proto, size_t _Size) {
            using _Alloc = _Rebind_alloc_t<_ProtoAlloc, _Aligned_block>;
            auto _Al = static_cast<_Alloc>(_Proto);

            if constexpr (default_initializable<_Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // don't store stateless allocator
                const _Dealloc_fn _Dealloc = [](void* const _Ptr, const size_t _Size) {
                    _Alloc _Al{};
                    const size_t _Count =
                        (_Size + sizeof(_Dealloc_fn) + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                    _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
                };

                const size_t _Count = (_Size + sizeof(_Dealloc_fn) + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                _CSTD memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc));
                return _Ptr;
            }
            else {
                // store stateful allocator
                static constexpr size_t _Align = (_STD max)(alignof(_Alloc), sizeof(_Aligned_block));

                const _Dealloc_fn _Dealloc = [](void* const _Ptr, size_t _Size) {
                    _Size += sizeof(_Dealloc_fn);
                    const auto _Al_address =
                        (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1) & ~(alignof(_Alloc) - 1);
                    auto& _Stored_al = *reinterpret_cast<const _Alloc*>(_Al_address);
                    _Alloc _Al{ _STD move(_Stored_al) };
                    _Stored_al.~_Alloc();

                    const size_t _Count = (_Size + sizeof(_Al) + _Align - 1) / sizeof(_Aligned_block);
                    _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
                };

                const size_t _Count = (_Size + sizeof(_Dealloc_fn) + sizeof(_Al) + _Align - 1) / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                _CSTD memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc));
                _Size += sizeof(_Dealloc_fn);
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1) & ~(alignof(_Alloc) - 1);
                ::new (reinterpret_cast<void*>(_Al_address)) _Alloc{ _STD move(_Al) };
                return _Ptr;
            }
        }

    public:
        static void* operator new(const size_t _Size) { // default: new/delete
            void* const _Ptr = ::operator new[](_Size + sizeof(_Dealloc_fn));
            const _Dealloc_fn _Dealloc = [](void* const _Ptr, const size_t _Size) {
                ::operator delete[](_Ptr, _Size + sizeof(_Dealloc_fn));
            };
            _CSTD memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc_fn));
            return _Ptr;
        }

        template <class _Alloc, class... _Args>
        static void* operator new(const size_t _Size, allocator_arg_t, const _Alloc& _Al, const _Args&...) {
            static_assert(_Has_real_pointers<_Alloc>, "coroutine allocators must use true pointers");
            return _Allocate(_Al, _Size);
        }

        template <class _This, class _Alloc, class... _Args>
        static void* operator new(const size_t _Size, const _This&, allocator_arg_t, const _Alloc& _Al, const _Args&...) {
            static_assert(_Has_real_pointers<_Alloc>, "coroutine allocators must use true pointers");
            return _Allocate(_Al, _Size);
        }

        static void operator delete(void* const _Ptr, const size_t _Size) noexcept {
            _Dealloc_fn _Dealloc;
            _CSTD memcpy(&_Dealloc, static_cast<const char*>(_Ptr) + _Size, sizeof(_Dealloc_fn));
            _Dealloc(_Ptr, _Size);
        }
    };

    namespace ranges {

#ifdef __cpp_lib_byte
        using _Elements_alloc_type = byte;
#else
        using _Elements_alloc_type = char;
#endif

        _EXPORT_STD template <range _Rng, class _Alloc = allocator<_Elements_alloc_type>>
            struct elements_of {
            /* [[no_unique_address]] */ _Rng range;
            /* [[no_unique_address]] */ _Alloc allocator{};
        };

        template <class _Rng, class _Alloc = allocator<_Elements_alloc_type>>
        elements_of(_Rng&&, _Alloc = {}) -> elements_of<_Rng&&, _Alloc>;

    } // namespace ranges

    template <class _Rty, class _Vty = void, class _Alloc = void>
    class generator;
    template <class _Rty, class _Vty>
    using _Gen_value_t = conditional_t<is_void_v<_Vty>, remove_cvref_t<_Rty>, _Vty>;
    template <class _Rty, class _Vty>
    using _Gen_reference_t = conditional_t<is_void_v<_Vty>, _Rty&&, _Rty>;
    template <class _Ref>
    using _Gen_yield_t = conditional_t<is_reference_v<_Ref>, _Ref, const _Ref&>;

    template <class _Yielded>
    class _Gen_promise_base {
    public:
        _STL_INTERNAL_STATIC_ASSERT(is_reference_v<_Yielded>);

#ifndef _PREFAST_ // TRANSITION, FIXME
        _NODISCARD
#endif // _PREFAST_
            suspend_always initial_suspend() noexcept {
            return {};
        }

        _NODISCARD auto final_suspend() noexcept {
            return _Final_awaiter{};
        }

        _NODISCARD suspend_always yield_value(_Yielded _Val) noexcept {
            _Ptr = _STD addressof(_Val);
            return {};
        }

        // clang-format off
        _NODISCARD auto yield_value(const remove_reference_t<_Yielded>& _Val)
            noexcept(is_nothrow_constructible_v<remove_cvref_t<_Yielded>, const remove_reference_t<_Yielded>&>)
            requires (is_rvalue_reference_v<_Yielded>&& constructible_from<remove_cvref_t<_Yielded>, const remove_reference_t<_Yielded>&>) {
            // clang-format on
            return _Element_awaiter{ _Val };
        }

        // clang-format off
        template <class _Rty, class _Vty, class _Alloc, class _Unused>
            requires same_as<_Gen_yield_t<_Gen_reference_t<_Rty, _Vty>>, _Yielded>
        _NODISCARD auto yield_value(
            _RANGES elements_of<generator<_Rty, _Vty, _Alloc>&&, _Unused> _Elem) noexcept {
            // clang-format on
            return _Nested_awaitable<_Rty, _Vty, _Alloc>{std::move(_Elem.range)};
        }

        // clang-format off
        template <_RANGES input_range _Rng, class _Alloc>
            requires convertible_to<_RANGES range_reference_t<_Rng>, _Yielded>
        _NODISCARD auto yield_value(_RANGES elements_of<_Rng, _Alloc> _Elem) noexcept {
            // clang-format on
            using _Vty = _RANGES range_value_t<_Rng>;
            return _Nested_awaitable<_Yielded, _Vty, _Alloc>{
                [](allocator_arg_t, _Alloc, _RANGES iterator_t<_Rng> _It,
                    const _RANGES sentinel_t<_Rng> _Se) -> generator<_Yielded, _Vty, _Alloc> {
                        for (; _It != _Se; ++_It) {
                            co_yield static_cast<_Yielded>(*_It);
                        }
                }(allocator_arg, _Elem.allocator, _RANGES begin(_Elem.range), _RANGES end(_Elem.range))};
        }

        void await_transform() = delete;

        void return_void() noexcept {}

        void unhandled_exception() {
            if (_Info) {
                _Info->_Except = _STD current_exception();
            }
            else {
                throw;
            }
        }

    private:
        struct _Element_awaiter {
            remove_cvref_t<_Yielded> _Val;

            _NODISCARD constexpr bool await_ready() const noexcept {
                return false;
            }

            template <class _Promise>
            constexpr void await_suspend(coroutine_handle<_Promise> _Handle) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible // TRANSITION, LLVM-48860
                _STL_INTERNAL_STATIC_ASSERT(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible

                _Gen_promise_base& _Current = _Handle.promise();
                _Current._Ptr = _STD addressof(_Val);
            }

            constexpr void await_resume() const noexcept {}
        };

        struct _Nest_info {
            exception_ptr _Except;
            coroutine_handle<_Gen_promise_base> _Parent;
            coroutine_handle<_Gen_promise_base> _Root;
        };

        struct _Final_awaiter {
            _NODISCARD bool await_ready() noexcept {
                return false;
            }

            template <class _Promise>
            _NODISCARD coroutine_handle<> await_suspend(coroutine_handle<_Promise> _Handle) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible // TRANSITION, LLVM-48860
                _STL_INTERNAL_STATIC_ASSERT(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible

                _Gen_promise_base& _Current = _Handle.promise();
                if (!_Current._Info) {
                    return _STD noop_coroutine();
                }

                coroutine_handle<_Gen_promise_base> _Cont = _Current._Info->_Parent;
                _Current._Info->_Root.promise()._Top = _Cont;
                _Current._Info = nullptr;
                return _Cont;
            }

            void await_resume() noexcept {}
        };

        template <class _Rty, class _Vty, class _Alloc>
        struct _Nested_awaitable {
            _STL_INTERNAL_STATIC_ASSERT(same_as<_Gen_yield_t<_Gen_reference_t<_Rty, _Vty>>, _Yielded>);

            _Nest_info _Nested;
            generator<_Rty, _Vty, _Alloc> _Gen;

            explicit _Nested_awaitable(generator<_Rty, _Vty, _Alloc>&& _Gen_) noexcept : _Gen(_STD move(_Gen_)) {}

            _NODISCARD bool await_ready() noexcept {
                return !_Gen._Coro;
            }

            template <class _Promise>
            _NODISCARD coroutine_handle<_Gen_promise_base> await_suspend(coroutine_handle<_Promise> _Current) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible // TRANSITION, LLVM-48860
                _STL_INTERNAL_STATIC_ASSERT(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible
                auto _Target = coroutine_handle<_Gen_promise_base>::from_address(_Gen._Coro.address());
                _Nested._Parent = coroutine_handle<_Gen_promise_base>::from_address(_Current.address());
                _Gen_promise_base& _Parent_promise = _Nested._Parent.promise();
                if (_Parent_promise._Info) {
                    _Nested._Root = _Parent_promise._Info->_Root;
                }
                else {
                    _Nested._Root = _Nested._Parent;
                }
                _Nested._Root.promise()._Top = _Target;
                _Target.promise()._Info = _STD addressof(_Nested);
                return _Target;
            }

            void await_resume() {
                if (_Nested._Except) {
                    _STD rethrow_exception(_STD move(_Nested._Except));
                }
            }
        };

        template <class, class>
        friend class _Gen_iter;

        // _Top and _Info are mutually exclusive, and could potentially be merged.
        coroutine_handle<_Gen_promise_base> _Top = coroutine_handle<_Gen_promise_base>::from_promise(*this);
        add_pointer_t<_Yielded> _Ptr = nullptr;
        _Nest_info* _Info = nullptr;
    };

    struct _Gen_secret_tag {};

    template <class _Value, class _Ref>
    class _Gen_iter {
    public:
        using value_type = _Value;
        using difference_type = ptrdiff_t;

        _Gen_iter(_Gen_iter&& _That) noexcept : _Coro{ _STD exchange(_That._Coro, {}) } {}

        _Gen_iter& operator=(_Gen_iter&& _That) noexcept {
            _Coro = _STD exchange(_That._Coro, {});
            return *this;
        }

        _NODISCARD _Ref operator*() const noexcept {
            _STL_ASSERT(!_Coro.done(), "Can't dereference generator end iterator");
            return static_cast<_Ref>(*_Coro.promise()._Top.promise()._Ptr);
        }

        _Gen_iter& operator++() {
            _STL_ASSERT(!_Coro.done(), "Can't increment generator end iterator");
            _Coro.promise()._Top.resume();
            return *this;
        }

        void operator++(int) {
            ++* this;
        }

        _NODISCARD bool operator==(default_sentinel_t) const noexcept {
            return _Coro.done();
        }

    private:
        template <class, class, class>
        friend class generator;

        explicit _Gen_iter(_Gen_secret_tag, coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>> _Coro_) noexcept
            : _Coro{ _Coro_ } {}

        coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>> _Coro;
    };

    _EXPORT_STD template <class _Rty, class _Vty, class _Alloc>
        class generator : public ranges::view_interface<generator<_Rty, _Vty, _Alloc>> {
        private:
            using _Value = _Gen_value_t<_Rty, _Vty>;
            static_assert(same_as<remove_cvref_t<_Value>, _Value>&& is_object_v<_Value>,
                "generator's value type must be a cv-unqualified object type");

            // clang-format off
            using _Ref = _Gen_reference_t<_Rty, _Vty>;
            static_assert(is_reference_v<_Ref>
                || (is_object_v<_Ref> && same_as<remove_cv_t<_Ref>, _Ref> && copy_constructible<_Ref>),
                "generator's second argument must be a reference type or a cv-unqualified "
                "copy-constructible object type");

            using _RRef = conditional_t<is_lvalue_reference_v<_Ref>, remove_reference_t<_Ref>&&, _Ref>;

            static_assert(common_reference_with<_Ref&&, _Value&>&& common_reference_with<_Ref&&, _RRef&&>
                && common_reference_with<_RRef&&, const _Value&>,
                "an iterator with the selected value and reference types cannot model indirectly_readable");
            // clang-format on

            static_assert(_Has_real_pointers<_Alloc>, "generator allocators must use true pointers");

            friend _Gen_promise_base<_Gen_yield_t<_Ref>>;

        public:
            struct __declspec(empty_bases)promise_type : _Promise_allocator<_Alloc>, _Gen_promise_base<_Gen_yield_t<_Ref>> {
                _NODISCARD generator get_return_object() noexcept {
                    return generator{ _Gen_secret_tag{}, coroutine_handle<promise_type>::from_promise(*this) };
                }
            };
            _STL_INTERNAL_STATIC_ASSERT(is_standard_layout_v<promise_type>);
#ifdef __cpp_lib_is_pointer_interconvertible // TRANSITION, LLVM-48860
            _STL_INTERNAL_STATIC_ASSERT(
                is_pointer_interconvertible_base_of_v<_Gen_promise_base<_Gen_yield_t<_Ref>>, promise_type>);
#endif // __cpp_lib_is_pointer_interconvertible

            generator(generator&& _That) noexcept : _Coro(_STD exchange(_That._Coro, {})) {}

            ~generator() {
                if (_Coro) {
                    _Coro.destroy();
                }
            }

            generator& operator=(generator _That) noexcept {
                _STD swap(_Coro, _That._Coro);
                return *this;
            }

            _NODISCARD _Gen_iter<_Value, _Ref> begin() {
                // Pre: _Coro is suspended at its initial suspend point
                _STL_ASSERT(_Coro, "Can't call begin on moved-from generator");
                _Coro.resume();
                return _Gen_iter<_Value, _Ref>{
                    _Gen_secret_tag{}, coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>>::from_address(_Coro.address())};
            }

            _NODISCARD default_sentinel_t end() const noexcept {
                return default_sentinel;
            }

        private:
            coroutine_handle<promise_type> _Coro = nullptr;

            explicit generator(_Gen_secret_tag, coroutine_handle<promise_type> _Coro_) noexcept : _Coro(_Coro_) {}
    };

}

#endif // __has_include(<generator>)