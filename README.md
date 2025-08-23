### Introduction
[中文](https://github.com/PixelCrafter42/NotionEinkTodo/blob/main/ReadMeCn.md)
I love Notion and e‑ink displays. I wanted a device that could show my to‑dos at a glance, helping me stay focused on what matters most.

The M5Stack Paper S3 made this idea real. A month ago I had no hands‑on experience in this area. Thanks to M5Stack’s UIFlow, I quickly built a prototype. With help from AI I iterated on the code, then rebuilt a faster, more responsive version with Arduino. The whole journey was fun and very rewarding.

This project uses the M5Stack Paper S3, a compact development board with an ESP32 and a touch e‑ink display. I leveraged its networking and touch features to build a kanban‑style dashboard that syncs to‑dos with Notion: tap an item on the device to mark it done, and the status updates back to your Notion Todo list. Fonts and configuration are stored on an SD card, and I added a configuration server mode so settings can be edited from a web page—no more plugging and unplugging the SD card.
![cover](https://i.imgur.com/FkcCQG8.jpeg)
![list](https://i.imgur.com/HfFJZxJ.jpeg)
![config](https://i.imgur.com/rMsshyo.jpeg)
![server](https://i.imgur.com/ceGph7t.jpeg)
### How to Use

1. Install the Arduino IDE and required libraries. See the [official guide](https://docs.m5stack.com/en/arduino/m5papers3/program).
2. Copy the following files from the repository to the SD card root:
    - cover.png (boot splash)
    - CNFont.vlw (required; Chinese font file—customize with the [Font Creator](https://vlw-font-creator.m5stack.com/))
    - config.json (required; fill in Wi‑Fi, Notion API, and Notion database settings)
3. Open the .ino file in Arduino, compile, and upload the firmware to the Paper S3.
4. Enjoy!

### References

- https://docs.m5stack.com/en/arduino/m5papers3/program
- https://developers.notion.com/
