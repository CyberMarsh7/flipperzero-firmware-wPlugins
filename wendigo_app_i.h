#pragma once

#include "wendigo_app.h"
#include "scenes/wendigo_scene.h"
#include "wendigo_custom_event.h"
#include "wendigo_uart.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_input.h>
#include "wendigo_hex_input.h"

#define START_MENU_ITEMS (7)
#define SETUP_MENU_ITEMS (3)

#define WENDIGO_TEXT_BOX_STORE_SIZE (4096)
#define WENDIGO_TEXT_INPUT_STORE_SIZE (512)

struct WendigoApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    char text_input_store[WENDIGO_TEXT_INPUT_STORE_SIZE + 1];
    FuriString* text_box_store;
    size_t text_box_store_strlen;
    TextBox* text_box;
    TextInput* text_input;
    Wendigo_TextInput* hex_input;
    Widget* widget;
    VariableItemList* var_item_list;
    Wendigo_Uart* uart;

    int setup_selected_menu_index;
    int setup_selected_option_index[SETUP_MENU_ITEMS];
    int selected_menu_index;
    int selected_option_index[START_MENU_ITEMS];
    const char* selected_tx_string;

    bool is_command;
    bool is_custom_tx_string;
    bool hex_mode;
    uint8_t uart_ch;
    uint8_t new_uart_ch;
    int BAUDRATE;
    int NEW_BAUDRATE;
    int TERMINAL_MODE; //1=AT mode, 0=other mode
};

typedef enum {
    WendigoAppViewVarItemList,
    WendigoAppViewConsoleOutput,
    WendigoAppViewTextInput,
    WendigoAppViewHexInput,
    WendigoAppViewHelp,
} WendigoAppView;
