#include "gapi/Device.hpp"

#include <memory>

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {

                class Device : public Render::Device::Device
                {
                public:
                    Device();
                    ~Device() override;

                    GAPIStatus Init() override;
                    GAPIStatus Reset(const PresentOptions& presentOptions) override;
                    GAPIStatus Present();

                private:
                    bool _enableDebug = true;
                    std::unique_ptr<struct PrivateDeviceData> _privateData;
                    std::unique_ptr<class TemporaryDX12Impl> _impl;

                    GAPIStatus CreateDevice();
                };

            }
        }
    }
}