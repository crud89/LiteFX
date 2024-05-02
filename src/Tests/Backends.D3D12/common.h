#pragma once

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define LITEFX_DEFINE_GLOBAL_EXPORTS
#include <litefx/litefx.h>
#include <litefx/core.h>
#include <litefx/backends/dx12.hpp>
#include <litefx/logging.hpp>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

class TestApp : public LiteFX::App {
public:
	static String Name() noexcept { return "LiteFX D3D12 Backend Test App"; }
	String name() const noexcept override { return Name(); }

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