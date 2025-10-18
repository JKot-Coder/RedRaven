#include "ecs_module/FeatureRegistry.hpp"
#include "ecs_imgui/ImGui.hpp"
#include "ecs/Ecs.hpp"
#include <imgui.h>

namespace
{
    bool show_demo_window = true;
    bool show_another_window = true;
    bool show_im_inspector_window = true;
}

namespace RR::Editor
{
    struct ImGuiDemoWindow
    {
        ECS_SINGLETON;
    };

    struct ImGuiTestWindow
    {
        ECS_SINGLETON;
    };

    struct ImInspectorWindow
    {
        ECS_SINGLETON;
    };


    struct Foo
    {
        int x;
        int y;
    };

    struct Selection
    {
        ECS_SINGLETON;

        bool IsSelected(RR::Ecs::EntityId id) const { return selectedEntities.contains(id); }
        void Select(RR::Ecs::EntityId id)
        {
            selectedEntities.insert(id);
        }
        void SelectSingle(RR::Ecs::EntityId id)
        {
            Clear();
            selectedEntities.insert(id);
        }
        void Unselect(RR::Ecs::EntityId id) { selectedEntities.erase(id); }
        void ToggleSelection(RR::Ecs::EntityId id)
        {
            if (IsSelected(id))
                Unselect(id);
            else
                Select(id);
        }
        void Clear() { selectedEntities.clear(); }

    private:
        absl::flat_hash_set<RR::Ecs::EntityId, Ecs::DummyHasher<RR::Ecs::EntityId>> selectedEntities;
    };

    void InitTestWindow(RR::Ecs::World& world)
    {
        world.Entity().Add<ImGuiTestWindow>().Apply();
        world.Entity().Add<ImInspectorWindow>().Apply();
        world.Entity().Add<ImGuiDemoWindow>().Apply();

        auto query = world.Query().With<std::string>().Build();

        world.RegisterComponent<Foo>().Element("x", &Foo::x).Element("y", &Foo::y);

        world.Entity().Add<Selection>().Apply();

        world.Entity()
            .Add<std::string>("qwe")
            .Add<Foo>()
            .Apply();

        world.Entity()
            .Add<std::string>("qwe2")
            .Add<Foo>()
            .Apply();

        world.System().OnEvent<ImGuiEcs::DrawEvent>().With<ImInspectorWindow>().ForEach([query](RR::Ecs::World& world) {
            ImGui::Begin("Inspector", &show_im_inspector_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            Selection* selection = nullptr;
            world.View().With<Selection>().ForEach([&selection](Selection* selection_) { selection = selection_; });
            ASSERT(selection);

            query.ForEach([selection](RR::Ecs::EntityId id, std::string& name) {
                ImGuiIO& io = ImGui::GetIO();

                bool selected = selection->IsSelected(id);

                ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
                flag |= selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
                flag |= ImGuiTreeNodeFlags_Leaf;

                if (ImGui::TreeNodeEx(name.c_str(), flag))
                {
                    if (ImGui::IsItemClicked())
                    {
                        if (io.KeyCtrl)
                        {
                            selection->ToggleSelection(id);
                        }
                        else
                        selection->SelectSingle(id);
                    }
                    ImGui::TreePop();
                }
            });

            ImGui::End();
        });

        world.System().OnEvent<ImGuiEcs::EarlyDrawEvent>().With<ImGuiTestWindow>().ForEach([]() {
           static bool opt_fullscreen = true;
            static bool opt_padding = false;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
               ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
                window_flags |= ImGuiWindowFlags_NoBackground;
                window_flags |= ImGuiWindowFlags_NoNav;
                window_flags |= ImGuiWindowFlags_NoInputs;
                window_flags |= ImGuiWindowFlags_NoDecoration;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
            // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
            // all active windows docked into it will lose their parent and become undocked.
            // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
            // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            bool p_open = true;
            ImGui::Begin("DockSpace Demo", &p_open, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
               ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            else
            {
                ASSERT(false);
            }


            ImGui::End();



        });

        world.System().OnEvent<ImGuiEcs::DrawEvent>().With<ImGuiDemoWindow>().ForEach([]() {
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

                if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open")) { /* ... */ }
                    if (ImGui::MenuItem("Exit")) { /* ... */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    ImGui::MenuItem("Undo");
                    ImGui::MenuItem("Redo");
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        });
    }
    ECS_REGISTER_FEATURE(InitTestWindow);
}