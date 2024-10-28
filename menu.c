#include "menu.h"
#include <stdlib.h>
#include <string.h>
#include "uart_utils.h"
#include "settings_storage.h"
#include "settings_def.h"
#include "confirmation_view.h" 


typedef struct {
    const char* label;      // Display label in menu 
    const char* command;    // UART command to send
    const char* capture_prefix; // Prefix for capture files (NULL if none)
    const char* file_ext;      // File extension for captures (NULL if none)
    const char* folder;        // Folder for captures (NULL if none)
    bool needs_input;          // Whether command requires text input
    const char* input_text;    // Text to show in input box (NULL if none)
    bool needs_confirmation;   // Whether command needs confirmation
    const char* confirm_header; // Confirmation dialog header
    const char* confirm_text;  // Confirmation dialog text
} MenuCommand;

typedef struct {
    AppState* state;
    const MenuCommand* command;
} MenuCommandContext;


// Function declarations
static void text_input_result_callback(void* context);

// WiFi menu command definitions - grouped by function
static const MenuCommand wifi_commands[] = {
    // Scanning Operations
    {"Scan WiFi APs", "scanap\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Scan WiFi Stations", "scansta\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Stop Scan", "stopscan\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"List APs", "list -a\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"List Stations", "list -s\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Select AP", "select -a", NULL, NULL, NULL, true, "AP Number", false, NULL, NULL},
    
    // Beacon Spam Operations
    {"Beacon Spam (List)", "beaconspam -l\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Beacon Spam (Random)", "beaconspam -r\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Beacon Spam (Rickroll)", "beaconspam -rr\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Custom Beacon Spam", "beaconspam", NULL, NULL, NULL, true, "SSID Name", false, NULL, NULL},
    {"Stop Spam", "stopspam\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    
    // Attack Operations
    {"Deauth", "attack -d\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Stop Deauth", "stopdeauth\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    
    // Capture Operations
    {"Sniff Raw Packets", "capture -raw\n", "raw_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Sniff PMKID", "capture -eapol\n", "pmkid_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Sniff Probes", "capture -probe\n", "probes_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Sniff WPS", "capture -wps\n", "wps_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Sniff Deauth", "capture -deauth\n", "deauth_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Sniff Beacons", "capture -beacon\n", "beacon_capture", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Stop Capture", "capture -stop\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    
    // Portal & Network Operations
    {"Evil Portal", "startportal\n", NULL, NULL, NULL, false, NULL, true, 
        "Evil Portal", "You need to configure\n settings in the WebUI\n for this command.\n\n"},
    {"Stop Portal", "stopportal\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Connect To WiFi", "connect", NULL, NULL, NULL, true, "SSID,Password", false, NULL, NULL},
    {"Cast Random Video", "dialconnect\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Printer Power", "powerprinter\n", NULL, NULL, NULL, false, NULL, true,
        "Printer Power", "You need to configure\n settings in the WebUI\n for this command.\n"},
};

// BLE menu command definitions
static const MenuCommand ble_commands[] = {
    {"Find the Flippers", "blescan -f\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    // TEMP REMOVE 
    // {"BLE Spam Detector", "blescan -ds\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"AirTag Scanner", "blescan -a\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"Sniff Bluetooth", "blescan -r\n", "btscan", "pcap", GHOST_ESP_APP_FOLDER_PCAPS, false, NULL, false, NULL, NULL},
    {"Stop BLE Scan", "blescan -s\n", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
};

// GPS menu command definitions
static const MenuCommand gps_commands[] = {
    {"Street Detector", "streetdetector", NULL, NULL, NULL, false, NULL, false, NULL, NULL},
    {"WarDrive", "wardrive", "wardrive_scan", "csv", GHOST_ESP_APP_FOLDER_WARDRIVE, false, NULL, false, NULL, NULL},
};

void send_uart_command(const char* command, void* state) {
    AppState* app_state = (AppState*)state;
    uart_send(app_state->uart_context, (uint8_t*)command, strlen(command));
}

void send_uart_command_with_text(const char* command, char* text, AppState* state) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s\n", command, text);
    uart_send(state->uart_context, (uint8_t*)buffer, strlen(buffer));
}

void send_uart_command_with_bytes(
    const char* command,
    const uint8_t* bytes,
    size_t length,
    AppState* state) {
    send_uart_command(command, state);
    uart_send(state->uart_context, bytes, length);
}

static void confirmation_ok_callback(void* context) {
    MenuCommandContext* cmd_ctx = context;
    if(cmd_ctx && cmd_ctx->state && cmd_ctx->command) {
        send_uart_command(cmd_ctx->command->command, cmd_ctx->state);
        uart_receive_data(
            cmd_ctx->state->uart_context,
            cmd_ctx->state->view_dispatcher,
            cmd_ctx->state,
            cmd_ctx->command->capture_prefix ? cmd_ctx->command->capture_prefix : "",
            cmd_ctx->command->file_ext ? cmd_ctx->command->file_ext : "",
            cmd_ctx->command->folder ? cmd_ctx->command->folder : "");
    }
    free(cmd_ctx);
}

static void confirmation_cancel_callback(void* context) {
    MenuCommandContext* cmd_ctx = context;
    if(cmd_ctx && cmd_ctx->state) {
        switch(cmd_ctx->state->previous_view) {
            case 1: show_wifi_menu(cmd_ctx->state); break;
            case 2: show_ble_menu(cmd_ctx->state); break;
            case 3: show_gps_menu(cmd_ctx->state); break;
            default: show_main_menu(cmd_ctx->state); break;
        }
    }
    free(cmd_ctx);
}


static void execute_menu_command(AppState* state, const MenuCommand* command) {
    // Check ESP connection first
    if(!uart_is_esp_connected(state->uart_context)) {
        // Save current view state
        uint32_t current_view = state->current_view;
        
        // Show error in text box
        text_box_set_text(state->text_box, "Error: ESP not connected!\nPlease check connection and try again.");
        view_dispatcher_switch_to_view(state->view_dispatcher, 5);
        state->current_view = 5;
        
        // Wait a bit and return to previous view
        furi_delay_ms(2000);
        
        // Return to previous view
        view_dispatcher_switch_to_view(state->view_dispatcher, current_view);
        state->current_view = current_view;
        return;
    }

    if(command->needs_confirmation) {
        MenuCommandContext* cmd_ctx = malloc(sizeof(MenuCommandContext));
        cmd_ctx->state = state;
        cmd_ctx->command = command;

        confirmation_view_set_header(state->confirmation_view, command->confirm_header);
        confirmation_view_set_text(state->confirmation_view, command->confirm_text);
        confirmation_view_set_ok_callback(state->confirmation_view, confirmation_ok_callback, cmd_ctx);
        confirmation_view_set_cancel_callback(state->confirmation_view, confirmation_cancel_callback, cmd_ctx);
        
        view_dispatcher_switch_to_view(state->view_dispatcher, 7);  // Switch to confirmation view
        return;
    }

    if(command->needs_input) {
        state->uart_command = command->command;
        text_input_reset(state->text_input);
        text_input_set_header_text(state->text_input, command->input_text);
        text_input_set_result_callback(
            state->text_input,
            text_input_result_callback,
            state,
            state->input_buffer,
            32,
            true);
        view_dispatcher_switch_to_view(state->view_dispatcher, 6);
        return;
    }

    send_uart_command(command->command, state);
    uart_receive_data(
        state->uart_context,
        state->view_dispatcher,
        state,
        command->capture_prefix ? command->capture_prefix : "",
        command->file_ext ? command->file_ext : "",
        command->folder ? command->folder : "");
}

// Generic function to show a menu
static void show_menu(AppState* state, const MenuCommand* commands, size_t command_count, 
                     const char* header, Submenu* menu, uint8_t view_id) {
    submenu_reset(menu);
    submenu_set_header(menu, header);
    
    for(size_t i = 0; i < command_count; i++) {
        submenu_add_item(menu, commands[i].label, i, submenu_callback, state);
    }
    
    view_dispatcher_switch_to_view(state->view_dispatcher, view_id);
    state->current_view = view_id;
    state->previous_view = view_id;
}

// Menu display functions
void show_wifi_menu(AppState* state) {
    show_menu(state, wifi_commands, COUNT_OF(wifi_commands), 
              "WiFi Utilities:", state->wifi_menu, 1);
}

void show_ble_menu(AppState* state) {
    show_menu(state, ble_commands, COUNT_OF(ble_commands),
              "BLE Utilities:", state->ble_menu, 2);
}

void show_gps_menu(AppState* state) {
    show_menu(state, gps_commands, COUNT_OF(gps_commands),
              "GPS Utilities:", state->gps_menu, 3);
}

// Menu command handlers
void handle_wifi_menu(AppState* state, uint32_t index) {
    if(index < COUNT_OF(wifi_commands)) {
        execute_menu_command(state, &wifi_commands[index]);
    }
}

void handle_ble_menu(AppState* state, uint32_t index) {
    if(index < COUNT_OF(ble_commands)) {
        execute_menu_command(state, &ble_commands[index]);
    }
}

void handle_gps_menu(AppState* state, uint32_t index) {
    if(index < COUNT_OF(gps_commands)) {
        execute_menu_command(state, &gps_commands[index]);
    }
}

// Callback for text input results
static void text_input_result_callback(void* context) {
    AppState* input_state = (AppState*)context;
    send_uart_command_with_text(
        input_state->uart_command, 
        input_state->input_buffer, 
        (AppState*)input_state);
    uart_receive_data(
        input_state->uart_context, 
        input_state->view_dispatcher, 
        input_state, "", "", "");
}

// Main menu callback handler
void submenu_callback(void* context, uint32_t index) {
    AppState* state = (AppState*)context;

    switch(state->current_view) {
    case 0: // Main Menu
        switch(index) {
        case 0: show_wifi_menu(state); break;
        case 1: show_ble_menu(state); break;
        case 2: show_gps_menu(state); break;
        case 3: // Settings button
            view_dispatcher_switch_to_view(state->view_dispatcher, 8); // Show settings actions menu
            state->current_view = 8;
            state->previous_view = 8;
            break;
        }
        break;
    case 1: handle_wifi_menu(state, index); break;
    case 2: handle_ble_menu(state, index); break;
    case 3: handle_gps_menu(state, index); break;
    }
}

bool back_event_callback(void* context) {
    AppState* state = (AppState*)context;

    // Get the current view ID
    uint32_t current_view = state->current_view;

    // Do not consume the back button event if the current view is the confirmation view
    if(current_view == 7) {  // 7 is the ID for the confirmation view
        // Allow the confirmation view's input callback to handle the back button
        return false;
    }

    if(current_view == 5) {  // Text box view
        FURI_LOG_D("Ghost ESP", "Stopping Thread");

        // Cleanup text buffer
        if(state->textBoxBuffer) {
            free(state->textBoxBuffer);
            state->textBoxBuffer = malloc(1);
            if(state->textBoxBuffer) {
                state->textBoxBuffer[0] = '\0';
            }
            state->buffer_length = 0;
        }

        // Only send stop commands if enabled in settings
        if(state->settings.stop_on_back_index) {
            // Send all relevant stop commands
            send_uart_command("stop\n", state);
        }

        // Close any open files
        if(state->uart_context->storageContext->current_file &&
           storage_file_is_open(state->uart_context->storageContext->current_file)) {
            state->uart_context->storageContext->HasOpenedFile = false;
            FURI_LOG_D("DEBUG", "Storage File Closed");
        }

        // Return to previous view
        switch(state->previous_view) {
            case 1: show_wifi_menu(state); break;
            case 2: show_ble_menu(state); break;
            case 3: show_gps_menu(state); break;
            default: show_main_menu(state); break;
        }
        state->current_view = state->previous_view;
    } else if(current_view == 8) { // Settings actions menu
        show_main_menu(state);
        state->current_view = 0;
    } else if(current_view != 0) {
        show_main_menu(state);
        state->current_view = 0;
    } else {
        view_dispatcher_stop(state->view_dispatcher);
    }

    // Consume the back button event for all views except the confirmation view
    return true;
}


// Function to show the main menu
void show_main_menu(AppState* state) {
    main_menu_reset(state->main_menu);
    main_menu_set_header(state->main_menu, "Select a Utility:");
    main_menu_add_item(state->main_menu, "WiFi", 0, submenu_callback, state);
    main_menu_add_item(state->main_menu, "BLE", 1, submenu_callback, state);
    // GPS temporarily hidden
    // main_menu_add_item(state->main_menu, "GPS", 2, submenu_callback, state);
    main_menu_add_item(state->main_menu, "CONF", 3, submenu_callback, state);
    view_dispatcher_switch_to_view(state->view_dispatcher, 0);
    state->current_view = 0;
}