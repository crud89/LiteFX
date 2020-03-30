#pragma once

#include <litefx/core.h>

namespace LiteFX {
	
	/**
	* Base class for a LiteFX application.
	**/
	class LITEFX_CORE_API CLiteFxApp {
	private:
		String m_name;

	public:
		CLiteFxApp();
		CLiteFxApp(const String& name);
		CLiteFxApp(const CLiteFxApp&) = delete;
		CLiteFxApp(CLiteFxApp&&) = delete;
		virtual ~CLiteFxApp();

	public:
		virtual const String& getName() const;

	protected:
		virtual void setName(const String& name);

	public:
		virtual void start(int argc, char** argv);
		virtual void start(const Array<String>& args) = 0;
		virtual void stop();
	};

}