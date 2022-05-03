# ESP32 based Four Legged Spider Robot

I believe the best way to learn STEM and engineering is through hand on experience. Hacking a four legged spider robot with my 5 year old son is my way of learning as well as teaching various engineering disciplines. With that said, let's dive into the details.

## Overview

### Chassis / Mechanical Design

Currently, I'm using the [Spider robot chassis](https://www.thingiverse.com/thing:1009659) designed by [regishsu](https://www.thingiverse.com/regishsu/designs) on [Thingiverse](https://www.thingiverse.com/). If you own a 3D printer, you can print your own with virtually no cost. Otherwise, you can have it printed for you by online services such as [CraftCloud](https://craftcloud3d.com/). I did a test, and it quoted me $27 USD to have someone print it for me in PLA, delivered in 10 - 18 business days.

### Hardware

#### ESP32 Micro Controller Board

The brain of the spider is a [ESP32 MCU](https://www.espressif.com/en/products/socs/esp32) that I bought on [Amazon](https://www.amazon.com/gp/product/B08DQQ8CBP/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&th=1). It has Bluetooth and WIFI support. Bluetooth is used to control the robot via a bluetooth controller such as PS4 / PS5 controller. WIFI is used to host a small web app where you can tweak various settings for the robot. The web app is not yet implemented. the planned use is to expose a UI for servo calibration and test, but other ideas are welcome. Just [file an issue](https://github.com/aq1018/spidy/issues), and I'll reply to your recommendations.

#### Servos

Since this is a 4 legged robot, there are 12 servo motors total to move the robot. The SG-90 ( clones or original ) are used in this project. Again, I bought some cloned ones called [Miuzei SG90 9G Servos](https://www.amazon.com/gp/product/B072V529YD/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&psc=1) from Amazon. There is also [this one](https://www.amazon.com/Dorhea-Helicopter-Airplane-Walking-Compatible/dp/B08FJ27Q1H/ref=sr_1_5?crid=1EAQ0NCMT8FEO&keywords=SG90&qid=1651563422&s=toys-and-games&sprefix=sg90%2Ctoys-and-games%2C106&sr=1-5&th=1) selling 12 of them for $21 USD. However, I have not tried those, so I do not know if they are of good quality.

#### Custom PCB (Optional)

You can probably hand solder a perfboard and hook up the ESP32 board, servos, and power together, but I made a [custom PCB](https://oshwlab.com/aq1018/robot) to make things a bit easier for me. This PCB allows you to hook up a 4.8V NIMH battery to power the MCU and the servos connected via the pins on the 4 sides of the board.

I then order 5 pieces of this custom PCB via [JLCPCB](https://jlcpcb.com/). It costed me $35 USD. You can probably get it cheaper with new customer coupons. It can get even cheaper if you opt in to solder the components yourself. But $35 USD for custom made PCB is already dirty cheap in my opinion.

### Battery

I personally bought a [7.4V LiPo Battery](https://www.amazon.com/gp/product/B016ZM3CVA/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&psc=1) as the power source. This means the servos are using 7.4V - 8.4V instead of the manufacturer recommended d 4.8V - 6V. I don't think this is a big deal because the servos are so cheap and they seem to be working alright under those voltages, but your milage may vary. I only chose to take the dangerous route because I don't want to spend the money to buy another battery. But if you want it up to spec, then you should probably get a [4.8V NIHM Battery like this one](https://www.amazon.com/Receiver-Battery-Connectors-Capacity-Rechargeable/dp/B08K78JGDG/) instead.

### Software

I used the official [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) as the base library for ESP32 boards. This is because it offers all the flexibility such as `ledc` fade functionality to provide efficient control over leg movements as well as bluetooth connectivity which allows me to use a PS4 controller to control the robot. I also used the following 3rd party libraries for various purposes:

* [Bluepad32](https://github.com/ricardoquesada/bluepad32) - Bluetooth gamepad connectivity.
* [btstack](https://github.com/bluekitchen/btstack) - Needed by `Bluepad32`. Low level bluetooth stack.
* [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) - Arduino compatible servo interface with ESP32 ledc. This will be replaced with custom implementation soon.
* [esp32-arduino](https://github.com/espressif/arduino-esp32) - Arduino component for ESP32. Provides Arduino interfaces. Needed by `Bluepad32` and `ESP32Servo` components.

## Setup

### 1. Install esp-idf

You need to install the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf) in order to build this project.

For windows, please follow [the official documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html) to install.

For Linux & MacOS, please [follow this official documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html).

### 2. Clone this repository

```
git clone https://github.com/aq1018/spidy.git --recursive
```

### 3. Install `btstack` Component

In a terminal, initialize the ESP-IDF environment by using the command below:

```
. ~/esp/esp-idf/export.sh
```

Note, this is assuming you have installed `esp-idf` under `~/esp/esp-idf`.

Next, go to the `btstack` directory.

```
# assuming you are in the project root, navigate to btstack by:
cd vendor/bluepad32/external/btstack

# now you need to go to the esp32 port:
cd port/esp32

# now run the following to integrate `btstack` to `esp-idf`:
./integrate_btstack.py

# check your esp-idf components dir, and you should see files there.
ls ~/esp/esp-idf/components/btstack
```

### 4. Build Project

If you haven't done so, in a terminal, initialize the ESP-IDF environment by using the command below:

```
. ~/esp/esp-idf/export.sh
```

Now under your project root run the following terminal command:

```
idf.py build
```

This should build the project.

Alternatively, if you use VSCode, you can install the [Espressif IDF](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) plugin, and then you can build, flash and monitor with a single button.

### 5. Upload binary to MCU

Run the following to flash your MCU:

```
idf.py flash
```

Or use VSCode Espressif IDF as described in previous section.
