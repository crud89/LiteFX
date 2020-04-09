#pragma once

#if !defined (LITEFX_APPMODEL_API)
#  if defined(LiteFX_AppModel_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_APPMODEL_API __declspec(dllexport)
#  elif (defined(LiteFX_AppModel_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_APPMODEL_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_AppModel_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_APPMODEL_API __declspec(dllimport)
#  endif
#endif

#ifndef LITEFX_APPMODEL_API
#  define LITEFX_APPMODEL_API
#endif

#include <litefx/core.h>

namespace LiteFX {

	enum class LITEFX_APPMODEL_API Platform {
		None = 0x00000000,
		Win32 = 0x00000001,
		// TODO: ...
		Other = 0x7FFFFFFF
	};

	/**
	*
	**/
	class LITEFX_APPMODEL_API AppVersion {
		LITEFX_IMPLEMENTATION(AppVersionImpl)

	public:
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0) noexcept;
		AppVersion(const AppVersion&) = delete;
		AppVersion(AppVersion&&) = delete;
		virtual ~AppVersion() noexcept;

	public:
		int getMajor() const noexcept;
		int getMinor() const noexcept;
		int getPatch() const noexcept;
		int getRevision() const noexcept;
		int getEngineMajor() const noexcept;
		int getEngineMinor() const noexcept;
		int getEngineRevision() const noexcept;
		int getEngineStatus() const noexcept;
		String getEngineIdentifier() const noexcept;
		String getEngineVersion() const noexcept;
	};

	/**
	* Base class for a LiteFX application.
	**/
	class LITEFX_APPMODEL_API App {
		LITEFX_IMPLEMENTATION(AppImpl)

	public:
		App(const Platform& platform) noexcept;
		App(const App&) = delete;
		App(App&&) = delete;
		virtual ~App() noexcept;

	public:
		virtual String getName() const noexcept = 0;
		virtual AppVersion getVersion() const noexcept = 0;
		virtual Platform getPlatform() const noexcept;

	public:
		virtual int start(const int argc, const char** argv);
		virtual int start(const Array<String>& args) = 0;
		virtual void stop() = 0;
		virtual void work() = 0;
	};

}