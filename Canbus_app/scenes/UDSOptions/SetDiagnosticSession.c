#include "../../app_user.h"

// To know the session
uint8_t session = 0;

// Thread to send the message
static int32_t set_diagnostic_session_thread(void* context);

// callback for the diagnostic sessions menu
void submenu_diagnostic_session_callback(void* context, uint32_t index) {
    App* app = context;
    session = index;
    scene_manager_next_scene(app->scene_manager, app_scene_uds_set_session_response);
}

// Scene on enter
void app_scene_uds_set_diagnostic_session_on_enter(void* context) {
    App* app = context;

    submenu_reset(app->submenu);

    submenu_set_header(app->submenu, "Set Diagnostic Session");

    submenu_add_item(app->submenu, "Default Session", 0, submenu_diagnostic_session_callback, app);
    submenu_add_item(
        app->submenu, "Programming Session", 1, submenu_diagnostic_session_callback, app);
    submenu_add_item(
        app->submenu, "Extended Session", 2, submenu_diagnostic_session_callback, app);

    submenu_add_item(app->submenu, "Safety Session", 3, submenu_diagnostic_session_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubmenuView);
}

// Scene on event
bool app_scene_uds_set_diagnostic_session_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

// Scene on exit
void app_scene_uds_set_diagnostic_session_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}

// Scene on enter for the response
void app_scene_uds_set_session_response_on_enter(void* context) {
    App* app = context;
    widget_reset(app->widget);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);

    app->thread =
        furi_thread_alloc_ex("SetSessionThread", 1024, set_diagnostic_session_thread, app);
    furi_thread_start(app->thread);
}

// Scene on event for the response
bool app_scene_uds_set_session_response_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;

    App* app = context;

    UNUSED(app);
    UNUSED(event);

    return consumed;
}

// Scene on exit for the response
void app_scene_uds_set_session_response_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);

    furi_thread_join(app->thread);
    furi_thread_free(app->thread);
}

// Thread to work
static int32_t set_diagnostic_session_thread(void* context) {
    App* app = context;
    UNUSED(app);
    return 0;
}
