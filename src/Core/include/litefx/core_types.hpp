#pragma once

#include <litefx/core.h>

namespace LiteFX {
	
	/**
	* Base class for a LiteFX application.
	**/
	class LITEFX_CORE_API CLiteFxApp {
	public:
		CLiteFxApp();
		CLiteFxApp(const CLiteFxApp&) = delete;
		CLiteFxApp(CLiteFxApp&&) = delete;
		virtual ~CLiteFxApp();
	};

}