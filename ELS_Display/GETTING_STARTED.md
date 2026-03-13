# 🚀 БЫСТРЫЙ СТАРТ - ELS Display

Прошивка для ESP32-2432S024 с дизайнами **#8 (Dark Theme Pro)** и **#3 (Industrial HMI)**

---

## ✅ Что уже готово:

1. ✅ Структура проекта PlatformIO
2. ✅ Конфигурация дисплея 320x240 (горизонтально)
3. ✅ UART протокол связи с Arduino
4. ✅ Базовый UI дизайна #8 (Dark Theme Pro)
5. ✅ Библиотеки LVGL + TFT_eSPI настроены

---

## 📋 ЧТО НУЖНО СДЕЛАТЬ:

### Шаг 1: Открыть проект в VSCode

```bash
cd ELS_Display
Gj bycnh
```

### Шаг 2: Собрать проект

**Через GUI:**
- Нажмите иконку PlatformIO (муравей) слева
- Project Tasks → esp32_2432s024 → General → Build

**Через терминал:**
```bash
pio run
```

**Горячая клавиша:** `Ctrl+Alt+B`

### Шаг 3: Подключить ESP32 через USB

Подключите ESP32-2432S024 к компьютеру через USB кабель.

### Шаг 4: Загрузить прошивку

**Через GUI:**
- Project Tasks → esp32_2432s024 → General → Upload

**Через терминал:**
```bash
pio run -t upload
```

**Горячая клавиша:** `Ctrl+Alt+U`

**Если не загружается:**
1. Нажмите и держите кнопку **BOOT** на ESP32
2. Нажмите **Upload** в VSCode
3. Отпустите BOOT когда увидите "Connecting..."

### Шаг 5: Открыть Serial Monitor

```bash
pio device monitor
```

Или **Ctrl+Alt+S**

Вы должны увидеть:
```
===========================================
ELS Display - Starting...
===========================================
Display: ESP32-2432S024 (320x240)
LVGL initialized
UART Protocol initialized
RX: GPIO16, TX: GPIO17, Baud: 115200
Creating UI Design #8...
===========================================
Setup complete! System ready.
===========================================
```

---

## 🔌 ПОДКЛЮЧЕНИЕ К ARDUINO MEGA

После того как прошивка загружена и работает на ESP32, подключите его к Arduino:

```
Arduino Mega 2560          ESP32-2432S024
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Pin 18 (TX1)        →      GPIO16 (RX)
Pin 19 (RX1)        ←      GPIO17 (TX)
GND                 ━      GND
5V (optional)       →      5V
```

**ВАЖНО:** Обязательно соедините GND!

---

## 🧪 ТЕСТИРОВАНИЕ

### Тест 1: Дисплей работает?

После загрузки прошивки на экране ESP32 должен появиться UI:
- ✅ Черный фон
- ✅ Синяя верхняя панель (toolbar) с "M3 - РЕЗЬБА"
- ✅ Левая панель с большой цифрой "1.50"
- ✅ Правая панель с позициями

**Если экран пустой:**
1. Проверьте что ESP32 получает питание (USB подключен)
2. Покрутите потенциометр яркости (если есть) на модуле
3. Проверьте Serial Monitor - там должны быть сообщения

### Тест 2: UART связь с Arduino

В Arduino прошивке ([7e2_Mod_LCD4.3_v1.3.ino](../7e2_Mod_LCD4.3_v1.3.ino)) нужно добавить код отправки данных на ESP32.

Добавьте в `setup()`:
```cpp
// Инициализация Serial1 для связи с ESP32
Serial1.begin(115200);
delay(100);

// Отправить первичные данные
Serial1.println("<MODE:3>");      // M3 - Thread
Serial1.println("<SUBMODE:2>");   // S2 - Manual
Serial1.println("<THREAD:150>");  // 1.50 мм
Serial1.println("<RPM:1500>");    // 1500 RPM
Serial1.println("<POS_Z:123450>"); // +123.450 мм
Serial1.println("<POS_X:-67890>"); // -67.890 мм
```

Добавьте в `loop()` обновление данных:
```cpp
// Отправлять обороты каждые 100 мс
static unsigned long last_rpm_update = 0;
if (millis() - last_rpm_update > 100) {
    char buf[32];
    snprintf(buf, sizeof(buf), "<RPM:%d>", current_rpm);
    Serial1.println(buf);
    last_rpm_update = millis();
}

// Отправлять позиции каждые 200 мс
static unsigned long last_pos_update = 0;
if (millis() - last_pos_update > 200) {
    char buf[32];
    snprintf(buf, sizeof(buf), "<POS_Z:%ld>", Size_Z_mm * 10);
    Serial1.println(buf);
    snprintf(buf, sizeof(buf), "<POS_X:%ld>", Size_X_mm * 10);
    Serial1.println(buf);
    last_pos_update = millis();
}
```

В Serial Monitor ESP32 вы должны увидеть:
```
UART RX: <MODE:3>
UART RX: <SUBMODE:2>
UART RX: <THREAD:150>
Data updated from Arduino!
Mode: 3, SubMode: 2, RPM: 1500
```

---

## 🎨 ПЕРЕКЛЮЧЕНИЕ ДИЗАЙНОВ

