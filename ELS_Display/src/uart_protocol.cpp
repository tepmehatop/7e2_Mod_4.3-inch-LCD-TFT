/**
 * @file uart_protocol.cpp
 * @brief Реализация UART протокола для связи с Arduino Mega
 */

#include "uart_protocol.h"
#include "display_config.h"
#include <string.h>

// ============================================================================
// UartProtocol Implementation
// ============================================================================

UartProtocol::UartProtocol()
    : serial_(nullptr)
    , rx_index_(0)
    , data_update_callback_(nullptr)
    , screen_change_callback_(nullptr)
    , alert_callback_(nullptr)
    , alert_dismiss_callback_(nullptr)
{
    memset(&data_, 0, sizeof(data_));
    memset(rx_buffer_, 0, sizeof(rx_buffer_));

    // Значения по умолчанию
    data_.mode = MODE_FEED;        // Arduino boots in M1 (Feed/Manual)
    data_.submode = SUBMODE_MANUAL;
    data_.thread_mm = 150;  // 1.50 мм
    data_.rpm = 1500;
}

void UartProtocol::begin(HardwareSerial& serial, uint32_t baud_rate)
{
    serial_ = &serial;
    serial_->begin(baud_rate, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

    Serial.println("UART Protocol initialized");
    Serial.printf("RX: GPIO%d, TX: GPIO%d, Baud: %d\n", UART_RX_PIN, UART_TX_PIN, baud_rate);

    // Отправляем READY
    sendReady();
}

void UartProtocol::process()
{
    if (!serial_) return;

    while (serial_->available()) {
        char c = serial_->read();

        if (c == '\n' || c == '\r') {
            if (rx_index_ > 0) {
                rx_buffer_[rx_index_] = '\0';
                parseCommand(rx_buffer_);
                rx_index_ = 0;
            }
        }
        else if (rx_index_ < sizeof(rx_buffer_) - 1) {
            rx_buffer_[rx_index_++] = c;
        }
    }
}

void UartProtocol::parseCommand(const char* cmd)
{
    // Команды имеют формат <CMD:PARAMS>
    if (cmd[0] != '<') return;

    char buffer[128];
    strncpy(buffer, cmd + 1, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Убрать закрывающую скобку
    char* end = strchr(buffer, '>');
    if (end) *end = '\0';

    // Разделить на команду и параметры
    char* colon = strchr(buffer, ':');
    if (!colon) return;

    *colon = '\0';
    const char* command = buffer;
    const char* params = colon + 1;

    // Serial.printf("UART RX: <%s:%s>\n", command, params);

    // Обработка команд
    if (strcmp(command, "MODE") == 0) {
        parseMode(params);
    }
    else if (strcmp(command, "SUBMODE") == 0) {
        parseSubMode(params);
    }
    else if (strcmp(command, "FEED") == 0) {
        parseFeed(params);
    }
    else if (strcmp(command, "AFEED") == 0) {
        parseAFeed(params);
    }
    else if (strcmp(command, "THREAD") == 0) {
        parseThread(params);
    }
    else if (strcmp(command, "POS_Z") == 0) {
        parsePosZ(params);
    }
    else if (strcmp(command, "POS_X") == 0) {
        parsePosX(params);
    }
    else if (strcmp(command, "RPM") == 0) {
        parseRPM(params);
    }
    else if (strcmp(command, "LIMITS") == 0) {
        parseLimits(params);
    }
    else if (strcmp(command, "SCREEN") == 0) {
        parseScreen(params);
    }
    else if (strcmp(command, "PING") == 0) {
        sendReady();  // Отвечаем READY, чтобы Arduino прислал полное состояние
    }
    else if (strcmp(command, "PASS") == 0) {
        parsePass(params);
    }
    else if (strcmp(command, "AP") == 0) {
        parseAp(params);
    }
    else if (strcmp(command, "PH") == 0) {
        parsePh(params);
    }
    else if (strcmp(command, "ANGLE") == 0) {
        parseAngle(params);
    }
    else if (strcmp(command, "CONE") == 0) {
        parseCone(params);
    }
    else if (strcmp(command, "CONE_ANGLE") == 0) {
        parseConeAngle(params);
    }
    else if (strcmp(command, "DIVN") == 0) {
        parseDivN(params);
    }
    else if (strcmp(command, "DIVM") == 0) {
        parseDivM(params);
    }
    else if (strcmp(command, "BAR") == 0) {
        parseBar(params);
    }
    else if (strcmp(command, "PASS_SPHR") == 0) {
        parsePassSphr(params);
    }
    else if (strcmp(command, "SPHERE") == 0) {
        parseSphere(params);
    }
    else if (strcmp(command, "STATE") == 0) {
        parseState(params);
    }
    else if (strcmp(command, "ALERT") == 0) {
        parseAlert(params);
    }
    else if (strcmp(command, "THREAD_NAME") == 0) {
        parseThreadName(params);
    }
    else if (strcmp(command, "RPM_LIM") == 0) {
        parseRpmLim(params);
    }
    else if (strcmp(command, "THREAD_CYCL") == 0) {
        parseThreadCycl(params);
    }
    else if (strcmp(command, "THREAD_TRAVEL") == 0) {
        parseThreadTravel(params);
    }
    else if (strcmp(command, "SELECTMENU") == 0) {
        parseSelectMenu(params);
    }
    else if (strcmp(command, "OTSKOK_Z") == 0) {
        parseOtskokZ(params);
    }
    else if (strcmp(command, "TENSION_Z") == 0) {
        parseTensionZ(params);
    }
    else if (strcmp(command, "DIAM_X") == 0) {
        parseDiamX(params);
    }
    else if (strcmp(command, "PASS_FIN") == 0) {
        parsePassFin(params);
    }
    else if (strcmp(command, "CONE_THR") == 0) {
        parseConeThr(params);
    }
    else if (strcmp(command, "CUTTER_W") == 0) {
        parseCutterW(params);
    }
    else if (strcmp(command, "CUTTING_W") == 0) {
        parseCuttingW(params);
    }
}

void UartProtocol::parseMode(const char* params)
{
    int mode = atoi(params);
    if (mode >= MODE_FEED && mode <= MODE_RESERVE) {
        data_.mode = (LatheMode)mode;
        if (data_update_callback_) {
            data_update_callback_(data_);
        }
    }
}

void UartProtocol::parseSubMode(const char* params)
{
    int submode = atoi(params);
    if (submode >= SUBMODE_INTERNAL && submode <= SUBMODE_EXTERNAL) {
        data_.submode = (LatheSubMode)submode;
        if (data_update_callback_) {
            data_update_callback_(data_);
        }
    }
}

void UartProtocol::parseFeed(const char* params)
{
    data_.feed_mm = atoi(params);
    if (data_update_callback_) {
        data_update_callback_(data_);
    }
}

void UartProtocol::parseAFeed(const char* params)
{
    data_.afeed_mm = atoi(params);
    if (data_update_callback_) {
        data_update_callback_(data_);
    }
}

void UartProtocol::parseThread(const char* params)
{
    data_.thread_mm = atoi(params);
    if (data_update_callback_) {
        data_update_callback_(data_);
    }
}

void UartProtocol::parsePosZ(const char* params)
{
    data_.pos_z = atol(params);
    pos_dirty_ = true;  // рендер по таймеру в loop(), не сразу
}

void UartProtocol::parsePosX(const char* params)
{
    data_.pos_x = atol(params);
    pos_dirty_ = true;
}

void UartProtocol::parseRPM(const char* params)
{
    data_.rpm = atoi(params);
    pos_dirty_ = true;
}

void UartProtocol::parseLimits(const char* params)
{
    // Формат: "1,0,0,1" (L,R,F,B)
    int values[4] = {0};
    sscanf(params, "%d,%d,%d,%d", &values[0], &values[1], &values[2], &values[3]);

    data_.limits.left = values[0];
    data_.limits.right = values[1];
    data_.limits.front = values[2];
    data_.limits.rear = values[3];

    if (data_update_callback_) {
        data_update_callback_(data_);
    }
}

void UartProtocol::parseScreen(const char* params)
{
    int screen = atoi(params);
    if (screen_change_callback_) {
        screen_change_callback_(screen);
    }
}

void UartProtocol::sendCommand(const char* cmd, const char* params)
{
    if (!serial_) return;

    serial_->print('<');
    serial_->print(cmd);
    if (params) {
        serial_->print(':');
        serial_->print(params);
    }
    serial_->println('>');

    Serial.printf("UART TX: <%s%s%s>\n", cmd, params ? ":" : "", params ? params : "");
}

void UartProtocol::sendButtonPress(const char* button_name)
{
    sendCommand("TOUCH", button_name);
}

void UartProtocol::sendReady()
{
    sendCommand("READY");
}

void UartProtocol::sendError(const char* message)
{
    sendCommand("ERROR", message);
}

void UartProtocol::sendPong()
{
    sendCommand("PONG");
}

void UartProtocol::parsePass(const char* params)
{
    // Формат: "nr,total", e.g. "3,10"
    sscanf(params, "%hu,%hu", &data_.pass_nr, &data_.pass_total);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseAp(const char* params)
{
    data_.ap = atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parsePh(const char* params)
{
    data_.ph = (uint8_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseAngle(const char* params)
{
    data_.spindle_angle = atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseCone(const char* params)
{
    data_.cone_idx = (uint8_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseConeAngle(const char* params)
{
    data_.cone_angle = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseDivN(const char* params)
{
    data_.total_tooth = (uint16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseDivM(const char* params)
{
    data_.current_tooth = (uint16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseBar(const char* params)
{
    data_.bar_r = atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parsePassSphr(const char* params)
{
    data_.pass_total_sphr = (uint16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseSphere(const char* params)
{
    data_.sphere_radius = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseState(const char* params)
{
    // STATE:run  → is_running=true
    // STATE:stop → is_running=false
    // STATE:N    → bit 0 = running
    if (strcmp(params, "run") == 0) {
        data_.is_running = true;
    } else if (strcmp(params, "stop") == 0) {
        data_.is_running = false;
    } else {
        int bits = atoi(params);
        data_.is_running = (bits & 1) != 0;
    }
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseAlert(const char* params)
{
    // Типы алертов от Arduino:
    // 1 = УСТАНОВИТЕ УПОРЫ (err_1_flag)
    // 2 = УСТАНОВИТЕ СУППОРТ В ИСХОДНУЮ ПОЗИЦИЮ (err_2_flag)
    // 3 = ОПЕРАЦИЯ ЗАВЕРШЕНА (Complete_flag)
    // 4 = Установите джойстик в нейтральное положение (err_0_flag)
    if (!alert_callback_) return;

    // If params is not purely numeric, treat as literal alert string
    bool is_numeric = (params[0] != '\0') && (params[0] == '-' || (params[0] >= '0' && params[0] <= '9'));
    if (!is_numeric) {
        alert_callback_(params);
        return;
    }

    int type = atoi(params);
    switch (type) {
        case 0: // Скрыть алерт (Arduino сбросил флаги ошибки)
                if (alert_dismiss_callback_) alert_dismiss_callback_();
                break;
        case 1: if (alert_callback_) alert_callback_("\xD0\xA3\xD0\xA1\xD0\xA2\xD0\x90\xD0\x9D\xD0\x9E\xD0\x92\xD0\x98\xD0\xA2\xD0\x95\n\xD0\xA3\xD0\x9F\xD0\x9E\xD0\xA0\xD0\xAB!");
                // УСТАНОВИТЕ УПОРЫ!
                break;
        case 2: if (alert_callback_) alert_callback_("УСТАНОВИТЕ СУППОРТ\nВ ИСХОДНУЮ ПОЗИЦИЮ!");
                // УСТАНОВИТЕ СУППОРТ В ИСХОДНУЮ ПОЗИЦИЮ!
                break;
        case 3: if (alert_callback_) alert_callback_("\xD0\x9E\xD0\x9F\xD0\x95\xD0\xA0\xD0\x90\xD0\xA6\xD0\x98\xD0\xAF\n\xD0\x97\xD0\x90\xD0\x92\xD0\x95\xD0\xA0\xD0\xA8\xD0\x95\xD0\x9D\xD0\x90!");
                // ОПЕРАЦИЯ ЗАВЕРШЕНА!
                break;
        case 4: if (alert_callback_) alert_callback_("Установите джойстик\nв нейтральное\nположение");
                // Установите джойстик в нейтральное положение
                break;
        default:
            Serial.printf("Unknown alert type: %d\n", type);
            break;
    }
}

void UartProtocol::parseThreadName(const char* params)
{
    strncpy(data_.thread_name, params, sizeof(data_.thread_name) - 1);
    data_.thread_name[sizeof(data_.thread_name) - 1] = '\0';
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseRpmLim(const char* params)
{
    data_.rpm_limit = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseThreadCycl(const char* params)
{
    data_.thread_cycles = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseThreadTravel(const char* params)
{
    data_.thread_travel = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseSelectMenu(const char* params)
{
    int sm = atoi(params);
    if (sm >= 1 && sm <= 3) {
        data_.select_menu = (uint8_t)sm;
        if (data_update_callback_) data_update_callback_(data_);
    }
}

void UartProtocol::parseOtskokZ(const char* params)
{
    data_.otskok_z = (int32_t)atol(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseTensionZ(const char* params)
{
    data_.tension_z = (int32_t)atol(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseDiamX(const char* params)
{
    data_.diam_x = (int32_t)atol(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parsePassFin(const char* params)
{
    data_.pass_fin = (uint8_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseConeThr(const char* params)
{
    data_.cone_thr = (atoi(params) != 0);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseCutterW(const char* params)
{
    data_.cutter_w = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

void UartProtocol::parseCuttingW(const char* params)
{
    data_.cutting_w = (int16_t)atoi(params);
    if (data_update_callback_) data_update_callback_(data_);
}

// ============================================================================
// DisplayFormatter Implementation
// ============================================================================

void DisplayFormatter::formatFeed(char* buffer, int16_t feed_mm_x100)
{
    snprintf(buffer, 16, "%d.%02d", feed_mm_x100 / 100, abs(feed_mm_x100 % 100));
}

void DisplayFormatter::formatThread(char* buffer, int16_t thread_mm_x100)
{
    snprintf(buffer, 16, "%d.%02d", thread_mm_x100 / 100, abs(thread_mm_x100 % 100));
}

void DisplayFormatter::formatPosition(char* buffer, int32_t pos_um)
{
    int32_t mm = pos_um / 1000;
    int32_t frac = abs(pos_um % 1000);

    if (pos_um < 0 && mm == 0) {
        snprintf(buffer, 16, "-%ld.%02ld", abs(mm), frac / 10);
    } else if (pos_um > 0) {
        snprintf(buffer, 16, "+%ld.%02ld", mm, frac / 10);
    } else {
        snprintf(buffer, 16, "%ld.%02ld", mm, frac / 10);
    }
}

void DisplayFormatter::formatRPM(char* buffer, uint16_t rpm)
{
    snprintf(buffer, 16, "%d", rpm);
}

const char* DisplayFormatter::getModeName(LatheMode mode)
{
    switch (mode) {
        case MODE_FEED:    return "Синхронная";
        case MODE_AFEED:   return "Асинхронная";
        case MODE_THREAD:  return "Резьба";
        case MODE_CONE_L:  return "Конус <";
        case MODE_CONE_R:  return "Конус >";
        case MODE_SPHERE:  return "Шар";
        case MODE_DIVIDER: return "Делитель";
        case MODE_RESERVE: return "Резерв";
        default:           return "???";
    }
}

const char* DisplayFormatter::getModeShortName(LatheMode mode)
{
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "M%d", (int)mode);
    return buffer;
}

const char* DisplayFormatter::getSubModeName(LatheSubMode submode)
{
    switch (submode) {
        case SUBMODE_INTERNAL: return "Внутренняя";
        case SUBMODE_MANUAL:   return "Ручная";
        case SUBMODE_EXTERNAL: return "Наружная";
        default:               return "???";
    }
}

const char* DisplayFormatter::getSubModeShortName(LatheSubMode submode)
{
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "S%d", (int)submode);
    return buffer;
}
