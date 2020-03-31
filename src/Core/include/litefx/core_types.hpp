#pragma once

#include <litefx/core.h>

namespace LiteFX {

	class LITEFX_CORE_API IResource
	{
	public:
		virtual const Handle getHandle() const = 0;

	public:
		template <class THandle>
		inline const THandle getHandle() const { return reinterpret_cast<THandle>(this->getHandle()); }
	};

	/**
	* 
	**/
	class LITEFX_CORE_API AppVersion
	{
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
	class LITEFX_CORE_API App 
	{
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