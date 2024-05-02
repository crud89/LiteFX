#pragma once

#include <litefx/core.h>
#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering.hpp>

#if defined(LITEFX_BUILD_VULKAN_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)
#  include <litefx/backends/vulkan.hpp>
#  include <litefx/backends/vulkan_builders.hpp>
#endif // defined(LITEFX_BUILD_VULKAN_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)

#if defined(LITEFX_BUILD_DIRECTX_12_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)
#  include <litefx/backends/dx12.hpp>
#  include <litefx/backends/dx12_builders.hpp>
#endif // defined(LITEFX_BUILD_DIRECTX_12_BACKEND) && defined (LITEFX_AUTO_IMPORT_BACKEND_HEADERS)

#if defined(LITEFX_AUTO_IMPORT_NAMESPACE)
using namespace LiteFX;
#endif // defined(LITEFX_AUTO_IMPORT_NAMESPACE)