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
		CLiteFxApp(const CLiteFxApp&) = delete;
		CLiteFxApp(CLiteFxApp&&) = delete;
		virtual ~CLiteFxApp();

	public:
		const String& getName() const;
		void setName(const String& name);
	};

}