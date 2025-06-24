//

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#include <cstdio>

// Data structures based on previous state_t and Circle
struct state_t {
    double min_radius;
    double max_radius;
    int min_creation_interval;
    int max_creation_interval;
    int initial_width;
    int initial_height;
    int circle_coeff;
    double decrement;
    int click_counter;
    int get_creation_interval() const {
        static std::mt19937 rng(std::random_device {}());
        // Adjust interval based on click_counter as before.
        std::uniform_int_distribution<int> dist(
            min_creation_interval / (1 + 0.1 * click_counter), max_creation_interval / (1 + 0.1 * click_counter)
        );
        return dist(rng);
    }
};

static state_t state = { 20.0, 80.0, 500, 2000, 1000, 1000, 1, 5.0, 0 };

struct Circle {
    float radius;
    ImVec2 center;
    bool clicked;
};

static std::vector<Circle> circles;
static std::mt19937 rng(std::random_device {}());

void schedule_circle_creation(float win_width, float win_height, double& circle_timer, int& creation_interval) {
    if(circles.size() < static_cast<size_t>(5 + state.circle_coeff * state.click_counter)) {
        std::uniform_real_distribution<float> dist_radius(state.min_radius, state.max_radius);
        float radius = dist_radius(rng);
        std::uniform_real_distribution<float> dist_x(radius, win_width - radius);
        std::uniform_real_distribution<float> dist_y(radius, win_height - radius);
        float x = dist_x(rng);
        float y = dist_y(rng);
        circles.push_back({ radius, ImVec2(x, y), false });
    }
    circle_timer = 0.0;
    creation_interval = state.get_creation_interval();
}

int main(int, char**) {
    // Setup GLFW window
    if(!glfwInit()) return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(state.initial_width, state.initial_height, "kicker (imgui)", NULL, NULL);
    if(window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // Setup Dear ImGui style and backend
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    auto last_time = std::chrono::high_resolution_clock::now();
    double circle_timer = 0.0;
    int creation_interval = state.get_creation_interval();
    static int last_delta = 0;
    static ImVec2 last_click_pos;
    static auto label_time = std::chrono::high_resolution_clock::now();
    static float label_offset = 0.0f;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto current_time = std::chrono::high_resolution_clock::now();
        double delta_time = std::chrono::duration<double, std::milli>(current_time - last_time).count();
        last_time = current_time;
        circle_timer += delta_time;

        int win_width, win_height;
        glfwGetWindowSize(window, &win_width, &win_height);

        // Create circle at random intervals
        if(circle_timer >= creation_interval) {
            schedule_circle_creation((float)win_width, (float)win_height, circle_timer, creation_interval);
        }

        // Handle mouse clicks
        if(ImGui::IsMouseClicked(0)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            bool clicked_any = false;
            int delta = 0;
            for(auto& circle : circles) {
                float dx = mouse_pos.x - circle.center.x;
                float dy = mouse_pos.y - circle.center.y;
                if(std::sqrt(dx * dx + dy * dy) <= circle.radius) {
                    circle.clicked = true;
                    delta++;
                    clicked_any = true;
                }
            }
            if(!clicked_any) {
                delta = -1;
            }
            state.click_counter += delta;
            if(state.click_counter < 0) state.click_counter = 0;
            // Store the delta and click position for display
            last_delta = delta;
            last_click_pos = mouse_pos;
            label_time = current_time;
            label_offset = 0.0f;
            // Remove clicked circles
            circles.erase(
                std::remove_if(circles.begin(), circles.end(), [](const Circle& c) { return c.clicked; }), circles.end()
            );
        }

        // Animate circles: shrink them over time
        float delta_radius = state.decrement * (delta_time / 1000.0f);
        for(auto it = circles.begin(); it != circles.end();) {
            it->radius -= delta_radius;
            if(it->radius <= 0.0f) {
                if(!it->clicked) {
                    state.click_counter--;
                    if(state.click_counter < 0) state.click_counter = 0;
                }
                it = circles.erase(it);
            }
            else {
                ++it;
            }
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a full-screen transparent window for drawing
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin(
            "kicker", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground
        );

        // Draw circles using the window's draw list
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for(const auto& circle : circles) {
            draw_list->AddCircleFilled(circle.center, circle.radius, IM_COL32(255, 0, 0, 255), 32);
        }
        // Show counter change label at mouse click location with fade and float effect
        if(last_delta != 0) {
            double elapsed = std::chrono::duration<double, std::milli>(current_time - label_time).count();
            if(elapsed < 2000) {
                float alpha = static_cast<float>(255 * (1.0 - elapsed / 2000.0));
                ImU32 color = last_delta > 0 ? IM_COL32(0, 255, 0, static_cast<int>(alpha))
                                             : IM_COL32(255, 0, 0, static_cast<int>(alpha));
                char delta_buf[32];
                snprintf(delta_buf, sizeof(delta_buf), "%+d", last_delta);
                // Draw the label with a vertical float-up effect
                draw_list->AddText(ImVec2(last_click_pos.x, last_click_pos.y - label_offset), color, delta_buf);
                // Increase the offset to float up (~20 pixels per second)
                label_offset += 20.0f * (static_cast<float>(delta_time) / 1000.0f);
            }
            else {
                last_delta = 0;
            }
        }
        // Display the click counter value in the top-left corner
        {
            char counter_buf[64];
            snprintf(counter_buf, sizeof(counter_buf), "Click Counter: %d", state.click_counter);
            draw_list->AddText(ImVec2(10, 10), IM_COL32(255, 255, 255, 255), counter_buf);
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
