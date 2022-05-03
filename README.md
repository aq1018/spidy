# ESP32 based Four Legged Spider Robot

I believe the best way to learn STEM and engineering is through hands on experience. Hacking a four legged spider robot with my 5 year old son is my way of learning as well as teaching various engineering disciplines. With that said, let's dive into the details.

## Overview

### Chassis / Mechanical Design

Currently, I'm using the [Spider robot chassis](https://www.thingiverse.com/thing:1009659) designed by [regishsu](https://www.thingiverse.com/regishsu/designs) on [Thingiverse](https://www.thingiverse.com/). If you own a 3D printer, you can print your own with virtually no cost. Otherwise, you can have it printed for you by an online service such as [CraftCloud](https://craftcloud3d.com/). Last time I checked, they quoted me $27 USD to have someone print it for me in PLA, delivered in 10 - 18 business days.

### Hardware

The goal of this project is affordability. Here is a list of parts and cost:

* [ESP32 MCU](https://www.amazon.com/MELIFE-Development-Dual-Mode-Microcontroller-Integrated/dp/B08BYJR9Y4/) - $14.99 ( 2 pcs )
* [SG90 Servos](https://www.amazon.com/Dorhea-Helicopter-Airplane-Walking-Compatible/dp/B08FJ27Q1H/) - $20.99 ( 12 pcs )
* [4.8V Battery](https://www.amazon.com/Receiver-Battery-Connectors-Capacity-Rechargeable/dp/B08K78JGDG/) - $11.99 ( 1 pc )
* [Perfboard](https://www.amazon.com/Nanlaohu-Perfboard-Breadboard-Electronics-Experiments/dp/B085TB5P4N/) - $6.39
* [2.54mm Pin Headers (Male & Female)](https://www.amazon.com/Glarks-Breakaway-Connectors-Arduino-Prototype/dp/B01GY36RJY/) - $10.98

The total cost of the robot would be $65.34.

If you don't have a 3D printer and opt to use a 3D printing service. You will need to add about $27, and the total comes out to $92.34.

If you opt-in to order the custom PCB from JLCPCB (which is optional if you know how to solder some pins to a perfboard), you will need to add about $20, and you don't need to buy the perfboard and 2.54mm pin headers.

#### ESP32 Micro Controller Board

The brain of the spider robot is a [ESP32 Micro Controller Board](https://www.amazon.com/gp/product/B08DQQ8CBP/) I bought on Amazon. You get 5 of them for $32 USD. If you want less, you can also [buy this one](https://www.amazon.com/MELIFE-Development-Dual-Mode-Microcontroller-Integrated/dp/B08BYJR9Y4/), which will give you 2 of them for $15 USD.

This MCU has Bluetooth and WIFI support. I use Bluetooth to control the robot via a game pad such as PS4 / PS5 controller. WIFI is used to host a small web app where you can tweak various settings for the robot. The web app is not yet implemented. The planned use is to expose a UI for servo calibrations, but other ideas are welcome. If you have some cool ideas, please suggest it by [filing an issue](https://github.com/aq1018/spidy/issues), and I'd be more than happy to read and discuss with you.

#### Servos

Since this is a 4 legged robot with 3 degrees of freedom on each leg, there are 12 servo motors in total. The SG-90 model servos are used in this project. I picked this model because it is very popular among hobby builders and is very very cheap. Again, I bought some cloned ones called [Miuzei SG90 9G Servos](https://www.amazon.com/gp/product/B072V529YD/) from Amazon. They work pretty good for me. They come in 10 pieces, but since we need at least 12 of them, I bought two orders, giving me 20 of them so if some of them break, I can always replace them. There is also [this one on Amazon](https://www.amazon.com/Dorhea-Helicopter-Airplane-Walking-Compatible/dp/B08FJ27Q1H/) comes with 12 of them for $21 USD. However, I have not tried those so I do not know if they are of good quality.

#### Custom PCB (Optional)

You can probably hand solder a perfboard and hook up the ESP32 board, servos, and power together, but I made a [custom PCB](https://oshwlab.com/aq1018/robot) to make things a bit easier for me. This PCB allows you to hook up a 4.8V NIMH battery to power the MCU and the servos connected via the pins on the 4 sides of the board.

I then order 5 pieces of this custom PCB via [JLCPCB](https://jlcpcb.com/). It costed me $35 USD. You can probably get it cheaper with new customer coupons. It can get even cheaper if you opt in to solder the components yourself. But $35 USD for custom made PCB is already dirty cheap in my opinion.

### Battery

I personally bought a [7.4V LiPo Battery](https://www.amazon.com/gp/product/B016ZM3CVA) as the power source. This means the servos are using 7.4V - 8.4V instead of the manufacturer recommended d 4.8V - 6V. I don't think this is a big deal because the servos are so cheap and they seem to be working alright under those voltages, but your milage may vary. I only chose to take the dangerous route because I don't want to spend the money to buy another battery. But if you want it up to spec, then you should probably get a [4.8V NIHM Battery like this one](https://www.amazon.com/Receiver-Battery-Connectors-Capacity-Rechargeable/dp/B08K78JGDG/) instead.

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
