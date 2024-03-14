#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include "scenes/app_scene_functions.h"

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
} App;

// This is for the menu Options
typedef enum { SniffingOption, SettingsOption } MainMenuOptions;
typedef enum { SniffingOptionEvent, SettingsOptionEvent } MainMenuEvents;

// This is for the Setting Options
typedef enum { BitrateOption, CristyalClkOption } OptionSettings;
typedef enum { BitrateOptionEvent, CristyalClkOptionEvent } SettingsMenuEvent;

typedef enum {
    SubmenuView,
    ViewWidget,
} scenesViews;