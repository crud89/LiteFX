#include <litefx/core_types.hpp>

using namespace LiteFX;

AppVersion::AppVersion(int major, int minor, int patch, int revision) :
	m_major(major), m_minor(minor), m_patch(patch), m_revision(revision)
{
}

int AppVersion::getMajor() const
{
	return m_major;
}

int AppVersion::getMinor() const
{
	return m_minor;
}

int AppVersion::getPatch() const
{
	return m_patch;
}

int AppVersion::getRevision() const
{
	return m_revision;
}