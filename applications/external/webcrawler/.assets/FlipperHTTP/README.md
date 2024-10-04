# FlipperHTTP
HTTP library for Flipper Zero. Compatible with Wifi Dev Board for Flipper Zero (ESP32S2 Dev Module).

## Installation
1. Download the `flipper_http_bootloader.bin`, `flipper_http_firmware_a.bin`, and `flipper_http_partitions.bin` files.
2. Unplug your Wifi Dev Board and connect your Flipper Zero to your computer.
3. Open up qFlipper.
4. Click on the `File manager`.
5. Naviate to `SD Card/apps_data/esp_flasher/`
6. Drag all three bin files (or the entire folder) into the directory.
7. Disconnect your Flipper from your computer then turn off your Flipper.
8. Plug your Wi-Fi Devboard into the Flipper then turn on your Flipper.
9. Open the ESP Flasher app on your Flipper, it should be located under `Apps->GPIO` from the main menu. If not, download it from the Flipper App Store.
10. In the ESP Flasher app, select the following options:
    - "Reset Board": wait a few seconds, then go back.
    - "Enter Bootloader": wait until the 'waiting for download' message appears, then go back.
11. Click on Manual Flash.
12. Click on Bootloader and select the `flipper_http_bootloader.bin` that you downloaded earlier.
13. Click on Part Table and select the `flipper_http_partitions.bin` that you downloaded earlier.
14. Click on FirmwareA and select the `flipper_http_firmware_a.bin` that you downloaded earlier.
15. Click on FLASH - slow. If successful, you will see three green LED blinks on the Dev board.
16. On the Dev Board, press the RESET button once.

You should be all set. Here's the initial guide: https://www.youtube.com/watch?v=Y2lUVTMTABE


# Usage
The `flipper_http.h` file is for seamless use of the FlipperHTTP Firmware in `C` applications and is the root of the `Web Crawler` app. A Javascript library will be available soon, followed by a mPython library.

## General
- Init:
    - `flipper_http_init(FlipperHTTP_Callback callback, void *context)`
- DeInit:
    - `flipper_http_deinit()`

## Wifi
- Connect To Wifi: 
    - `flipper_http_connect_wifi()`
- Disconnect: 
    - `flipper_http_disconnect_wifi()`
- Ping: 
    - `flipper_http_ping()`
- Save Wifi: 
    - `flipper_http_save_wifi(const char *ssid, const char *password)`

## Extras
- Send Data:
    - `flipper_http_send_data(const char *data)`
- Rx Callback:
    - `flipper_http_rx_callback(const char *line, void *context)`
- GET request:
    - `flipper_http_get_request(const char *url)`
- Save Data to SD:
    - `flipper_http_save_received_data(size_t bytes_received, const char line_buffer[])`
