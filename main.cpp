#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gdkmm/event.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <cmath>

class MyWindow : public Gtk::Window {
public:
    MyWindow() : m_circle_radius(50.0), m_decrement(5.0) {
        add_events(Gdk::BUTTON_PRESS_MASK);
        m_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyWindow::on_timeout), 1000);
    }

protected:
    bool on_button_press_event(GdkEventButton* event) override {
        std::cout << "Mouse click detected at ("
                  << event->x << ", " << event->y << ")" << std::endl;
        return Gtk::Window::on_button_press_event(event);
    }
    
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        int width = get_allocated_width();
        int height = get_allocated_height();
        double cx = width / 2.0;
        double cy = height / 2.0;
        cr->set_source_rgb(1, 0, 0);
        cr->arc(cx, cy, m_circle_radius, 0, 2 * M_PI);
        cr->fill();
        return Gtk::Window::on_draw(cr);
    }

    bool on_timeout() {
        m_circle_radius -= m_decrement;
        if (m_circle_radius <= 0) {
            m_circle_radius = 0;
            m_connection.disconnect();
        }
        queue_draw();
        return (m_circle_radius > 0);
    }

private:
    double m_circle_radius;
    const double m_decrement;
    sigc::connection m_connection;
};

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
    MyWindow window;
    window.set_default_size(200,200);
    window.set_resizable(false);
    return app->run(window);
}
