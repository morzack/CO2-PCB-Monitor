# CO2-PCB-Monitor

## v1

This repo contains the hardware and software required to manufacture and assemble a small seven segment CO2 monitor.

The monitor is built around a 4 digit seven segment display (with automatic dimming based on ambient light) that displays the current atmospheric CO2 concentration as measured by an Adafruit SCD-40 module over i2c.

![image](https://github.com/user-attachments/assets/6a175729-8cb8-4cba-a6e8-6cd24dcc3875)

In practice, this project can serve as a generic platform for any i2c module that might benefit from a 4 digit display. Potentially, other projects could be built around this architecture.

There is a BOM included: the total cost per unit, as of when it was made in early 2025, was ~90 USD.
Most of the cost comes from the SCD-40 module used -- directly integrating one of the modules onto the board should lower that price.

A rough block diagram is below:

![image](https://github.com/user-attachments/assets/981d0c5c-de65-40db-af4a-f3157fbf6a07)

The system has two power domains: a 5V domain is used for all digital logic, while the 3V domain is reserved for the seven-segement characters and associated dimming circuit.
This separation of power supplies was chosen to both allow for a soft startup sequence (putting less strain on the overall power supply), and to remove some voltage headroom for the LEDs.

The final design was manufactured using JLCPCB's services. A BOM is included with this repo, as well as manufacturing files.

The board can be programmed using the Arduino IDE and a separate ISP over the SPI lines.

## v2

Following the success of the first version, I want to make a few changes to ease manufacturing.
In particular, the design should shift to SMT where possible to enable JLCPCB PCBA.

The SPI and UART headers can be rethought as well to allow for future off-board additions.
There's value in making a port that an ESP32 or similar SoC can clip into for periodic recording over a network.

