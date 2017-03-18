//-----------------------------------------------------------------------------
// An abstraction for platform-dependent GUI functionality.
//
// Copyright 2017 whitequark
//-----------------------------------------------------------------------------

#ifndef SOLVESPACE_GUI_H
#define SOLVESPACE_GUI_H

namespace Platform {

//-----------------------------------------------------------------------------
// Event descriptions.
//-----------------------------------------------------------------------------

// A mouse input event.
class MouseEvent {
public:
    enum class Type {
        MOTION,
        PRESS,
        DBL_PRESS,
        RELEASE,
        SCROLL_VERT,
        LEAVE,
    };

    enum class Button {
        NONE,
        LEFT,
        MIDDLE,
        RIGHT,
    };

    Type        type;
    double      x;
    double      y;
    Button      button;
    bool        shiftDown;
    bool        controlDown;
    int         scrollDelta;
};

// A keyboard input event.
class KeyboardEvent {
public:
    enum class Type {
        PRESS,
        RELEASE,
    };

    enum class Key {
        CHARACTER,
        FUNCTION,
    };

    Type        type;
    Key         key;
    char32_t    chr; // for Key::CHARACTER
    int         num; // for Key::FUNCTION
    bool        shiftDown;
    bool        controlDown;
};

//-----------------------------------------------------------------------------
// Interfaces for platform-dependent functionality.
//-----------------------------------------------------------------------------

// A native single-shot timer.
class Timer {
public:
    std::function<void()>   onTimeout;

    virtual void WindUp(unsigned milliseconds) = 0;
};

typedef std::unique_ptr<Timer> TimerRef;

// A native menu item.
class MenuItem {
public:
    enum class Indicator {
        NONE,
        CHECK_MARK,
        RADIO_MARK,
    };

    std::function<void()>   onActivate;

    virtual void SetIndicator(Indicator indicator) = 0;
    virtual void SetState(bool state) = 0;
};

typedef std::shared_ptr<MenuItem> MenuItemRef;

// A native menu.
class Menu {
public:
    virtual std::shared_ptr<MenuItem> AddItem(const std::string &label,
                                              std::function<void()> onActivated = NULL,
                                              KeyboardEvent accel = {}) = 0;
    virtual std::shared_ptr<Menu> AddSubmenu(const std::string &label) = 0;
    virtual void AddSeparator() = 0;

    virtual bool PopUp() = 0;

    virtual void Clear() = 0;
};

typedef std::shared_ptr<Menu> MenuRef;

// A native top-level window, with an OpenGL context, and an editor overlay.
class Window {
public:
    std::function<void()>               onClose;
    std::function<void(bool)>           onFullScreen;
    std::function<bool(MouseEvent)>     onMouseEvent;
    std::function<bool(KeyboardEvent)>  onKeyboardEvent;
    std::function<void(std::string)>    onEditingDone;
    std::function<void()>               onRender;

    virtual bool IsVisible() = 0;
    virtual bool IsFullScreen() = 0;
    virtual bool IsEditorVisible() = 0;

    virtual void GetSize(double *width, double *height) = 0;
    virtual int GetIntegralScaleFactor() = 0;       // for raster rendering
    virtual double GetFractionalScaleFactor() = 0;  // for vector rendering and text
    virtual double GetPixelDensity() = 0;           // in pixels per inch; for scaling

    virtual void SetVisible(bool visible) = 0;
    virtual void SetFullScreen(bool fullScreen) = 0;
    virtual void SetTitle(const std::string &title) = 0;
    virtual bool SetTitleForFilename(const Path &filename) { return false; }
    virtual void SetTooltip(const std::string &text) = 0;

    virtual void ShowEditor(double x, double y, double fontHeight, int widthInChars,
                            bool isMonospace, const std::string &text) = 0;
    virtual void HideEditor() = 0;

    virtual void Invalidate() = 0;
    virtual void Redraw() = 0;
};

typedef std::unique_ptr<Window> WindowRef;

// Factories.
TimerRef CreateTimer();
MenuRef CreateMenu();
WindowRef CreateWindow();

// Application-wide functionality.
void Quit();

}

#endif
