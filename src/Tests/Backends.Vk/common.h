#pragma once

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define LITEFX_DEFINE_GLOBAL_EXPORTS
#include <litefx/litefx.h>
#include <litefx/core.h>
#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#include <litefx/logging.hpp>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

constexpr char TEST_LOG[] = "Tests";

#define LITEFX_TEST_FAIL(msg) { LITEFX_ERROR(TEST_LOG, msg); throw LiteFX::RuntimeException("Test failed."); }

class TestApp : public LiteFX::App {
public:
	static StringView Name() noexcept { return "LiteFX Vulkan Backend Test App"; }
	StringView name() const noexcept override { return Name(); }

	static AppVersion Version() noexcept { return AppVersion(1, 0, 0, 0); }
	AppVersion version() const noexcept override { return Version(); }

public:
	TestApp() : App()
	{
		this->initializing += std::bind(&TestApp::onInit, this);
		this->startup += std::bind(&TestApp::onStartup, this);
		this->resized += std::bind(&TestApp::onResize, this, std::placeholders::_1, std::placeholders::_2);
		this->shutdown += std::bind(&TestApp::onShutdown, this);
	}

private:
	void onInit();
	void onStartup();
	void onShutdown();
	void onResize(const void* sender, ResizeEventArgs e);
};