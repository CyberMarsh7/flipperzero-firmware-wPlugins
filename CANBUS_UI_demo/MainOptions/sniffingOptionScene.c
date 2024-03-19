#include "../app_user.h"

static void callback_interrupt(void* context) {
    App* app = context;
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagReceived);
}

static int32_t sniffing_worker(void* context) {
    App* app = context;
    MCP2515* mcp_can = app->mcp_can;
    CANFRAME* frame = app->can_frame;

    ERROR_CAN debugStatus = mcp2515_init(mcp_can);

    furi_hal_gpio_init(&gpio_swclk, GpioModeInterruptFall, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_swclk, callback_interrupt, app);

    if(debugStatus == ERROR_OK) {
        log_info("MCP START OK");
    } else {
        log_exception("MCP SET FAILURE");
    }

    while(true) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        if(events & WorkerflagStop)
            break;
        else if(events & WorkerflagReceived) {
            if(readMSG(mcp_can, frame) == ERROR_OK) log_info("Message Received");
        }
    }

    furi_hal_gpio_remove_int_callback(&gpio_swclk);
    freeMCP2515(mcp_can);
    return 0;
}

void app_scene_Sniffing_on_enter(void* context) {
    App* app = context;
    app->mcp_can->mode = MCP_LISTENONLY;

    //  We alloc the thread
    app->thread = furi_thread_alloc_ex("SniffingWork", 1024, sniffing_worker, app);

    // Start the thread
    furi_thread_start(app->thread);

    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 25, 15, AlignLeft, AlignCenter, FontPrimary, "Sniffing");
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewWidget);
}

bool app_scene_Sniffing_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void app_scene_Sniffing_on_exit(void* context) {
    App* app = context;

    // To close the Thread
    furi_thread_flags_set(furi_thread_get_id(app->thread), WorkerflagStop);
    furi_thread_join(app->thread);
    furi_thread_free(app->thread);

    // widget Reset
    widget_reset(app->widget);
}