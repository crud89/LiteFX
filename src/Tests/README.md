# LiteFX Test Suite

Cloud based test runners typically do not feature dedicated graphics hardware, so the tests are falling back to software emulation. For DirectX, [WARP](https://learn.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp) is automatically installed when building with tests enabled. For Vulkan, [Mesa 3D](https://github.com/pal1000/mesa-dist-win) must be installed manually. This is due to permanent changes to the registry, that are required for the Vulkan loader to pickup the Mesa driver. To do this, download and extract a release of Mesa3D. Afterwards create two new registry values:

```registry
DWORD: HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Drivers\<path-to-x64-icd>
DWORD: HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Khronos\Drivers\<path-to-x86-icd>
```

Use the absolute paths to the x64 and x86 ICD *json* files as key names and enable the keys by setting their value to `0` (setting them to `1` will counter-intuitively disable them!). This will allow the Vulkan backend to pick up the driver when enumerating adapters.

If the Vulkan tests are failing during instance creation, due to some layer or ICD files are being invalid, you can follow the registry keys above (instead of *Drivers* also look into *ImplicitLayers*) and disable them accordingly. Note that layers are, however, typically stored in `HKEY_CURRENT_USER`.