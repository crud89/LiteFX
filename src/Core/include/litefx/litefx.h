#pragma once

#include <litefx/core.h>
#include <litefx/core_types.hpp>
#include <litefx/rendering.hpp>

#if defined(BUILD_OPEN_GL_BACKEND)
#  include <litefx/backends/opengl.hpp>
#endif // defined(BUILD_OPEN_GL_BACKEND)

#if defined(BUILD_VULKAN_BACKEND)
#  include <litefx/backends/vulkan.hpp>
#endif // defined(BUILD_VULKAN_BACKEND)

#if defined(LITEFX_AUTO_IMPORT_NAMESPACE)
using namespace LiteFX;
#endif // defined(LITEFX_AUTO_IMPORT_NAMESPACE)
