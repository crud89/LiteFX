#include <litefx/app.hpp>

using namespace LiteFX;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class AppVersion::AppVersionImpl : public Implement<AppVersion> {
public:
	friend class AppVersion;

private:
	int m_major, m_minor, m_patch, m_revision;

public:
	AppVersionImpl(AppVersion* parent, int major, int minor, int patch, int revision) : 
		base(parent), m_major(major), m_minor(minor), m_patch(patch), m_revision(revision) 
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

AppVersion::AppVersion(int major, int minor, int patch, int revision) noexcept :
	m_impl(makePimpl<AppVersionImpl>(this, major, minor, patch, revision))
{
}

AppVersion::~AppVersion() noexcept = default;

int AppVersion::getMajor() const noexcept
{
	return m_impl->m_major;
}

int AppVersion::getMinor() const noexcept
{
	return m_impl->m_minor;
}

int AppVersion::getPatch() const noexcept
{
	return m_impl->m_patch;
}

int AppVersion::getRevision() const noexcept
{
	return m_impl->m_revision;
}

int AppVersion::getEngineMajor() const noexcept
{
	return LITEFX_MAJOR;
}

int AppVersion::getEngineMinor() const noexcept
{
	return LITEFX_MINOR;
}

int AppVersion::getEngineRevision() const noexcept
{
	return LITEFX_REV;
}

int AppVersion::getEngineStatus() const noexcept
{
	return LITEFX_STATUS;
}

String AppVersion::getEngineIdentifier() const noexcept
{
	return LITEFX_ENGINE_ID;
}

String AppVersion::getEngineVersion() const noexcept
{
	return LITEFX_VERSION;
}