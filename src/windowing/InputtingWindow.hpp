#pragma once

#include "windowing\Windowing.hpp"
#include "windowing\Window.hpp"

namespace Inputting {
	class IKeyboardListener;
	class IMouseListener;
}

namespace Windowing {

	class InputtingWindow final : public Window, IListener
	{
	public:
		InputtingWindow();
		~InputtingWindow();

		bool Init(const WindowSettings &settings, bool trapMouse_);

		void SubscribeOnKeyboardEvents(Inputting::IKeyboardListener *listener);
		void UnSubscribeOnKeyboardEvents(const Inputting::IKeyboardListener *listener);

		void SubscribeOnMouseEvents(Inputting::IMouseListener *listener);
		void UnSubscribeOnMouseEvents(const Inputting::IMouseListener *listener);
	private:
		bool trapMouse = false;
		bool mouseHolded = false;
		std::vector<Inputting::IKeyboardListener*> keyboardListeners;
		std::vector<Inputting::IMouseListener*> mouseListeners;

		virtual void OnWindowFocusLost(const Window &window) override;

		virtual void OnKeyUp(const Window &window, const SDL_Keysym &keysym) override;
		virtual void OnKeyDown(const Window &window, const SDL_Keysym &keysym) override;

		virtual void OnMouseMotion(const Window &window, const Common::vec2 &position, const Common::vec2 &relative) override;
		virtual void OnMouseButtonUp(const Window &window, uint32_t button) override;
		virtual void OnMouseButtonDown(const Window &window, uint32_t button) override;

		void HoldMouse();
		void ReleaseMouse();
	};

}