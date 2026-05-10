<img width="476" height="962" alt="image_2026-05-09_18-52-16" src="https://github.com/user-attachments/assets/7dc61a7d-6545-4a59-a80d-ced3f7248d74" />
# Arduino CLI Master 🔧

<div align="center">

![C](https://img.shields.io/badge/C-99-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**Профессиональная CLI-утилита для управления Arduino с поддержкой 120+ команд**

[Возможности](#-возможности) • [Установка](#-установка) • [Команды](#-команды) • [Схемы](#-примеры-подключения) • [Протокол](#-протокол-обмена)

</div>

---

## 🎯 Что это?

Arduino CLI Master — это консольная утилита для Windows, предоставляющая интерфейс командной строки для управления Arduino через последовательный порт. Программа поддерживает более 120 различных команд: от базового управления пинами до работы с датчиками, дисплеями, WiFi, SD-картами и даже нейронными сетями.

**Назначение:** Быстрая отладка, тестирование и управление Arduino проектами без необходимости перепрошивки.

---

## ✨ Возможности

### 📊 Статистика
| Параметр | Значение |
|----------|----------|
| Команды | 120+ |
| Поддерживаемые модули | 30+ |
| Протокол | UART (Serial) |
| Скорость | Любая (9600 по умолчанию) |

### 🔌 Поддерживаемые модули

| Категория | Модули |
|-----------|--------|
| 💡 **Выводы** | Digital I/O, Analog I/O, PWM, Servo |
| 📟 **Дисплеи** | LCD 16x2, OLED 128x64, LED Matrix, 7-Segment |
| 📡 **Связь** | WiFi, HTTP, MQTT, RFID, I2C, SPI |
| 🧠 **Сенсоры** | DHT, Ultrasonic, PIR, Gas, Light, Temperature, Humidity |
| 🎮 **Ввод** | Joystick, Keypad, Touch, Button, Potentiometer |
| 🎵 **Аудио** | Buzzer, Tone Generator |
| 💾 **Хранение** | EEPROM, SD Card |
| ⏰ **Время** | RTC (DS3231), Alarm |
| 🚗 **Моторы** | DC Motor, Stepper Motor |
| 🎨 **RGB** | RGB LED, Color Sensor |
| 🧬 **Продвинутые** | Voice Recognition, Face Detection, Neural Networks |

---

## 🚀 Установка

### Требования

| Компонент | Требование |
|-----------|------------|
| ОС | Windows 7/8/10/11 |
| Компилятор | MinGW GCC / MSVC |
| Ардуино | Любая плата (Uno, Nano, Mega, ESP8266, ESP32) |
| Прошивка | Специальная (см. раздел "Прошивка") |

### Компиляция

**MinGW (рекомендуется):**
```bash
gcc -o arduino-cli.exe main.c -std=c99 -lws2_32
