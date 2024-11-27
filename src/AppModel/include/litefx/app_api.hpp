#pragma once

#include <litefx/config.h>

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
#include <litefx/logging.hpp>

namespace LiteFX {

	class IBackend;
	class AppVersion;
	class App;
	class AppBuilder;

	enum class Platform {
		None = 0x00000000,
		Win32 = 0x00000001,
		Other = 0x7FFFFFFF
	};

	enum class BackendType {
		Rendering = 0x01,
		Other = 0xFF
	};

	constexpr std::array VALID_BACKEND_TYPES { BackendType::Rendering };

	enum class BackendState {
		Inactive = 0x01,
		Active = 0x02
	};

	/// <summary>
	/// Contains the version of an <see cref="App" />.
	/// </summary>
	class LITEFX_APPMODEL_API AppVersion {
		LITEFX_IMPLEMENTATION(AppVersionImpl);

	public:
		/// <summary>
		/// Creates a new app version instance.
		/// </summary>
		/// <param name="major">The major version of the app.</param>
		/// <param name="minor">The minor version of the app.</param>
		/// <param name="patch">The patch number of the app.</param>
		/// <param name="revision">The revision of the app.</param>
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0) noexcept;
		virtual ~AppVersion() noexcept;

		AppVersion(const AppVersion& _other) = delete;
		AppVersion(AppVersion&& _other) = delete;
		AppVersion& operator=(const AppVersion&) = delete;
		AppVersion& operator=(AppVersion&&) = delete;

	public:
		/// <summary>
		/// Gets the major version of the app.
		/// </summary>
		/// <returns>The major version of the app.</returns>
		int major() const noexcept;

		/// <summary>
		/// Gets the minor version of the app.
		/// </summary>
		/// <returns>The minor version of the app.</returns>
		int minor() const noexcept;

		/// <summary>
		/// Gets the patch number of the app.
		/// </summary>
		/// <returns>The patch number of the app.</returns>
		int patch() const noexcept;

		/// <summary>
		/// Gets the revision of the app.
		/// </summary>
		/// <returns>The revision of the app.</returns>
		int revision() const noexcept;

		/// <summary>
		/// Gets the major version of the engine build.
		/// </summary>
		/// <returns>The major version of the engine build.</returns>
		int engineMajor() const noexcept;

		/// <summary>
		/// Gets the minor version of the engine build.
		/// </summary>
		/// <returns>The minor version of the engine build.</returns>
		int engineMinor() const noexcept;

		/// <summary>
		/// Gets the revision of the engine build.
		/// </summary>
		/// <returns>The revision of the engine build.</returns>
		int engineRevision() const noexcept;

		/// <summary>
		/// Gets the status of the engine build.
		/// </summary>
		/// <returns>The status of the engine build.</returns>
		int engineStatus() const noexcept;

		/// <summary>
		/// Gets the identifier of the engine build.
		/// </summary>
		/// <returns>The identifier of the engine build.</returns>
		StringView engineIdentifier() const noexcept;

		/// <summary>
		/// Gets the version string of the engine build.
		/// </summary>
		/// <returns>The version string of the engine build.</returns>
		StringView engineVersion() const noexcept;
	};

}