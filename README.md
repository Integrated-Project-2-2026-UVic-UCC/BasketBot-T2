# 🏀 BasketBot

**Smart Basketball Hoop — Embedded Scoring & Game System**

An ESP32-powered physical basketball scoreboard with motorized height adjustment, multi-player game modes, and a WiFi-hosted mobile web app.

---

## 📋 Overview

BasketBot combines embedded hardware (IR sensors, dual TM1637 displays, NEMA 17 stepper motor, LEDs) with a browser-based remote control interface. The ESP32 serves as both the game controller and a WiFi access point, allowing players to configure games and control the hoop from their phones.

### Key Features
- **4 Game Modes**: 1vs1, Game 21 (1–4 players), Shoot Out, and Free Practice
- **Dual IR Basket Detection**: TSOP58438 sensors detect ball passing through hoop and net with rebound filtering
- **Motorized Height Adjustment**: NEMA 17 stepper with TMC2209 driver
- **Dual Seven-Segment Displays**: TM1637-based time and score readouts
- **Mobile Web App**: Served directly by the ESP32 at `192.168.4.1`
- **Edge-Detected Scoring**: Reliable basket detection with 600 ms cooldown logic
- **Non-Blocking Homing**: Limit-switch-based calibration with automatic backoff

---

## 🎮 Game Modes

| Mode | Players | Description |
|------|---------|-------------|
| **1vs1** | 2 | Classic head-to-head match with timer |
| **Game 21** | 1–4 | First to 21 points wins; supports up to 4 players |
| **Shoot Out** | 1-4 | Timed shooting challenge |
| **Free Practice** | Any | Practice mode with live score tracking |

---

## 🔌 Hardware Architecture

### Microcontroller
- **ESP32 DevKit** (standard development board; IO pin labels map directly to GPIO numbers)

### IR Basket Detection System (New in v2.0)
Dual TSOP58438 sensors with a 38 kHz IR LED emitter for reliable ball detection through the hoop.

| Component | GPIO | Type | Description |
|-----------|------|------|-------------|
| Upper Sensor (hoop) | 15 | INPUT | TSOP58438 — detects ball at rim level |
| Lower Sensor (net) | 13 | INPUT | TSOP58438 — detects ball passing through net |
| IR LED Emitter | 12 | PWM OUT | 940 nm IR LED driven at 38 kHz |

**Detection Logic:**
- Ball must break the upper sensor beam first, then the lower sensor within `600 ms`
- Rebounds (upper sensor only) are filtered out with a timeout
- Shots from below the hoop are ignored
- 600 ms cooldown prevents double-counting

### Displays
| Display | Purpose | CLK | DIO |
|---------|---------|-----|-----|
| TM1637 #1 | Timer | GPIO 26 | GPIO 4 |
| TM1637 #2 | Score | GPIO 14 | GPIO 27 |

### Motor System
| Component | Pin | Notes |
|-----------|-----|-------|
| TMC2209 STEP | GPIO 32 | Step pulses |
| TMC2209 DIR | GPIO 33 | Direction |
| TMC2209 EN | GPIO 25 | Active LOW |
| Limit Switch | GPIO 35 | INPUT_PULLUP |

&gt; **Note:** GPIO 36 was avoided because it lacks internal pull-up on ESP32. GPIO 34/35 are input-only on some boards.

### Game Buttons & LEDs
| Component | GPIO | Mode | Notes |
|-----------|------|------|-------|
| Button Team A | 16 | INPUT_PULLUP | Confirm basket (1vs1) |
| Button Team B | 17 | INPUT_PULLUP | Confirm basket (1vs1) |
| Button Ready | 16 | INPUT_PULLUP | Shared with Team A button |
| Green LED | 21 | OUTPUT | Basket pending / status |
| Blue LED | 19 | OUTPUT | Team A scored |
| Red LED | 18 | OUTPUT | Team B scored |

### Power Supply
- **12V 5A** supply recommended for stability
- **LM2596** buck converter steps 12V → 5V for ESP32 **VIN** pin
- **Never connect 5V to the 3.3V pin**

---

## 📡 Web App

The ESP32 creates a WiFi access point:

- **SSID**: `BasketBot`
- **IP Address**: `192.168.4.1`

### Features
- 4-mode grid layout for game selection
- Drum-style pickers for player count and settings
- Motor setup card with slider and quick-select height buttons
- Mobile-first responsive design

---

## 🗂️ Project Structure

### Naming Convention
All game logic files use mode-specific prefixes to avoid naming conflicts in the global Arduino scope:
- `_g1_` → 1vs1
- `_g21_` → Game 21
- `_gp_` / `_so_` / `_mc_` → Other modes

---

## 🔧 Installation & Setup

### Prerequisites
- Arduino IDE with ESP32 board support
- [TM1637Display](https://github.com/avishorp/TM1637) library by Avishay Orpaz (via Library Manager)
