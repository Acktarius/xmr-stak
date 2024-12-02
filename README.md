# XMR-Stak CCX Fork

A specialized fork of XMR-Stak focused on enhancing the Conceal mining experience.

## Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [Build Instructions](#build-instructions)
- [Usage](#usage)
  - [Initial Setup](#initial-setup)
  - [Daily Operations](#daily-operations)
- [Original Project](#original-project)

## Overview

This project is a fork of XMR-Stak by Fireice-uk and Psychocrypt. Please refer to their [original repository](https://github.com/fireice-uk/xmr-stak) for background information and licensing compliance.

This fork aims to streamline mining operations, particularly for Conceal Network miners (Concealers).

## Key Features

- Conceal (CCX) added to supported cryptocurrencies
- Removed cryptonight_conceal algorithm to prevent confusion
- Simplified pool configuration and validation during setup
- User-friendly GUI interface for Conceal mining operations
- Streamlined pool management and basic mining controls
- An easy ON/OFF approach, to easely launch mining during down time

## Installation Ubuntu/Debian Systems
*in a terminal,*
### Dependencies
```bash
sudo apt install libmicrohttpd-dev libssl-dev cmake build-essential \
    libhwloc-dev pkg-config libjsoncpp-dev libwxgtk3.0-gtk3-dev
```

### Build Instructions

First, clone the repository:
*in a location you won't need root access, if you want to launch with icon*
```bash
git clone https://github.com/conceal-network/conceal-xmr-stak.git
cd conceal-xmr-stak
```

Option 1: Manual Build
```bash
mkdir build
cd build
cmake ..
make install
```
Option 2: Automated Build Script for AMD user
```
./ubuntu_AMD_builder.sh
```

## Usage

### Initial Setup
Binaries will be located in `build/bin` folder
```bash
cd buil/bin
```
First-time configuration:
```bash
./xmr-stak
```
Follow the interactive prompts to configure your mining settings. `Ctrl+C` when your done.

### Dai to day Operations

For regular mining operations, use the GUI interface:
```bash
./xmr-stak-gui-ccx
```
or for even quicker access, create the shortcut:
```bash
sudo ./ubuntu_shortcut_creator.sh
```

---


## Original Project

<table>
    <tr>
        <td align="center"><a href="https://github.com/xmrig/xmrig"><img src="doc/_img/xmrig.png" alt="XMRig"></a></td>
        <td align="center"><a href="https://ragerx.lol"><img src="doc/_img/ragerx.png" alt="RagerX"></a></td>
        <td align="center"><a href="https://github.com/fireice-uk/xmr-stak/tree/xmr-stak-rx/doc/README.md"><img src="doc/_img/rx.png" alt="XMR-Stak-RX"></a></td>
    </tr>
</table>

---

For detailed documentation relative to original projet, visit original [Wiki](doc/README.md).
