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
#include <iostream>

namespace LiteFX {
	class IBackend;
	class AppVersion;
	class App;
	class AppBuilder;

	enum class LITEFX_APPMODEL_API Platform {
		None = 0x00000000,
		Win32 = 0x00000001,
		// TODO: ...
		Other = 0x7FFFFFFF
	};

	enum class LITEFX_APPMODEL_API BackendType {
		Rendering = 0x01,
		//Physics = 0x02,
		Other = 0xFF
	};

	class IBackend {
	public:
		virtual ~IBackend() noexcept = default;

	public:
		virtual BackendType getType() const noexcept = 0;
	};

	class LITEFX_APPMODEL_API App {
		LITEFX_IMPLEMENTATION(AppImpl)

	public:
		App();
		App(const App&) = delete;
		App(App&&) = delete;
		virtual ~App() noexcept;

	public:
		virtual String getName() const noexcept = 0;
		virtual AppVersion getVersion() const noexcept = 0;
		Platform getPlatform() const noexcept;
		virtual const IBackend* findBackend(const BackendType& type) const;
		virtual const IBackend* operator[](const BackendType& type) const;

	public:
		virtual void useBackend(UniquePtr<IBackend>&& backend);
		virtual void run() = 0;

	public:
		template <typename TApp>
		static AppBuilder build() { return AppBuilder::make<TApp, AppBuilder>(); }
	};

	class AppBuilder : public Builder<App> {
	public:
		using Builder<App>::Builder;

	public:
		virtual const IBackend* findBackend(const BackendType& type) const noexcept { return this->instance()->operator[](type); }
		void use(UniquePtr<IBackend>&& backend) { this->instance()->useBackend(std::move(backend)); }

	public:
		template <typename TBackend, typename ...TArgs, std::enable_if_t<rtti::has_initializer_v<TBackend>, int> = 0, typename TInitializer = TBackend::initializer>
		TInitializer useBackend(TArgs&&... _args) {
			return TInitializer::makeFor<TBackend, TInitializer>(*this, *this->instance(), std::forward<TArgs>(_args)...);
		}

		template <typename TBackend, typename ...TArgs, std::enable_if_t<!rtti::has_initializer_v<TBackend>, int> = 0, typename TInitializer = Initializer<TBackend, AppBuilder>>
		TInitializer useBackend(TArgs&&... _args) {
			return TInitializer::makeFor<TBackend, TInitializer>(*this, *this->instance(), std::forward<TArgs>(_args)...);
		}
	};

	template <typename TBackend>
	class BackendInitializer : public Initializer<TBackend, AppBuilder> {
	public:
		typedef TBackend backend_type;
		typedef AppBuilder builder_type;

	public:
		using Initializer<TBackend, AppBuilder>::Initializer;
	};

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

}