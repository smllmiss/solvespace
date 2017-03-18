//-----------------------------------------------------------------------------
// The GTK-based implementation of platform-dependent GUI functionality.
//
// Copyright 2017 whitequark
//-----------------------------------------------------------------------------
#include <glibmm/main.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/glarea.h>
#include <gtkmm/window.h>
#include <gtkmm/main.h>
#include "solvespace.h"

namespace SolveSpace {
namespace Platform {

//-----------------------------------------------------------------------------
// Implementations of our extensions for stock GTK widgets.
//-----------------------------------------------------------------------------

class GtkMenuItem : public Gtk::CheckMenuItem {
    Platform::MenuItem  *_receiver;
    bool                _has_indicator;

public:
    GtkMenuItem(Platform::MenuItem *receiver) : _receiver(receiver), _has_indicator(false) {
    }

    void set_accel_key(const Gtk::AccelKey &accel_key) {
        Gtk::CheckMenuItem::set_accel_key(accel_key);
    }

    bool has_indicator() const {
        return _has_indicator;
    }

    void set_has_indicator(bool has_indicator) {
        _has_indicator = has_indicator;
    }

protected:
    void on_activate() override {
        if(_receiver->onActivate) {
            _receiver->onActivate();
        }
    }

    void draw_indicator_vfunc(const Cairo::RefPtr<Cairo::Context> &cr) override {
        if(_has_indicator) {
            Gtk::CheckMenuItem::draw_indicator_vfunc(cr);
        }
    }
};

class GtkGLWidget : public Gtk::GLArea {
    Platform::Window    *_receiver;

public:
    GtkGLWidget(Platform::Window *receiver) : _receiver(receiver) {
        set_has_depth_buffer(true);
        set_can_focus(true);
        set_events(Gdk::POINTER_MOTION_MASK |
                   Gdk::BUTTON_PRESS_MASK |
                   Gdk::BUTTON_RELEASE_MASK |
                   Gdk::BUTTON_MOTION_MASK |
                   Gdk::SCROLL_MASK |
                   Gdk::LEAVE_NOTIFY_MASK |
                   Gdk::KEY_PRESS_MASK |
                   Gdk::KEY_RELEASE_MASK);
    }

protected:
    // Work around a bug fixed in GTKMM 3.22:
    // https://mail.gnome.org/archives/gtkmm-list/2016-April/msg00020.html
    Glib::RefPtr<Gdk::GLContext> on_create_context() override {
        return get_window()->create_gl_context();
    }

    bool on_render(const Glib::RefPtr<Gdk::GLContext> &context) override {
        if(_receiver->onRender) {
            _receiver->onRender();
        }
        return true;
    }

    bool process_pointer_event(MouseEvent::Type type, double x, double y,
                               guint state, guint button = 0, int scroll_delta = 0) {
        MouseEvent event = {};
        event.type = type;
        event.x = x;
        event.y = y;
        if(button == 1 || (state & GDK_BUTTON1_MASK) != 0) {
            event.button = MouseEvent::Button::LEFT;
        } else if(button == 2 || (state & GDK_BUTTON2_MASK) != 0) {
            event.button = MouseEvent::Button::MIDDLE;
        } else if(button == 3 || (state & GDK_BUTTON3_MASK) != 0) {
            event.button = MouseEvent::Button::RIGHT;
        }
        if((state & GDK_SHIFT_MASK) != 0) {
            event.shiftDown = true;
        }
        if((state & GDK_CONTROL_MASK) != 0) {
            event.controlDown = true;
        }
        event.scrollDelta = scroll_delta;

        if(_receiver->onMouseEvent) {
            return _receiver->onMouseEvent(event);
        }

        return false;
    }

    bool on_motion_notify_event(GdkEventMotion *event) override {
        return process_pointer_event(MouseEvent::Type::MOTION,
                                     event->x, event->y, event->state);
    }

    bool on_button_press_event(GdkEventButton *event) override {
        if(event->type == GDK_BUTTON_PRESS) {
            return process_pointer_event(MouseEvent::Type::PRESS,
                                         event->x, event->y, event->state, event->button);
        } else if(event->type == GDK_2BUTTON_PRESS) {
            return process_pointer_event(MouseEvent::Type::DBL_PRESS,
                                         event->x, event->y, event->state, event->button);
        }
        return false;
    }

    bool on_button_release_event(GdkEventButton *event) override {
        return process_pointer_event(MouseEvent::Type::RELEASE,
                                     event->x, event->y, event->state, event->button);
    }

