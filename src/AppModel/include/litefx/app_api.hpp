#pragma once

#include <litefx/config.h>
#include <litefx/core.h>
#include <litefx/logging.hpp>

#include <litefx/app_export.h>

namespace LiteFX {

	class IBackend;
	class AppVersion;
	class App;
	class AppBuilder;

	enum class Platform : std::uint8_t {
		None = 0x00,
		Win32 = 0x01,
		Other = 0xFF
	};

	enum class BackendType : std::uint8_t {
		Rendering = 0x01,
		Other = 0xFF
	};

	constexpr std::array VALID_BACKEND_TYPES { BackendType::Rendering };

	enum class BackendState : std::uint8_t {
		Inactive = 0x01,
		Active = 0x02
	};

	/// @brief Contains the version of an @ref App.
	class LITEFX_APPMODEL_API AppVersion {
		LITEFX_IMPLEMENTATION(AppVersionImpl);

	public:
		/// @brief Creates a new app version instance.
		///
		/// @param major The major version of the app.
		/// @param minor The minor version of the app.
		/// @param patch The patch number of the app.
		/// @param revision The revision of the app.
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0) noexcept;
		virtual ~AppVersion() noexcept;

		AppVersion(const AppVersion& _other) = delete;
		AppVersion(AppVersion&& _other) noexcept = delete;
		AppVersion& operator=(const AppVersion&) = delete;
		AppVersion& operator=(AppVersion&&) noexcept = delete;

	public:
		/// @brief Gets the major version of the app.
		///
		/// @return The major version of the app.
		int major() const noexcept;

		/// @brief Gets the minor version of the app.
		///
		/// @return The minor version of the app.
		int minor() const noexcept;

		/// @brief Gets the patch number of the app.
		///
		/// @return The patch number of the app.
		int patch() const noexcept;

		/// @brief Gets the revision of the app.
		///
		/// @return The revision of the app.
		int revision() const noexcept;

		/// @brief Gets the major version of the engine build.
		///
		/// @return The major version of the engine build.
		int engineMajor() const noexcept;

		/// @brief Gets the minor version of the engine build.
		///
		/// @return The minor version of the engine build.
		int engineMinor() const noexcept;

		/// @brief Gets the revision of the engine build.
		///
		/// @return The revision of the engine build.
		int engineRevision() const noexcept;

		/// @brief Gets the status of the engine build.
		///
		/// @return The status of the engine build.
		int engineStatus() const noexcept;

		/// @brief Gets the identifier of the engine build.
		///
		/// @return The identifier of the engine build.
		StringView engineIdentifier() const noexcept;

		/// @brief Gets the version string of the engine build.
		///
		/// @return The version string of the engine build.
		StringView engineVersion() const noexcept;
	};

}