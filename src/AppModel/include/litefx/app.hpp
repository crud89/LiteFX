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
	
	template <typename TBackend, typename TApp>
	class BackendInitializer;

	template <typename TApp>
	class AppBuilder;

	class IBackend;
	class AppVersion;
	class App;

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

	template <typename TApp>
	class AppBuilder : public IBuilder<TApp> {
	private:
		UniquePtr<TApp> m_app;

	public:
		explicit AppBuilder(const Platform& platform) noexcept :
			m_app(makeUnique<TApp>(platform)) { }
		AppBuilder(const AppBuilder&& _other) = delete;
		AppBuilder(AppBuilder&) = delete;
		virtual ~AppBuilder() noexcept = default;

	public:
		virtual const App& getApp() const noexcept { return *m_app.get(); }
		virtual const IBackend* findBackend(const BackendType& type) const noexcept { return m_app->operator[](type); }
		virtual UniquePtr<TApp> go() override { return std::move(m_app); }
		
		void use(UniquePtr<IBackend>&& backend) { m_app->useBackend(std::move(backend)); }

	public:
		template<typename TBackend, typename ...TArgs>
		Initializer<TBackend, AppBuilder<TApp>> backend(TArgs&&... _args) {
			return Initializer<TBackend, AppBuilder<TApp>>(*this, makeUnique<TBackend>(this->getApp(), std::forward<TArgs>(_args)...));
		}
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
		virtual const Platform& getPlatform() const noexcept;
		virtual const IBackend* findBackend(const BackendType& type) const;
		virtual const IBackend* operator[](const BackendType& type) const;

	public:
		virtual void useBackend(UniquePtr<IBackend>&& backend);
		virtual void run() = 0;

	public:
		template <typename TApp>
		static AppBuilder<TApp> build() {

			const Platform& platform =
#if defined(_WIN32) || defined(WINCE)
				Platform::Win32;
#else
				Platform::Other;
#endif

			return AppBuilder<TApp>(platform);
		}
	};

}