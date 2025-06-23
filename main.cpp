#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gdkmm/event.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>

struct config_t {
    double min_radius;
    double max_radius;
    int min_creation_interval;
    int max_creation_interval;
    int initial_width;
    int initial_height;
    int circle_coeff;
    int get_creation_interval() const {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(min_creation_interval, max_creation_interval);
        return dist(rng);
    }
};

static config_t state = { 20.0, 80.0, 500, 2000, 1000, 1000, 1 };

class MyWindow : public Gtk::Window {
public:
    MyWindow() : m_decrement(5.0) {
        set_app_paintable(true);
        add_events(Gdk::BUTTON_PRESS_MASK);
        m_animation_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyWindow::on_timeout), 33);
        schedule_circle_creation();
        m_click_counter = 0;
    }

protected:
    bool on_button_press_event(GdkEventButton* event) override {
        double click_x = event->x;
        double click_y = event->y;
        auto it = m_circles.begin();
        while (it != m_circles.end()) {
            double dx = click_x - it->center_x;
            double dy = click_y - it->center_y;
            if (std::sqrt(dx*dx + dy*dy) <= it->radius) {
                it = m_circles.erase(it);
                m_click_counter++;
            } else {
                ++it;
            }
        }
        std::cout << "Click counter: " << m_click_counter << std::endl;
        return Gtk::Window::on_button_press_event(event);
    }

    bool on_circle_creation() {
        const int win_width = get_allocated_width();
        const int win_height = get_allocated_height();
        if(m_circles.size() < static_cast<size_t>(5 + state.circle_coeff * m_click_counter)) {
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist_radius(state.min_radius, state.max_radius);
            double radius = dist_radius(rng);
            std::uniform_real_distribution<double> dist_x(radius, win_width - radius);
            std::uniform_real_distribution<double> dist_y(radius, win_height - radius);
            double center_x = dist_x(rng);
            double center_y = dist_y(rng);
            m_circles.push_back({radius, center_x, center_y});
        }
        schedule_circle_creation();
        return false;
    }

    void schedule_circle_creation() {
        int interval = state.get_creation_interval();
        m_creation_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyWindow::on_circle_creation), interval);
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        for (const auto& circle : m_circles) {
            cr->set_source_rgb(1, 0, 0);
            cr->arc(circle.center_x, circle.center_y, circle.radius, 0, 2 * M_PI);
            cr->fill();
        }
        // Draw counter in top-left corner
        cr->set_source_rgb(0, 1, 0);
        cr->select_font_face("Sans", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_BOLD);
        cr->set_font_size(20);
        {
            std::string counter_text = "Counter: " + std::to_string(m_click_counter);
            cr->move_to(10, 30);
            cr->show_text(counter_text);
        }
        return true;
    }

    bool on_timeout() {
        double delta = m_decrement * 33 / 1000.0;
        for(auto it = m_circles.begin(); it != m_circles.end();) {
            it->radius -= delta;
            if(it->radius <= 0) {
                m_click_counter--;
                if(m_click_counter < 0) {
                    m_click_counter = 0;
                }
                it = m_circles.erase(it);
            } else {
                ++it;
            }
        }
        queue_draw();
        return true;
    }

private:
    struct Circle {
        double radius;
        double center_x;
        double center_y;
    };
    std::vector<Circle> m_circles;
    const double m_decrement;
    sigc::connection m_animation_connection;
    sigc::connection m_creation_connection;
    int m_click_counter;
};

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.kicker");
    MyWindow window;
    window.set_default_size(state.initial_width, state.initial_height);
    window.set_resizable(false);
    return app->run(window);
}