    bool on_scroll_event(GdkEventScroll *event) override {
        if(event->delta_y < 0 || event->direction == GDK_SCROLL_UP) {
            return process_pointer_event(MouseEvent::Type::SCROLL_VERT,
                                         event->x, event->y, event->state, 0, -1);
        } else if(event->delta_y > 0 || event->direction == GDK_SCROLL_DOWN) {
            return process_pointer_event(MouseEvent::Type::SCROLL_VERT,
                                         event->x, event->y, event->state, 0, 1);
        }
        return false;
    }

    bool on_leave_notify_event(GdkEventCrossing *event) override {
        return process_pointer_event(MouseEvent::Type::LEAVE,
                                     event->x, event->y, event->state);
    }

    bool process_key_event(KeyboardEvent::Type type, GdkEventKey *gdk_event) {
        KeyboardEvent event = {};

        char32_t chr = gdk_keyval_to_unicode(gdk_keyval_to_lower(gdk_event->keyval));
        if(chr != 0) {
            event.key = KeyboardEvent::Key::CHARACTER;
            event.chr = chr;
        } else if(gdk_event->keyval >= GDK_KEY_F1 &&
                  gdk_event->keyval <= GDK_KEY_F12) {
            event.key = KeyboardEvent::Key::FUNCTION;
            event.num = gdk_event->keyval - GDK_KEY_F1;
        } else {
            return false;
        }

        event.shiftDown   = (gdk_event->state & GDK_SHIFT_MASK)   != 0;
        event.controlDown = (gdk_event->state & GDK_CONTROL_MASK) != 0;

        if(_receiver->onKeyboardEvent) {
            return _receiver->onKeyboardEvent(event);
        }

        return true;
    }

    bool on_key_press_event(GdkEventKey *event) override {
        return process_key_event(KeyboardEvent::Type::PRESS, event);
    }

    bool on_key_release_event(GdkEventKey *event) override {
        return process_key_event(KeyboardEvent::Type::RELEASE, event);
    }
};

class GtkWindow : public Gtk::Window {
    Platform::Window   *_receiver;
    GtkGLWidget         _gl_widget;
    bool                _is_fullscreen;

public:
    GtkWindow(Platform::Window *receiver) : _receiver(receiver), _gl_widget(receiver) {
        add(_gl_widget);
    }

    bool is_full_screen() {
        return _is_fullscreen;
    }

    GtkGLWidget &get_gl_widget() {
        return _gl_widget;
    }

protected:
    void on_hide() {
        if(_receiver->onClose) {
            _receiver->onClose();
        }
    }

    bool on_window_state_event(GdkEventWindowState *event) override {
        _is_fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
        if(_receiver->onFullScreen) {
            _receiver->onFullScreen(_is_fullscreen);
        }

        return Gtk::Window::on_window_state_event(event);
    }
};

//-----------------------------------------------------------------------------
// Implementation of our interfaces.
//-----------------------------------------------------------------------------

class TimerImplGtk : public Timer {
public:
    sigc::connection    connection;

    void WindUp(unsigned milliseconds) override {
        if(!connection.empty()) {
            connection.disconnect();
        }

        auto handler = [this]() {
            if(this->onTimeout) {
                this->onTimeout();
            }
            return false;
        };
        connection = Glib::signal_timeout().connect(handler, milliseconds);
    }
};

TimerRef CreateTimer() {
    return std::unique_ptr<TimerImplGtk>(new TimerImplGtk);
}

class MenuItemImplGtk : public MenuItem {
public:
    GtkMenuItem gtkMenuItem;

    MenuItemImplGtk() : gtkMenuItem(this) {}

    void SetIndicator(Indicator state) override {
        switch(state) {
            case Indicator::NONE:
                gtkMenuItem.set_has_indicator(false);
                break;

            case Indicator::CHECK_MARK:
                gtkMenuItem.set_has_indicator(true);
                gtkMenuItem.set_draw_as_radio(false);
                break;

            case Indicator::RADIO_MARK:
                gtkMenuItem.set_has_indicator(true);
                gtkMenuItem.set_draw_as_radio(true);
                break;
        }
    }

    void SetState(bool state) override {
        ssassert(gtkMenuItem.has_indicator(),
                 "Cannot change state of a menu item without indicator");
        gtkMenuItem.set_active(state);
    }
};

class MenuImplGtk : public Menu {
public:
    Gtk::Menu   gtkMenu;
    std::vector<std::shared_ptr<MenuItemImplGtk>>   menuItems;

    std::string PrepareLabel(std::string label) {
        std::replace(label.begin(), label.end(), '&', '_');
        return label;
    }

