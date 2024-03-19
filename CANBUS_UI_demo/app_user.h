#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_box.h>
#include "scenes/app_scene_functions.h"

#include "libraries/mcp_can_2515.h"

typedef enum {
    WorkerflagStop = (1 << 0),
    WorkerflagReceived = (1 << 1),
} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS (WorkerflagStop | WorkerflagReceived)

typedef struct {
    MCP2515* mcp_can;
    CANFRAME* can_frame;
    FuriThread* thread;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
    VariableItemList* varList;
    TextBox* textBox;
    FuriString* text;
} App;

// This is for the menu Options
typedef enum { SniffingOption, SettingsOption } MainMenuOptions;
typedef enum { SniffingOptionEvent, SettingsOptionEvent } MainMenuEvents;

// This is for the Setting Options
typedef enum { BitrateOption, CristyalClkOption } OptionSettings;
typedef enum { BitrateOptionEvent, CristyalClkOptionEvent } SettingsMenuEvent;

// Views in the App
typedef enum {
    SubmenuView,
    ViewWidget,
    VarListView,
    TextBoxView,
} scenesViews;