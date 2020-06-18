#include "D3DUtils.hpp"

#include <d3d12.h>
#include <dxgi1_4.h>

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                namespace D3DUtils
                {

                    void CreateDevice(const ComSharedPtr<IDXGIFactory1>& dxgiFactory)
                    {
                    }

                    void GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, D3D_FEATURE_LEVEL minimumFeatureLevel, ComSharedPtr<IDXGIAdapter1>& adapter)
                    {

                        for (uint32_t adapterIndex = 0;; ++adapterIndex)
                        {
                            if (FAILED(dxgiFactory->EnumAdapters1(adapterIndex, adapter.put())))
                                break;

                            DXGI_ADAPTER_DESC1 desc;
                            if (FAILED(adapter->GetDesc1(&desc)))
                                break;

                            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                            {
                                // Don't select the software adapter.
                                continue;
                            }

                            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                            if (SUCCEEDED(D3D12CreateDevice(adapter.get(), minimumFeatureLevel, _uuidof(ID3D12Device), nullptr)))
                            {
#ifdef _DEBUG
                                wchar_t buff[256] = {};
                                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                                OutputDebugStringW(buff);
#endif
                                break;
                            }
                        }
                    }

                }
            }
        }
    }
}