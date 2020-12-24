#pragma once

#include <litefx/core.h>
#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering.hpp>

#if defined(BUILD_VULKAN_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)
#  include <litefx/backends/vulkan.hpp>
#endif // defined(BUILD_VULKAN_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)

#if defined(BUILD_DIRECTX_12_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)
#  include <litefx/backends/dx12.hpp>
#endif // defined(BUILD_DIRECTX_12_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)

#if defined(LITEFX_AUTO_IMPORT_NAMESPACE)
using namespace LiteFX;
#endif // defined(LITEFX_AUTO_IMPORT_NAMESPACE)