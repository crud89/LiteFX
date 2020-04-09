#include <litefx/app.hpp>

using namespace LiteFX;

class AppVersion::AppVersionImpl {
private:
	int m_major, m_minor, m_patch, m_revision;

public:
	AppVersionImpl::AppVersionImpl(int major, int minor, int patch, int revision) noexcept :
		m_major(major), m_minor(minor), m_patch(patch), m_revision(revision) { }

public:
	int getMajor() const noexcept
	{
		return m_major;
	}

	int getMinor() const noexcept
	{
		return m_minor;
	}

	int getPatch() const noexcept
	{
		return m_patch;
	}

	int getRevision() const noexcept
	{
		return m_revision;
	}
};

AppVersion::AppVersion(int major, int minor, int patch, int revision) noexcept :
	m_impl(makePimpl<AppVersionImpl>(major, minor, patch, revision))
{
}

AppVersion::~AppVersion() noexcept = default;

int AppVersion::getMajor() const noexcept
{
	return m_impl->getMajor();
}

int AppVersion::getMinor() const noexcept
{
	return m_impl->getMinor();
}

int AppVersion::getPatch() const noexcept
{
	return m_impl->getPatch();
}

int AppVersion::getRevision() const noexcept
{
	return m_impl->getRevision();
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