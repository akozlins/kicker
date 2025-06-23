#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gdkmm/event.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

struct config_t {
    double min_radius;
    double max_radius;
};

static const config_t config = {20.0, 80.0};

class MyWindow : public Gtk::Window {
public:
    MyWindow() : m_decrement(5.0) {
        set_app_paintable(true);
        add_events(Gdk::BUTTON_PRESS_MASK);
        const int win_width = 200;
        const int win_height = 200;
        // Compute random starting radius within config limits
        m_circle_radius = config.min_radius + (config.max_radius - config.min_radius) * (static_cast<double>(rand())/RAND_MAX);
        // Ensure the circle appears fully inside the window area.
        m_circle_center_x = m_circle_radius + (win_width - 2*m_circle_radius) * (static_cast<double>(rand())/RAND_MAX);
        m_circle_center_y = m_circle_radius + (win_height - 2*m_circle_radius) * (static_cast<double>(rand())/RAND_MAX);
        m_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyWindow::on_timeout), 33);
    }

protected:
    bool on_button_press_event(GdkEventButton* event) override {
        std::cout << "Mouse click detected at ("
                  << event->x << ", " << event->y << ")" << std::endl;
        return Gtk::Window::on_button_press_event(event);
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        cr->set_source_rgb(1, 0, 0);
        cr->arc(m_circle_center_x, m_circle_center_y, m_circle_radius, 0, 2 * M_PI);
        cr->fill();
        return true;
    }

    bool on_timeout() {
        double delta = m_decrement * 33 / 1000.0;
        m_circle_radius -= delta;
        if (m_circle_radius <= 0) {
            m_circle_radius = 0;
            m_connection.disconnect();
        }
        queue_draw();
        return (m_circle_radius > 0);
    }

private:
    double m_circle_radius;
    double m_circle_center_x;
    double m_circle_center_y;
    const double m_decrement;
    sigc::connection m_connection;
};

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
    MyWindow window;
    window.set_default_size(200,200);
    window.set_resizable(false);
    return app->run(window);
}
