namespace Render
{
	namespace Device
	{
		class SingleThreadDeviceInterface
		{
			virtual void Init() = 0;
		};

		class MultiThreadDeviceInterface
		{
		};

		class Device : public SingleThreadDeviceInterface, public MultiThreadDeviceInterface
		{
		public:
			virtual ~Device() = default;
		};

	}
}

