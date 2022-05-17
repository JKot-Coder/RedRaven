#pragma once

#include "platform/Window.hpp"

namespace RR
{
    bool ImGui_ImplRR_InitForOpenGL(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks);
    bool ImGui_ImplRR_InitForVulkan(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks);
    bool ImGui_ImplRR_InitForOther(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks);
    void ImGui_ImplGlfw_Shutdown();
    void ImGui_ImplGlfw_NewFrame();
}