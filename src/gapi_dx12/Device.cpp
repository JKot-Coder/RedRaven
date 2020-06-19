#include "Device.hpp"

#include "common/Logger.hpp"
#include "gapi_dx12/ComSharedPtr.hpp"
#include <string>
#include <vector>

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {

                Device::Device() { }

                Device::~Device() { }

                void Device::Init() { }
                /*
                ID3D12Device createDevice(IDXGIFactory4* pFactory, D3D_FEATURE_LEVEL requestedFeatureLevel,
                    const std::vector<UUID>& experimentalFeatures)
                {
                    ComSharedPtr<ID2D1Factory1> factory;
                    // Feature levels to try creating devices. Listed in descending order so the highest supported level is used.
                    const static D3D_FEATURE_LEVEL kFeatureLevels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
                        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                        D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };

                    // Find the HW adapter
                    IDXGIAdapter1* pAdapter;
                    ID3D12Device* pDevice;
                    D3D_FEATURE_LEVEL deviceFeatureLevel;

                    auto createMaxFeatureLevel = [&](const D3D_FEATURE_LEVEL* pFeatureLevels, uint32_t featureLevelCount) -> bool {
                        for (uint32_t i = 0; i < featureLevelCount; i++)
                        {
                            if (SUCCEEDED(D3D12CreateDevice(pAdapter, pFeatureLevels[i], IID_PPV_ARGS(&pDevice))))
                            {
                                deviceFeatureLevel = pFeatureLevels[i];
                                return true;
                            }
                        }

                        return false;
                    };

                    uint32_t gpuDeviceId = 0;
                    for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(i, &pAdapter); i++)
                    {
                        DXGI_ADAPTER_DESC1 desc;
                        pAdapter->GetDesc1(&desc);

                        // Skip SW adapters
                        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                            continue;

                        // Skip if vendorId doesn't match requested
                        if (selectedGpuVendorId != 0 && desc.VendorId != selectedGpuVendorId)
                            continue;

                        // Skip to selected device id
                        if (gpuDeviceId++ < selectedGpuDeviceId)
                            continue;

                        if (requestedFeatureLevel == 0)
                            createMaxFeatureLevel(kFeatureLevels, arraysize(kFeatureLevels));
                        else
                            createMaxFeatureLevel(&requestedFeatureLevel, 1);

                        if (pDevice != nullptr)
                        {
                            Log::Info(FMT_STRING("Successfully created device with feature level: {}"), deviceFeatureLevel);
                            return pDevice;
                        }
                    }

                    ASSERT(false, "Could not find a GPU that supports D3D12 device");
                    return nullptr;
                }
                */
            }
        }
    }
}