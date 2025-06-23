#include <gtkmm/application.h>
#include <gtkmm/window.h>

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
    Gtk::Window window;
    window.set_default_size(200,200);
    window.set_resizable(false);
    return app->run(window);
}
