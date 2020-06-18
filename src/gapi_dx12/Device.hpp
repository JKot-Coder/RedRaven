#include "gapi/Device.hpp"

#include <d3d12.h>
#include <dxgi1_4.h>

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

				void Init() override;
			};
		}
	}
}