    MenuItemRef AddItem(const std::string &label, std::function<void()> onActivate = NULL,
                        KeyboardEvent accel = {}) override {
        guint accelKey;
        if(accel.key == KeyboardEvent::Key::CHARACTER) {
            accelKey = gdk_unicode_to_keyval(accel.chr);
        } else if(accel.key == KeyboardEvent::Key::FUNCTION) {
            accelKey = GDK_KEY_F1 + accel.num;
        }

        Gdk::ModifierType accelMods = {};
        if(accel.shiftDown) {
            accelMods |= Gdk::SHIFT_MASK;
        }
        if(accel.controlDown) {
            accelMods |= Gdk::CONTROL_MASK;
        }

        auto menuItem = std::make_shared<MenuItemImplGtk>();
        menuItem->gtkMenuItem.set_label(PrepareLabel(label));
        menuItem->gtkMenuItem.set_use_underline(true);
        menuItem->gtkMenuItem.set_accel_key(Gtk::AccelKey(accelKey, accelMods));
        menuItem->onActivate = onActivate;

        gtkMenu.append(menuItem->gtkMenuItem);
        menuItems.push_back(menuItem);

        return menuItem;
    }

    MenuRef AddSubmenu(const std::string &label) override {
        auto menuItem = std::make_shared<MenuItemImplGtk>();
        menuItem->gtkMenuItem.set_label(PrepareLabel(label));
        menuItem->gtkMenuItem.set_use_underline(true);

        auto submenu = std::make_shared<MenuImplGtk>();
        menuItem->gtkMenuItem.set_submenu(submenu->gtkMenu);

        gtkMenu.append(menuItem->gtkMenuItem);
        menuItems.push_back(menuItem);

        return submenu;
    }

    void AddSeparator() override {
        Gtk::SeparatorMenuItem *menuItem = Gtk::manage(new Gtk::SeparatorMenuItem());
        gtkMenu.append(*Gtk::manage(menuItem));
    }

    bool PopUp() override {
        bool dismissed = true;
        gtkMenu.signal_selection_done().connect([&]() { dismissed = false; });

        Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create();
        gtkMenu.signal_deactivate().connect([&]() { loop->quit(); });

        gtkMenu.show_all();
        gtkMenu.popup(0, GDK_CURRENT_TIME);

        loop->run();

        return !dismissed;
    }

    void Clear() override {
        for(auto &menuItem : menuItems) {
            gtkMenu.remove(((MenuItemImplGtk*)menuItem.get())->gtkMenuItem);
        }
        menuItems.clear();
    }
};

MenuRef CreateMenu() {
    return std::shared_ptr<MenuImplGtk>(new MenuImplGtk);
}

class WindowImplGtk : public Window {
public:
    GtkWindow   gtkWindow;

    WindowImplGtk() : gtkWindow(this) {}

    bool IsVisible() override {
        return gtkWindow.is_visible();
    }

    bool IsFullScreen() override {
        return gtkWindow.is_full_screen();
    }

    bool IsEditorVisible() override {
        return false;
    }

    void GetSize(double *width, double *height) override {
        *width  = gtkWindow.get_allocated_width();
        *height = gtkWindow.get_allocated_height();
    }

    int GetIntegralScaleFactor() override {
        return gtkWindow.get_scale_factor();
    }

    double GetFractionalScaleFactor() override {
        return gtkWindow.get_scale_factor();
    }

    double GetPixelDensity() override {
        return gtkWindow.get_screen()->get_resolution();
    }

    void SetVisible(bool visible) override {
        if(visible) {
            gtkWindow.show_all();
        } else {
            gtkWindow.hide();
        }
    }

    void SetFullScreen(bool fullScreen) override {
        if(fullScreen) {
            gtkWindow.fullscreen();
        } else {
            gtkWindow.unfullscreen();
        }
    }

    void SetTitle(const std::string &title) override {
        gtkWindow.set_title(title + " — SolveSpace");
    }

    void SetTooltip(const std::string &text) override {
        if(text.empty()) {
            gtkWindow.get_gl_widget().set_has_tooltip(false);
        } else {
            gtkWindow.get_gl_widget().set_tooltip_text(text);
        }
    }

    void ShowEditor(double x, double y, double fontHeight, int widthInChars,
                    bool isMonospace, const std::string &text) override {}
    void HideEditor() override {}

    void Invalidate() override {
        gtkWindow.get_gl_widget().queue_render();
    }

    void Redraw() override {
        Invalidate();
        Gtk::Main::iteration(/*blocking=*/false);
    }
};

WindowRef CreateWindow() {
    return std::unique_ptr<WindowImplGtk>(new WindowImplGtk);
}

//-----------------------------------------------------------------------------
// Implemenetation of application-wide functionality.
//-----------------------------------------------------------------------------

void Quit() {
    Gtk::Main::quit();
}

}
}

//-----------------------------------------------------------------------------
// The main function.
//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    Gtk::Main main(argc, argv);

    SS.Init();

    main.run();

    return 0;
}
