//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ***** DRO HS800-2 — Чтение цифровых линеек через UART ***** //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Этот файл активен ТОЛЬКО если в 7e2_Mod_LCD4.3_v1.3.ino раскомментирован:
//   #define USE_DRO_HS800
//
// ПОДКЛЮЧЕНИЕ к Arduino Mega:
//   HS800-2 STM32 TX (резистор R78/R79/R80) ──── Mega pin 15 (RX3)
//   HS800-2 GND ─────────────────────────────── Mega GND
//   (TX3 pin 14 — не используется, провод не нужен)
//
//   Используется Serial3 (hardware UART) — свободный, надёжный, без конфликтов.
//   SoftwareSerial (pins 51/52) для дисплея JC4827W543 — не затронут.
//
// ПРОТОКОЛ (подтверждён реверс-инжинирингом, репо: github.com/tepmehatop/HS-800-2-analyze):
//   UART 57600/8N1, CRC16/MODBUS, фрейминг: SOF=0xFE, EOF=0xEF
//   Пакет тип 0x33, 29 байт (STM32 → SWM34S, каждые ~38мс):
//     byte[0]      = 0xFE  SOF
//     byte[1]      = 0x33  тип
//     byte[6][7]   = X позиция (signed int16 little-endian, единицы 0.001мм)
//     byte[10][11] = Y позиция (signed int16 little-endian, единицы 0.001мм)
//     byte[18][19] = код кнопки (0x00 0x00 = нет нажатия)
//     byte[26][27] = CRC16/MODBUS от byte[1..25]
//     byte[28]     = 0xEF  EOF
//
// МАППИНГ ОСЕЙ:
//   HS800-2 X → гитара X (поперечная, суппорт)
//   HS800-2 Y → гитара Z (продольная, каретка)  ← в проекте называется Size_Z_mm
//   (оси Z на этом станке нет)
//
// ЗНАК позиции: по умолчанию DRO_X_SIGN и DRO_Y_SIGN = +1.
//   Если позиция движется в обратную сторону — измени на -1.
//
// ЕДИНИЦЫ:
//   DRO даёт позицию в 0.001мм (микрометры).
//   Size_X_mm / Size_Z_mm в гитаре — в 0.01мм.
//   Конвертация: Size_X_mm = dro_pos_x / 10
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_DRO_HS800

// ==================== НАСТРОЙКИ ====================
#define DRO_SERIAL    Serial3   // RX3=pin15, TX3=pin14
#define DRO_BAUD      57600

// Знак направления осей. Измени на -1 если ось движется в обратную сторону.
#define DRO_X_SIGN    1
#define DRO_Y_SIGN    1
// ====================================================

// ==================== ПУБЛИЧНЫЕ ПЕРЕМЕННЫЕ ====================
// Текущие позиции в единицах 0.001мм (signed, обновляются в DRO_Process)
int32_t dro_pos_x = 0;   // HS800-2 ось X
int32_t dro_pos_y = 0;   // HS800-2 ось Y (= гитара Z)

// Последнее нажатие кнопки (0 если нет)
uint8_t dro_btn_b18 = 0;
uint8_t dro_btn_b19 = 0;
bool    dro_btn_new = false;  // true на один цикл loop() после нового нажатия
// ==============================================================

// ==================== ПРИВАТНЫЕ ====================
#define DRO_SOF    0xFE
#define DRO_EOF    0xEF
#define DRO_MAXPKT 32

static uint8_t  s_dro_buf[DRO_MAXPKT];
static uint8_t  s_dro_len = 0;
static bool     s_dro_frame = false;
static uint32_t s_dro_last_pkt_ms = 0;

// ==================== CRC16/MODBUS ====================
static uint16_t _dro_crc16(const uint8_t* data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
    return crc;
}

// ==================== РАЗБОР ПАКЕТА ====================
static void _dro_handle_packet(const uint8_t* p, uint8_t len) {
    if (len < 3 || p[0] != DRO_SOF || p[len-1] != DRO_EOF) return;
    if (p[1] != 0x33 || len != 29) return;   // нас интересует только тип 0x33, 29 байт

    // Проверка CRC (байты [1..25])
    uint16_t crc_calc = _dro_crc16(&p[1], 25);
    uint16_t crc_recv = p[26] | ((uint16_t)p[27] << 8);
    if (crc_calc != crc_recv) return;   // битый пакет — игнорируем

    // Позиции (signed int16 little-endian, 0.001мм)
    int16_t raw_x = (int16_t)(p[6]  | ((uint16_t)p[7]  << 8));
    int16_t raw_y = (int16_t)(p[10] | ((uint16_t)p[11] << 8));

    dro_pos_x = (int32_t)raw_x * DRO_X_SIGN;
    dro_pos_y = (int32_t)raw_y * DRO_Y_SIGN;

    // Кнопка
    uint8_t b18 = p[18], b19 = p[19];
    if ((b18 || b19) && (b18 != dro_btn_b18 || b19 != dro_btn_b19)) {
        dro_btn_b18 = b18;
        dro_btn_b19 = b19;
        dro_btn_new = true;
    } else if (!b18 && !b19) {
        dro_btn_b18 = 0;
        dro_btn_b19 = 0;
    }

    s_dro_last_pkt_ms = millis();
}

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
void DRO_Init() {
    DRO_SERIAL.begin(DRO_BAUD);
    Serial.println(F("DRO HS800-2: Serial3 @ 57600 (pin 15 RX3)"));
}

// ==================== ОБРАБОТКА (вызывать в loop()) ====================
void DRO_Process() {
    dro_btn_new = false;   // сбрасываем флаг каждый цикл

    while (DRO_SERIAL.available()) {
        uint8_t b = DRO_SERIAL.read();
        if (b == DRO_SOF) {
            s_dro_buf[0] = b;
            s_dro_len = 1;
            s_dro_frame = true;
        } else if (s_dro_frame) {
            if (s_dro_len < DRO_MAXPKT) s_dro_buf[s_dro_len++] = b;
            if (b == DRO_EOF) {
                _dro_handle_packet(s_dro_buf, s_dro_len);
                s_dro_len = 0;
                s_dro_frame = false;
            } else if (s_dro_len >= DRO_MAXPKT) {
                s_dro_len = 0;
                s_dro_frame = false;
            }
        }
    }
}

// ==================== УТИЛИТЫ ====================
// Время с момента последнего валидного пакета (мс). >500 = нет связи с HS800-2.
uint32_t DRO_LastPacketAge() {
    return millis() - s_dro_last_pkt_ms;
}

bool DRO_IsConnected() {
    return (s_dro_last_pkt_ms > 0) && (DRO_LastPacketAge() < 500);
}

#endif // USE_DRO_HS800