В файле [src/main.cpp](src/main.cpp) в начале есть:

```cpp
// Текущий дизайн (3 или 8)
uint8_t current_design = 8;  // По умолчанию Dark Theme Pro
```

Измените на `3` чтобы использовать Industrial HMI Style:
```cpp
uint8_t current_design = 3;  // Industrial HMI Style
```

Перекомпилируйте и загрузите снова.

---

## 📊 ПРОТОКОЛ UART

### Команды Arduino → ESP32:

| Команда | Формат | Пример | Описание |
|---------|--------|--------|----------|
| MODE | `<MODE:N>` | `<MODE:3>` | Режим M1-M8 (1-8) |
| SUBMODE | `<SUBMODE:N>` | `<SUBMODE:2>` | Подрежим S1-S3 (1-3) |
| THREAD | `<THREAD:NNN>` | `<THREAD:150>` | Резьба 1.50мм (150 = 1.50*100) |
| FEED | `<FEED:NN>` | `<FEED:25>` | Подача 0.25мм/об |
| POS_Z | `<POS_Z:NNNNN>` | `<POS_Z:123450>` | Поз. Z в мкм (123.45мм) |
| POS_X | `<POS_X:NNNNN>` | `<POS_X:-67890>` | Поз. X в мкм (-67.89мм) |
| RPM | `<RPM:NNNN>` | `<RPM:1500>` | Обороты 1500 |
| LIMITS | `<LIMITS:L,R,F,B>` | `<LIMITS:1,0,0,1>` | Лимиты (1=вкл, 0=выкл) |

### Команды ESP32 → Arduino:

| Команда | Формат | Описание |
|---------|--------|----------|
| READY | `<READY>` | ESP32 готов |
| TOUCH | `<TOUCH:BTN>` | Нажата кнопка на экране |
| PONG | `<PONG>` | Ответ на PING |

---

## 🐛 РЕШЕНИЕ ПРОБЛЕМ

### Не компилируется

**Ошибка:** `lvgl.h: No such file or directory`
**Решение:** PlatformIO автоматически скачает библиотеки при первой сборке. Подождите.

**Ошибка:** `TFT_eSPI.h: No such file or directory`
**Решение:** Установите библиотеку: `pio lib install "bodmer/TFT_eSPI@^2.5.43"`

### Не загружается на ESP32

**Решение:**
1. Проверьте что выбран правильный COM порт
2. Нажмите BOOT на ESP32 перед загрузкой
3. Используйте качественный USB кабель

### Экран пустой

**Решение:**
1. Проверьте Serial Monitor - есть ли сообщения?
2. Проверьте что `platformio.ini` имеет правильные настройки дисплея
3. Попробуйте изменить `SCREEN_ROTATION` в `display_config.h`

### UART не работает

**Ре��ение:**
1. Проверьте подключение TX/RX (TX Arduino → RX ESP32)
2. Проверьте что GND соединен
3. Проверьте скорость 115200 на обоих устройствах
4. Смотрите Serial Monitor ESP32 - команды должны выводиться

---

## 📂 СТРУКТУРА ФАЙЛОВ

```
ELS_Display/
├── platformio.ini              # Конфигурация проекта ⚙️
├── include/
│   ├── lv_conf.h              # Настройки LVGL
│   ├── display_config.h       # Конфиг дисплея (легко меняется!)
│   ├── uart_protocol.h        # UART протокол
│   └── ui_common.h            # Общие UI функции
├── src/
│   ├── main.cpp               # 🔥 ГЛАВНЫЙ ФАЙЛ
│   ├── uart_protocol.cpp      # Реализация UART
│   ├── ui_design_3_industrial.cpp  # Дизайн #3
│   └── ui_design_8_darkpro.cpp     # Дизайн #8 (TODO)
└── README.md
```

---

## ✨ ЧТО ДАЛЬШЕ

После того как базовая прошивка заработает:

### 1. Доработка UI
- [ ] Полная реализация дизайна #3 (Industrial HMI)
- [ ] Обработка всех режимов (не только Thread)
- [ ] Динамическое обновление всех параметров
- [ ] Анимации переходов

### 2. Touch управление
- [ ] Добавить обработку touch экрана (XPT2046)
- [ ] Кнопки на экране для управления
- [ ] Отправка команд обратно на Arduino

### 3. Оптимизация
- [ ] Уменьшить частоту обновлений LVGL
- [ ] Оптимизировать буфер отрисовки
- [ ] Добавить дебаунс для UART

### 4. Смена дисплея
Когда придет JC4827W543 (4.3"):
- Изменить в `display_config.h`: `#define DISPLAY_JC4827W543`
- Изменить в `platformio.ini`: `default_envs = jc4827w543`
- **Всё!** Код автоматически адаптируется

---

## 🎯 БЫСТРАЯ ПРОВЕРКА

✅ **Компиляция:** `pio run` должна пройти без ошибок
✅ **Загрузка:** `pio run -t upload` загружает прошивку
✅ **Serial Monitor:** Видны сообщения о старте системы
✅ **Дисплей:** Показывает UI с данными
✅ **UART:** Получает команды от Arduino (видно в Serial Monitor)

Если всё ✅ - **система работает!**

---

**Удачи! Если возникнут вопросы - спрашивайте!** 🚀
