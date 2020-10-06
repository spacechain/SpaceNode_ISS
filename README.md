# ISS Space Node software
Version 1.0

## Background

Our Space Node software is used to perform multisignature transactions through the International Space Station. 

## Hardware introduction

Our hardware is based on GomSpace NanoMind Z7000: 
* Xilinx XC7Z030 
* dual-core ARM Cortex-A9
* Kintex-7 FPGA
* 1 GB DDR3 memory
* 64 MB QSPI NOR flash
* 4 GB eMMC flash 
* I2C、CAN、Ethernet

## Software introduction

Space Node software is operated on the International Space Station. The main functions include: 
* Ground station verification
* On-orbit key generation 
* Transactions authentication 
* Satellite public key update 
* Client private key update

## Compile

* Executing make command
* Generating executable file under bin of Space Node


