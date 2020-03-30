#pragma once

#include <litefx/litefx.h>

using namespace LiteFX;

class SampleApp : public LiteFX::CLiteFxApp 
{
public:
	SampleApp();

public:
	virtual void start(const Array<String>& args) override;
};