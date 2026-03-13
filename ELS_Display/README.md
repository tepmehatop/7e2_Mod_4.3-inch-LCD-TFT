# ELS Display - ESP32 GUI для электронного лидера

**Текущий дисплей:** ESP32-2432S024 (2.4" 240x320)
**Будущий дисплей:** JC4827W543 (4.3" 480x272)

---

## 📁 Структура проекта

```
ELS_Display/
├── platformio.ini          # Конфигурация PlatformIO
├── include/
│   ├── lv_conf.h          # Конфигурация LVGL
│   ├── display_config.h   # Конфигурация дисплея (легко меняется!)
│   └── uart_protocol.h    # Протокол UART с Arduino
├── src/
│   ├── main.cpp           # Основной код
│   ├── uart_protocol.cpp  # Реализация UART
│   └── ui_design_*.cpp    # Файлы дизайнов UI
└── README.md
```

---

## 🚀 Быстрый старт

### 1. Открыть проект в VSCode

```bash
cd ELS_Display
code .
```

### 2. Собрать проект

**Через GUI:**
- Нажмите на иконку PlatformIO (муравей) слева
- Project Tasks → env:esp32_2432s024 → General → Build

**Через терминал:**
```bash
pio run
```

**Горячая клавиша:** `Ctrl+Alt+B`

### 3. Загрузить на ESP32

**Подключите ESP32 через USB**, затем:

**Через GUI:**
- Project Tasks → env:esp32_2432s024 → General → Upload

**Через терминал:**
```bash
pio run -t upload
```

**Горячая клавиша:** `Ctrl+Alt+U`

### 4. Открыть Serial Monitor

**Горячая клавиша:** `Ctrl+Alt+S`

или

```bash
pio device monitor
```

---

## 🔌 Подключение к Arduino Mega

```
Arduino Mega 2560          ESP32-2432S024
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Pin 18 (TX1)        →      RX (GPIO16)
Pin 19 (RX1)        ←      TX (GPIO17)
GND                 ━      GND
5V (optional)       →      5V
```

**ВАЖНО:** Обязательно соедините GND!

---

## 🎨 Смена дизайна

Дизайны находятся в файлах `src/ui_design_*.cpp`

Чтобы выбрать другой дизайн, измените в `src/main.cpp`:

```cpp
// Текущий дизайн
#define CURRENT_DESIGN 1  // Дизайн #1

// Измените на другой
#define CURRENT_DESIGN 5  // Дизайн #5
```

---

## 🔄 Смена дисплея (когда придет новый)

### В файле `include/display_config.h` измените:

```cpp
// Закомментируйте старый
// #define DISPLAY_ESP32_2432S024

// Раскомментируйте новый
#define DISPLAY_JC4827W543
```

### В файле `platformio.ini` измените:

```ini
# Закомментируйте [env:esp32_2432s024]
# Раскомментируйте [env:jc4827w543]

[platformio]
default_envs = jc4827w543  # Было: esp32_2432s024
```

**Всё!** Код автоматически адаптируется под новый размер благодаря макросам `SCALE_*`.

---

## 📊 Протокол UART

### Команды Arduino → ESP32:

```
<MODE:3>              - Режим M3
<SUBMODE:2>           - Подрежим S2
<THREAD:150>          - Резьба 1.50 мм
<POS_Z:123450>        - Позиция Z = 123.450 мм
<POS_X:-67890>        - Позиция X = -67.890 мм
<RPM:1500>            - Обороты 1500
<LIMITS:1,0,0,1>      - Лимиты L,R,F,B
```

### Команды ESP32 → Arduino:

```
<TOUCH:BTN_UP>        - Нажата кнопка UP
<TOUCH:BTN_DOWN>      - Нажата кнопка DOWN
<TOUCH:BTN_SELECT>    - Нажата кнопка SELECT
<READY>               - ESP32 готов
```

---

## 🐛 Отладка

### Включить отладку в Serial Monitor:

В `src/main.cpp` раскомментируйте:

```cpp
#define DEBUG_UART
```

Тогда будут выводиться все принятые/отправленные команды.

---

## 📚 Полезные ссылки

- [LVGL Documentation](https://docs.lvgl.io/)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [PlatformIO Docs](https://docs.platformio.org/)

---

**Готово к работе!** 🚀
