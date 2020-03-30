#include "litefx/core_types.hpp"

using namespace LiteFX;

CLiteFxApp::CLiteFxApp() :
	m_name("LiteFX Application")
{

}

CLiteFxApp::~CLiteFxApp()
{

}

const String& CLiteFxApp::getName() const
{
	return m_name;
}

void CLiteFxApp::setName(const String& name)
{
	m_name = name;
}