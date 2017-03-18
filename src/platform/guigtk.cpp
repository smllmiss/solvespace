//-----------------------------------------------------------------------------
// The GTK-based implementation of platform-dependent GUI functionality.
//
// Copyright 2017 whitequark
//-----------------------------------------------------------------------------
#include <glibmm/main.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/entry.h>
#include <gtkmm/glarea.h>
#include <gtkmm/box.h>
#include <gtkmm/fixed.h>
#include <gtkmm/window.h>
#include <gtkmm/main.h>
#include "solvespace.h"

namespace SolveSpace {
namespace Platform {

//-----------------------------------------------------------------------------
// Implementations of our extensions for stock GTK widgets.
//-----------------------------------------------------------------------------

class GtkMenuItem : public Gtk::CheckMenuItem {
    Platform::MenuItem *_receiver;
    bool                _has_indicator;
    bool                _synthetic_event;

public:
    GtkMenuItem(Platform::MenuItem *receiver) :
        _receiver(receiver), _has_indicator(false), _synthetic_event(false) {
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

    void set_active(bool active) {
        if(Gtk::CheckMenuItem::get_active() == active)
            return;

        _synthetic_event = true;
        Gtk::CheckMenuItem::set_active(active);
        _synthetic_event = false;
    }

protected:
    void on_activate() override {
        Gtk::CheckMenuItem::on_activate();

        if(!_synthetic_event && _receiver->onTrigger) {
            _receiver->onTrigger();
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
                                         event->x, event->y, event->state, 0, 1);
        } else if(event->delta_y > 0 || event->direction == GDK_SCROLL_DOWN) {
            return process_pointer_event(MouseEvent::Type::SCROLL_VERT,
                                         event->x, event->y, event->state, 0, -1);
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
            event.num = gdk_event->keyval - GDK_KEY_F1 + 1;
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

class GtkEditorOverlay : public Gtk::Fixed {
    Platform::Window   *_receiver;
    GtkGLWidget         _gl_widget;
    Gtk::Entry          _entry;

public:
    GtkEditorOverlay(Platform::Window *receiver) : _receiver(receiver), _gl_widget(receiver) {
        add(_gl_widget);

        _entry.set_no_show_all(true);
        _entry.set_has_frame(false);
        add(_entry);

        _entry.signal_activate().
            connect(sigc::mem_fun(this, &GtkEditorOverlay::on_activate));
    }

    bool is_editing() const {
        return _entry.is_visible();
    }

    void start_editing(int x, int y, int font_height, bool is_monospace, int minWidthChars,
                       const std::string &val) {
        Pango::FontDescription font_desc;
        font_desc.set_family(is_monospace ? "monospace" : "normal");
        font_desc.set_absolute_size(font_height * Pango::SCALE);
        _entry.override_font(font_desc);

        // y coordinate denotes baseline
        Pango::FontMetrics font_metrics = get_pango_context()->get_metrics(font_desc);
        y -= font_metrics.get_ascent() / Pango::SCALE;

        Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(get_pango_context());
        layout->set_font_description(font_desc);
        layout->set_text(val + " "); // avoid scrolling
        int width = layout->get_logical_extents().get_width();

        Gtk::Border margin  = _entry.get_style_context()->get_margin();
        Gtk::Border border  = _entry.get_style_context()->get_border();
        Gtk::Border padding = _entry.get_style_context()->get_padding();
        move(_entry,
             x - margin.get_left() - border.get_left() - padding.get_left(),
             y - margin.get_top()  - border.get_top()  - padding.get_top());
        _entry.set_width_chars(minWidthChars);
        _entry.set_size_request(
            width / Pango::SCALE + padding.get_left() + padding.get_right(),
            -1);

        _entry.set_text(val);
        if(!_entry.is_visible()) {
            _entry.show();
            _entry.grab_focus();

            // we grab the input for ourselves and not the entry to still have
            // the pointer events go through to the underlay
            add_modal_grab();
        }
    }

    void stop_editing() {
        if(_entry.is_visible()) {
            remove_modal_grab();
        }
        _entry.hide();
    }

    GtkGLWidget &get_gl_widget() {
        return _gl_widget;
    }

protected:
    bool on_key_press_event(GdkEventKey *event) override {
        if(is_editing()) {
            if(event->keyval == GDK_KEY_Escape) {
                stop_editing();
            } else {
                _entry.event((GdkEvent *)event);
            }
            return true;
        } else {
            return false;
        }
    }

    bool on_key_release_event(GdkEventKey *event) override {
        if(is_editing()) {
            _entry.event((GdkEvent *)event);
            return true;
        } else {
            return false;
        }
    }

    void on_size_allocate(Gtk::Allocation& allocation) override {
        Gtk::Fixed::on_size_allocate(allocation);

        _gl_widget.size_allocate(allocation);
    }

    void on_activate() {
        if(_receiver->onEditingDone) {
            _receiver->onEditingDone(_entry.get_text());
        }
    }
};

class GtkWindow : public Gtk::Window {
    Platform::Window   *_receiver;
    Gtk::VBox           _box;
    Gtk::MenuBar       *_menu_bar;
    GtkEditorOverlay    _editor_overlay;
    bool                _is_fullscreen;

public:
    GtkWindow(Platform::Window *receiver) :
            _receiver(receiver), _menu_bar(NULL), _editor_overlay(receiver) {
        add(_box);
        _box.pack_end(_editor_overlay, /*expand=*/true, /*fill=*/true);
    }

    bool is_full_screen() const {
        return _is_fullscreen;
    }

    Gtk::MenuBar *get_menu_bar() const {
        return _menu_bar;
    }

    void set_menu_bar(Gtk::MenuBar *menu_bar) {
        if(_menu_bar) {
            _box.remove(*_menu_bar);
        }
        _menu_bar = menu_bar;
        if(_menu_bar) {
            _menu_bar->show_all();
            _box.pack_start(*_menu_bar, /*expand=*/false, /*fill=*/false);
        }
    }

    GtkEditorOverlay &get_editor_overlay() {
        return _editor_overlay;
    }

    GtkGLWidget &get_gl_widget() {
        return _editor_overlay.get_gl_widget();
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

static std::string PrepareMenuLabel(std::string label) {
    std::replace(label.begin(), label.end(), '&', '_');
    return label;
}

class MenuItemImplGtk : public MenuItem {
public:
    GtkMenuItem gtkMenuItem;

    MenuItemImplGtk() : gtkMenuItem(this) {}

    void SetAccelerator(KeyboardEvent accel) override {
        guint accelKey;
        if(accel.key == KeyboardEvent::Key::CHARACTER) {
            if(accel.chr == '\t') {
                accelKey = GDK_KEY_Tab;
            } else if(accel.chr == '\e') {
                accelKey = GDK_KEY_Escape;
            } else if(accel.chr == '\x7f') {
                accelKey = GDK_KEY_Delete;
            } else {
                accelKey = gdk_unicode_to_keyval(accel.chr);
            }
        } else if(accel.key == KeyboardEvent::Key::FUNCTION) {
            accelKey = GDK_KEY_F1 + accel.num - 1;
        }

        Gdk::ModifierType accelMods = {};
        if(accel.shiftDown) {
            accelMods |= Gdk::SHIFT_MASK;
        }
        if(accel.controlDown) {
            accelMods |= Gdk::CONTROL_MASK;
        }

        gtkMenuItem.set_accel_key(Gtk::AccelKey(accelKey, accelMods));
    }

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

    void SetActive(bool active) override {
        ssassert(gtkMenuItem.has_indicator(),
                 "Cannot change state of a menu item without indicator");
        gtkMenuItem.set_active(active);
    }

    void SetEnabled(bool enabled) override {
        gtkMenuItem.set_sensitive(enabled);
    }
};

class MenuImplGtk : public Menu {
public:
    Gtk::Menu   gtkMenu;
    std::vector<std::shared_ptr<MenuItemImplGtk>>   menuItems;
    std::vector<std::shared_ptr<MenuImplGtk>>       subMenus;

    MenuItemRef AddItem(const std::string &label,
                        std::function<void()> onTrigger = NULL) override {
        auto menuItem = std::make_shared<MenuItemImplGtk>();
        menuItems.push_back(menuItem);

        menuItem->gtkMenuItem.set_label(PrepareMenuLabel(label));
        menuItem->gtkMenuItem.set_use_underline(true);
        menuItem->gtkMenuItem.show();
        menuItem->onTrigger = onTrigger;
        gtkMenu.append(menuItem->gtkMenuItem);

        return menuItem;
    }

    MenuRef AddSubMenu(const std::string &label) override {
        auto menuItem = std::make_shared<MenuItemImplGtk>();
        menuItems.push_back(menuItem);

        auto subMenu = std::make_shared<MenuImplGtk>();
        subMenus.push_back(subMenu);

        menuItem->gtkMenuItem.set_label(PrepareMenuLabel(label));
        menuItem->gtkMenuItem.set_use_underline(true);
        menuItem->gtkMenuItem.set_submenu(subMenu->gtkMenu);
        menuItem->gtkMenuItem.show_all();
        gtkMenu.append(menuItem->gtkMenuItem);

        return subMenu;
    }

    void AddSeparator() override {
        Gtk::SeparatorMenuItem *gtkMenuItem = Gtk::manage(new Gtk::SeparatorMenuItem());
        gtkMenuItem->show();
        gtkMenu.append(*Gtk::manage(gtkMenuItem));
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
        gtkMenu.foreach([&](Gtk::Widget &w) { gtkMenu.remove(w); });
        menuItems.clear();
        subMenus.clear();
    }
};

MenuRef CreateMenu() {
    return std::shared_ptr<MenuImplGtk>(new MenuImplGtk);
}

class MenuBarImplGtk : public MenuBar {
public:
    Gtk::MenuBar    gtkMenuBar;
    std::vector<std::shared_ptr<MenuImplGtk>>       subMenus;

    MenuRef AddSubMenu(const std::string &label) override {
        auto subMenu = std::make_shared<MenuImplGtk>();
        subMenus.push_back(subMenu);

        Gtk::MenuItem *gtkMenuItem = Gtk::manage(new Gtk::MenuItem);
        gtkMenuItem->set_label(PrepareMenuLabel(label));
        gtkMenuItem->set_use_underline(true);
        gtkMenuItem->set_submenu(subMenu->gtkMenu);
        gtkMenuItem->show_all();
        gtkMenuBar.append(*gtkMenuItem);

        return subMenu;
    }

    void Clear() override {
        gtkMenuBar.foreach([&](Gtk::Widget &w) { gtkMenuBar.remove(w); });
        subMenus.clear();
    }
};

MenuBarRef CreateMenuBar() {
    return std::shared_ptr<MenuBar>(new MenuBarImplGtk);
}

class WindowImplGtk : public Window {
public:
    GtkWindow       gtkWindow;
    MenuBarRef      menuBar;

    WindowImplGtk() : gtkWindow(this) {}

    bool IsVisible() override {
        return gtkWindow.is_visible();
    }

    bool IsFullScreen() override {
        return gtkWindow.is_full_screen();
    }

    bool IsEditorVisible() override {
        return gtkWindow.get_editor_overlay().is_editing();
    }

    void GetSize(double *width, double *height) override {
        *width  = gtkWindow.get_gl_widget().get_allocated_width();
        *height = gtkWindow.get_gl_widget().get_allocated_height();
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

    void SetMenuBar(MenuBarRef newMenuBar) override {
        menuBar = newMenuBar;
        if(menuBar) {
            Gtk::MenuBar *gtkMenuBar = &((MenuBarImplGtk*)&*menuBar)->gtkMenuBar;
            gtkWindow.set_menu_bar(gtkMenuBar);
        } else {
            gtkWindow.set_menu_bar(NULL);
        }
    }

    void ShowEditor(double x, double y, double fontHeight, int widthInChars,
                    bool isMonospace, const std::string &text) override {
        gtkWindow.get_editor_overlay().start_editing(x, y, fontHeight, widthInChars,
                                                     isMonospace, text);
    }

    void HideEditor() override {
        gtkWindow.get_editor_overlay().stop_editing();
    }

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

void Quit() {
    Gtk::Main::quit();
}

}
}

//-----------------------------------------------------------------------------
// The main function.
//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    std::vector<std::string> args = InitPlatform(argc, argv);
    Gtk::Main main(argc, argv);

    SS.Init();
    if(args.size() > 1) {
        SS.Load(Platform::Path::From(args[1]).Expand(/*fromCurrentDirectory=*/true));
    }

    main.run();

    return 0;
}
