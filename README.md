# My Arduino project: an automated plant watering system

Welcome to my side project: an automated plant watering station!

> [!note]
> The main purpose of this project is to put my knowledge about embedded software development into practice. The project in itself is really basic and is mostly intended for educational purpose.

## Context

I recently got into cooking and boy do I love basil 🌿! Sadly, I don't have a green thumb at all. I've tried to grow my own basil but failed to water it on time and the result is always the same: it dies after a few days... But that time is over!

At first, I thought that I could maybe buy a system that does it for me such as an [Auk mini](https://www.auk.com/products/auk-mini) but then I realized I could actually do it my self and it would be much much cheaper! It doesn't need to be fancy after all!

## Hardware

At its core, the whole automated plant watering station is separated in 3 units:

- The Arduino board
- The water pump controlled by a relay module
- The moisture sensor

### The development board

#### Development board requirements

The micro-controller board will need to read data from a sensor, and activate/deactivate a water pump in reaction.

A key characteristic for the chip is a low power consumption since it will be running on a cell battery. Being able to communicate with the user through WiFi or Bluetooth is appreciated as it might be a further improvement on the project.

#### Development board selection

I always heard about the Arduino boards as good development boards for prototyping and quick development. I am more familiar with Raspberry devices but I consider them more as "mini computer" whereas Arduino are more embedded development boards. So, my choice went for Arduino for the simplicity and the quick development.

After benchmarking the different Arduino boards, I chose the ***Arduino Nano ESP32***
for several reasons:

- My project is pretty small and meant as a hobby and the Arduino Nano family proposes the cheapest boards
- The Nano boards have a low consumption so they are well suited for this project
- The integration of FreeRTOS is really easy on an ESP32 chip.
- The ESP32 chip also provides WiFi and BLE connectivity.
- *Espressif* provides a lot of documentations and integrates works well with the Arduino IDE

### The water pump

#### The water pump requirements


#### The water pump selection

For the water pump I took a basic [5V water pump](https://fr.aliexpress.com/item/1005008972772137.html?spm=a2g0o.order_list.order_list_main.28.28e65e5bmDyd49&gatewayAdapt=glo2fra).

As power source, I took a basic 9V-6LP3146 battery cell, but since the pump work on a 5V DC voltage, I also took this [ASM1117 5V DC buck converter](https://fr.aliexpress.com/item/4000936086305.html?spm=a2g0o.cart.0.0.1882378dEqeYVb&mp=1&pdp_npi=6%40dis%21EUR%21EUR%200.98%21EUR%200.98%21%21EUR%200.98%21%21%21%40211b813f17701280661811579e1238%2110000011293840721%21ct%21FR%217079097145%21%211%210%21&gatewayAdapt=glo2fra).

And finally to command this power chain from the Arduino board I used this [relay module](https://fr.aliexpress.com/item/1005009446541758.html?spm=a2g0o.productlist.main.31.364523a0g4z2tf&algo_pvid=a48a385a-abb8-4102-bcbb-331ae478020c&algo_exp_id=a48a385a-abb8-4102-bcbb-331ae478020c-28&pdp_ext_f=%7B%22order%22%3A%2238%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21EUR%217.83%217.39%21%21%2162.63%2159.12%21%40210390c917701282096084667eb260%2112000049126956111%21sea%21FR%217079097145%21X%211%210%21n_tag%3A-29911%3Bd%3A4b7447c7%3Bm03_new_user%3A-29895&curPageLogUid=kSDn2vay7Khh&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005009446541758%7C_p_origin_prod%3A).

### The moisture sensor

#### The moisture sensor requirements

It exist 2 types of moisture sensor: resistive sensors which are cheaper but don't really last over time, and capacitive sensors. It wanted to take a resistive sensor because this project is more educational than functional, so I don't mind having to change the sensor from time to time.

#### The moisture sensor selection

But I actually didn't find a resistive sensor so I instead took this [capacitive moisture sensor](https://fr.aliexpress.com/item/1005008744986367.html?spm=a2g0o.order_list.order_list_main.23.5c765e5boykon8&gatewayAdapt=glo2fra).

## Software

The software aspect of the project consists of several incremental steps:

- **Step 1**: Program in the Arduino framework and provide a Proof of Concept.
- **Step 2**: Implement a real-time OS (likely FreeRTOS)
- **Step 3**: Rewrite code in Rust.

### Step 1: Programming in the Arduino framework

As I have never developed in the Arduino environment, I first took some time to get familiar with the Arduino IDE and Arduino language (which is basically a wrapper around the C++ language).
Useful information can be found in the [Arduino Docs](https://docs.arduino.cc/software/ide/?_gl=1*w9cg38*_up*MQ..*_ga*MTM4NDg1Mzk3NS4xNzcwMTIzNDg0*_ga_NEXN8H46L5*czE3NzAxMjM0ODIkbzEkZzEkdDE3NzAxMjQxODQkajYwJGwwJGg3Mzk4MTg1Ng..).

Especially documentations about the [Arduino Nano ESP32](https://docs.arduino.cc/hardware/nano-esp32/?_gl=1*m685kj*_up*MQ..*_ga*Mjg3MjQzNTI5LjE3NzAxMjM0ODA.*_ga_NEXN8H46L5*czE3NzAxMjYxODUkbzIkZzEkdDE3NzAxMjYyODIkajU2JGwwJGgxNTY2MDE5MTUz), the [Arduino IDE](https://docs.arduino.cc/software/ide/?_gl=1*8hp86n*_up*MQ..*_ga*Mjg3MjQzNTI5LjE3NzAxMjM0ODA.*_ga_NEXN8H46L5*czE3NzAxMjYxODUkbzIkZzEkdDE3NzAxMjYzNzMkajU4JGwwJGgxNTY2MDE5MTUz) and the [Arduino language](https://docs.arduino.cc/language-reference/?_gl=1*1or0s5r*_up*MQ..*_ga*MTIwNjE3MDUzNi4xNzcwMTI2Mzc4*_ga_NEXN8H46L5*czE3NzAxMjYzNzckbzEkZzEkdDE3NzAxMjY0MTIkajI1JGwwJGg0ODg5MTU4NzM.).

## Improvements already implemented

None so far.

## Future improvements

- A green LED to indicate the pump is on
- A switch to override the dryness threshold and force the activation of the pump
- Add a screen to display information about the sensors value
- Add a potentiometer to let the user adjust the threshold representing the soil dryness.
- Add a sensor to indicate when the water level in the tank is hitting low.
- Add a UV lamp to the system to come up with a humble but functional equivalent
to the [Auk Mini](https://www.auk.com/products/auk-mini).
- Shield the system into a plastic box. Maybe design one is 3D using 
[SketchUp](https://sketchup.trimble.com/fr?srsltid=AfmBOor1Zja2f0j-pDvHCqfTN2BXYjokowDQQhZg4QRR71f0tR67cjy_).
