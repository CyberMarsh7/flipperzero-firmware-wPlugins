# Flipper MFKey

<img src="https://github.com/noproto/FlipperMfkey/assets/11845893/475bf66a-2e95-44a8-b884-62f28ca5512f" alt="Dolphin" width="60%" height="60%">

## What
This Flipper application ("FAP") cracks Mifare Classic 1K/4K keys on your Flipper Zero. No companion app/desktop needed.

## How
Official guide: https://docs.flipperzero.one/nfc/mfkey32

These are the general steps:

1. Use the Detect Reader function to save nonces on your Flipper from the reader
2. Use the MFKey app to crack the keys
3. Scan the Mifare Classic card

All cracked nonces are automatically added to your user dictionary, allowing you to clone Mifare Classic 1K/4K cards upon re-scanning them.

## Builds
OFW: Available in the App Hub ([Download](https://lab.flipper.net/apps/mfkey)) and distributed by Flipper Devices (https://github.com/flipperdevices/flipperzero-good-faps/tree/dev/mfkey).

Published (options):
* [Unleashed repository](https://github.com/xMasterX/all-the-plugins/tree/main/apps/Tools_Extra)
* [FlipC.org](https://flipc.org/noproto/FlipperMfkey?branch=master&root=fap)
  * Download the FAP at one of the above links then copy the FAP to your Flipper Zero device (using qFlipper or manually copying it to the SD)
* [Unleashed firmware](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
  * Use qFlipper to install the build that ends in "e"
* [RogueMaster firmware](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/releases/latest)
  * Built in to every release, you're good to go!

Manual: Copy the fap/ directory to applications_user/mfkey/ and build it with fbt

## Why
This was the only function of the Flipper Zero that was [thought to be impossible on the hardware](https://old.reddit.com/r/flipperzero/comments/is31re/comment/g72077x/). You can still use other methods if you prefer them.

## Misc Stats
1. RAM used: **127 KB**, RAM free: 13 KB (original was ~53,000 KB, 99.75% RAM usage eliminated)
2. Disk used: (None)
3. Time per unsolved key:

| Category | Time |
| -------- | ---- |
| Best (real world) | 31 seconds |
| Average | 3.4 min |
| Worst possible (expected) | 6.8 min |

NB: Keys that are already in the system/user dictionary or nonces with already found keys are cracked instantly. This means on average cracking an arbitrary number of nonces from the same reader will take 3.4 minutes (1 unknown key).

Writeup: Coming soon

## Developers
noproto, AG
