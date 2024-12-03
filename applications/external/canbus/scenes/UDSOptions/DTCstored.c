#include "../../app_user.h"

bool delete_dtc = false;

// Thread to work with for dtc
static int32_t thread_uds_protocol_get_dtc(void* context);

// Thread to delete dtc
static int32_t thread_uds_protocol_delete_dtc(void* context);

// Callback for the menu
void storage_dtc_menu_callback(void* context, uint32_t index) {
    App* app = context;
    delete_dtc = (index == 1) ? true : false;

    // Go to the response scene
    scene_manager_next_scene(app->scene_manager, app_scene_uds_dtc_response_option);
}

// Scene on enter
void app_scene_uds_get_dtc_menu_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);

    submenu_set_header(app->submenu, "MENU DTC STORED");

    submenu_add_item(app->submenu, "Read DTC", 0, storage_dtc_menu_callback, app);
    submenu_add_item(app->submenu, "Delete DTC", 1, storage_dtc_menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_uds_get_dtc_menu_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_get_dtc_menu_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

//Scene on enter for dtc response
void app_scene_uds_dtc_response_on_enter(void* context) {
    App* app = context;

    widget_reset(app->widget);

    if(delete_dtc) {
        app->thread =
            furi_thread_alloc_ex("Get DTCs", 4 * 1024, thread_uds_protocol_delete_dtc, app);
    } else {
        app->thread = furi_thread_alloc_ex("Get DTCs", 4 * 1024, thread_uds_protocol_get_dtc, app);
    }
    furi_thread_start(app->thread);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

// Scene on event for dtc response
bool app_scene_uds_dtc_response_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_dtc_response_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
}

/**
 * Functions to Draw the responses
 */

// To show no DTC read it
void draw_no_dtc(App* app) {
    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "No DTC\nStored to show");
}

/**
 * Thread to work with DTC and UDS protocol
 */

static int32_t thread_uds_protocol_get_dtc(void* context) {
    App* app = context;
    MCP2515* CAN = app->mcp_can;

    FuriString* text = app->text;

    furi_string_reset(text);

    UDS_SERVICE* uds_service = uds_service_alloc(
        UDS_REQUEST_ID_DEFAULT, UDS_RESPONSE_ID_DEFAULT, CAN->mode, CAN->clck, CAN->bitRate);

    bool debug = uds_init(uds_service);

    furi_delay_ms(100);

    if(debug) {
        uint16_t count_of_dtc = 0;

        debug = uds_get_count_stored_dtc(uds_service, &count_of_dtc);

        if(!debug) {
            draw_transmition_failure(app);
        }

        if(count_of_dtc == 0 && debug) {
            draw_no_dtc(app);
            debug = false;
        }

        uint8_t data_count = 4 * count_of_dtc;

        uint8_t codes[data_count];

        if(debug) {
            if(uds_get_stored_dtc(uds_service, codes, &count_of_dtc)) {
                debug = true;
            } else {
                draw_transmition_failure(app);
                debug = false;
            }
        }
    } else {
        draw_device_no_connected(app);
    }

    while(debug && furi_hal_gpio_read(&gpio_button_back)) {
        furi_delay_ms(1);
    }

    free_uds(uds_service);

    log_info("Espacio liberado");

    return 0;
}

/**
 * Thread to delete the DTC stored
 */

static int32_t thread_uds_protocol_delete_dtc(void* context) {
    App* app = context;
    MCP2515* CAN = app->mcp_can;

    FuriString* text = app->text;

    furi_string_reset(text);

    UDS_SERVICE* uds_service = uds_service_alloc(
        UDS_REQUEST_ID_DEFAULT, UDS_RESPONSE_ID_DEFAULT, CAN->mode, CAN->clck, CAN->bitRate);

    if(!uds_init(uds_service)) {
        draw_device_no_connected(app);
        free_uds(uds_service);
        return 0;
    }

    free_uds(uds_service);

    log_info("Espacio liberado");

    return 0;
}
