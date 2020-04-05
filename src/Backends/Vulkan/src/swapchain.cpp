//
//if (format == Format::Other || format == Format::None)
//	throw std::invalid_argument("The provided surface format it not a valid value.");
//
//// Query swap chain format.
//auto surfaceFormats = this->getSurfaceFormats(parent, surface);
//auto selectedFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [format](const Format& surfaceFormat) { return surfaceFormat == format; });
//
//if (selectedFormat == surfaceFormats.end())
//throw std::runtime_error("The adapter does not provide a graphics device.");
//
//// Set the present mode to VK_PRESENT_MODE_FIFO_KHR for now, which is always available.
//// TODO: Change present mode:
//// -VK_PRESENT_MODE_IMMEDIATE_KHR: to disable VSync
//// -VK_PRESENT_MODE_MAILBOX_KHR: to enable triple buffering
//VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
//
//if (capabilities.currentExtent.width != UINT32_MAX) {
//    return capabilities.currentExtent;
//}
//else {
//    VkExtent2D actualExtent = { WIDTH, HEIGHT };
//
//    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
//    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
//
//    return actualExtent;
//}