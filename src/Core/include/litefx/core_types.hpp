#pragma once

#include <litefx/core.h>

namespace LiteFX {

	template <class THandle>
	class IResource {
	private:
		THandle m_handle;

	protected:
		IResource(const THandle handle) : m_handle(handle) { }

	public:
		IResource(const IResource&) = delete;
		IResource(IResource&&) = delete;
		virtual ~IResource() = default;

	protected:
		THandle handle() const { return m_handle; }
		THandle& handle() { return m_handle; }

	public:
		virtual const Handle getHandle() const { return reinterpret_cast<Handle>(m_handle); }
	};

	/**
	* 
	**/
	class LITEFX_CORE_API AppVersion {
	private:
		int m_major, m_minor, m_patch, m_revision;

	public:
		explicit AppVersion(int major = 1, int minor = 0, int patch = 0, int revision = 0);
		AppVersion(const AppVersion&) = delete;
		AppVersion(AppVersion&&) = delete;
		virtual ~AppVersion() = default;

	public:
		int getMajor() const;
		int getMinor() const;
		int getPatch() const;
		int getRevision() const;
		int getEngineMajor() const;
		int getEngineMinor() const;
		int getEngineRevision() const;
		int getEngineStatus() const;
		String getEngineIdentifier() const;
		String getEngineVersion() const;
	};

	/**
	* Base class for a LiteFX application.
	**/
	class LITEFX_CORE_API App {
	public:
		App();
		App(const App&) = delete;
		App(App&&) = delete;
		virtual ~App();

	public:
		virtual String getName() const = 0;
		virtual AppVersion getVersion() const = 0;

	public:
		virtual int start(int argc, char** argv);
		virtual int start(const Array<String>& args);
		virtual void stop();
		virtual void work() = 0;
	};

}