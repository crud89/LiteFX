#pragma once

#include <ranges>
#include <coroutine>
#include <litefx/core.h>

import LiteFX.Core;

using namespace LiteFX;

class Base {
public:
    virtual ~Base() noexcept = default;

    virtual int index() const noexcept = 0;
};

class Foo : public Base {
private:
    int _i;

public:
    Foo(int i) : _i(i) { }
    virtual ~Foo() noexcept = default;

    virtual int index() const noexcept override {
        return _i;
    }
};

class Bar : public Base {
private:
    int _i;

public:
    Bar(int i) : _i(i) { }
    virtual ~Bar() noexcept = default;

    virtual int index() const noexcept override {
        return _i;
    }
};

struct Test {
    int index;
    String name;
};