#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>

// peripheral setting
constexpr byte rows {4};
constexpr byte cols {4};
byte rowPins[] {9, 8, 7, 6};
byte colPins[] {5, 4, 3, 2};
char hexaKeys[rows][cols] {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
// A: Mode Change
// B: Up/Left/Previous
// C: Down/Right/Next
// D: Confirm/Pause/Play

// peripherals
constexpr byte buzzer_pin {A0};
// LiquidCrystal lcd(9, 8, 10, 11, 12, 13);
// LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, rows, cols);
LiquidCrystal_I2C lcd(0x27, 16, 2);

constexpr uint32_t init_time {1640775600UL};
void setup() {
    adjustTime(init_time);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    Serial.begin(9600);
    // Connect to wifi access point
    // Init time from ntp server
    lcd.blink();
}





enum class Mode {
    clock, set_alarm, stopwatch_start, stopwatch_pause, timer_set, timer_start, timer_pause, weather
};
// Mode Cycle: clock -> set_alarm -> stopwatch_pause -> timer_set -> weather

Mode mode {Mode::clock};

uint32_t track_millis {};
uint32_t accum_millis {};
uint32_t timer_millis {};
uint32_t alarm_millis {~0UL};

void lcdUpdate();


constexpr int map2lcd[] {0, 1, 3, 4, 6, 7};
struct {
    constexpr static int digits {4};
    char time[digits] {};
    int offset {};
    int cursor {};

    void next_digit() {
        cursor = min(digits - 1, cursor + 1);
    }

    void last_digit() {
        cursor = max(0, cursor - 1);
    }

    void set_digit(char ch) {
        time[cursor] = ch;
    }

    TimeElements get_tm() {
        TimeElements tm;
        tm.Hour = time[0] * 10 + time[1];
        tm.Minute = time[2] * 10 + time[3];
        return tm;
    }
} time_format_hm;

struct {
    constexpr static int digits {6};
    char time[digits] {};
    int offset {};
    int cursor {};

    void next_digit() {
        cursor = min(digits - 1, cursor + 1);
    }

    void last_digit() {
        cursor = max(0, cursor - 1);
    }

    void set_digit(char ch) {
        time[cursor] = ch;
    }

    TimeElements get_tm() {
        TimeElements tm;
        tm.Hour = time[0] * 10 + time[1];
        tm.Minute = time[2] * 10 + time[3];
        tm.Second = time[4] * 10 + time[5];
        return tm;
    }
} time_format_hms;

bool isBuzzing {false};
void buzz() {
    if(isBuzzing) {
        tone(buzzer_pin, 440);
    } else {
        noTone(buzzer_pin);
    }
}

void loop() {

    // state machine
    char key {};
    if(key = kp.getKey()) {
        switch(mode) {
            case Mode::clock: {
                if(key == 'A') {
                    mode = Mode::set_alarm;
                    lcd.clear();
                    time_format_hm = {};
                }
                if(key == 'D') {
                    // stop alarm
                    isBuzzing = false;
                }
                break;
            }
            case Mode::set_alarm: {
                if(key == 'A') {
                    mode = Mode::stopwatch_pause;
                    lcd.clear();
                    time_format_hm = {};
                }
                if(key >= '0' && key <= '9') {
                    // replace digit
                    time_format_hm.set_digit(key - '0');
                    time_format_hm.next_digit();
                }
                if(key == 'B') {
                    // move cursor to previous digit
                    time_format_hm.last_digit();
                }
                if(key == 'C') {
                    // move cursor to next digit
                    time_format_hm.next_digit();
                }
                if(key == 'D') {
                    // save alarm, back to clock
                    const auto& t = time_format_hm.get_tm();
                    alarm_millis = ((uint32_t)t.Hour * 3600UL + (uint32_t)t.Minute * 60UL) * 1000UL;

                    time_format_hm = {};
                    mode = Mode::clock;
                    lcd.clear();
                }
                break;
            }
            case Mode::stopwatch_pause: {
                if(key == 'A') {
                    mode = Mode::timer_set;
                    time_format_hms = {};
                    accum_millis = 0;
                    lcd.clear();
                }
                if(key == 'D') {
                    mode = Mode::stopwatch_start;
                }
                break;
            }
            case Mode::stopwatch_start: {
                if(key == 'A') {
                    mode = Mode::timer_set;
                    time_format_hms = {};
                    accum_millis = 0;
                    lcd.clear();
                }
                if(key == 'D')
                    mode = Mode::stopwatch_pause;
                break;
            }
            case Mode::timer_set: {
                if(key == 'A') {
                    time_format_hms = {};
                    // mode = Mode::weather;
                    mode = Mode::clock;
                    lcd.clear();
                    // fetch weather data
                }
                if(key >= '0' && key <= '9') {
                    time_format_hms.set_digit(key - '0');
                    time_format_hms.next_digit();
                }
                if(key == 'B') {
                    time_format_hms.last_digit();
                }
                if(key == 'C') {
                    time_format_hms.next_digit();
                }
                if(key == 'D') {
                    mode = Mode::timer_start;
                    accum_millis = 0;
                    lcd.clear();
                    const auto& t = time_format_hms.get_tm();
                    timer_millis = ((uint32_t)t.Hour * 3600UL + (uint32_t)t.Minute * 60UL + (uint32_t)t.Second) * 1000UL;
                    // timer_millis = (makeTime(time_format_hms.get_tm()) - 641410304UL) * 1000UL;
                    time_format_hms = {};
                }
                break;
            }
            case Mode::timer_pause: {
                if(key == 'A') {
                    time_format_hms = {};
                    accum_millis = 0;
                    // mode = Mode::weather;
                    mode = Mode::clock;
                    lcd.clear();
                    // fetch weather data
                }
                if(key == 'D') {
                    mode = isBuzzing ? Mode::clock : Mode::timer_start;
                    isBuzzing = false;
                }
                break;
            }
            case Mode::timer_start: {
                if(key == 'A') {
                    time_format_hms = {};
                    accum_millis = 0;
                    // mode = Mode::weather;
                    mode = Mode::clock;
                    // fetch weather data
                }
                if(key == 'D') {
                    mode = Mode::timer_pause;
                    isBuzzing = false;
                }
                break;
            }
        }
    }
    lcdUpdate();
    buzz();
    delay(100);
}


void lcdUpdate() {
    auto diff = millis() - track_millis;
    // detect alarm time
    const auto& prev_millis = (init_time % 86400) * 1000 + track_millis;
    const auto& now_millis = (init_time % 86400) * 1000 + track_millis + diff;
    if(prev_millis <= alarm_millis && alarm_millis <= now_millis) {
        isBuzzing = true;
    }
    // Serial.println(now_millis);
    // Serial.println(alarm_millis);
    // Serial.println();

    track_millis += diff;

    if(mode == Mode::clock) {
        // Update time on LCD
        // lcd.noBlink();
        lcd.noCursor();
        lcd.setCursor(0, 0);
        

        char buf[20] {};
        sprintf(buf, "%04d-%02d-%02d %s  ", year(), month(), day(), dayShortStr(dayOfWeek(now())));
        lcd.println(buf);

        lcd.setCursor(0, 1);
        sprintf(buf, "    %02d:%02d:%02d    ", hour(), minute(), second());
        lcd.println(buf);

    } else if(mode == Mode::set_alarm) {
        // Display date and time with a cursor, let the user set alarm time
        time_format_hm.offset = 5;
        lcd.cursor();

        lcd.setCursor(0, 0);
        // lcd.clear();
        lcd.println("Set alarm time  ");
        // lcd.println("Once|Every|WEEK_OF_DAY");

        lcd.setCursor(0, 1);
        const auto& t = time_format_hm.get_tm();
        char buf[20] {};
        sprintf(buf, "     %02d:%02d      ", t.Hour, t.Minute);
        lcd.println(buf);

        auto pos {time_format_hm.offset + map2lcd[time_format_hm.cursor]};
        lcd.setCursor(pos, 1);

    } else if(mode == Mode::stopwatch_pause) {
        // Display time and text blinking
        lcd.noCursor();
        if(second() & 1) {
            lcd.setCursor(0, 0);

            char buf[20] {};
            TimeElements tm;
            breakTime((time_t)(accum_millis / 1000), tm);
            sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
            lcd.println(buf);
        } else {
            lcd.clear();
        }
    } else if(mode == Mode::stopwatch_start) {
        lcd.noCursor();

        accum_millis += diff;
        lcd.setCursor(0, 0);

        char buf[20] {};
        TimeElements tm;
        breakTime((time_t)(accum_millis / 1000), tm);
        sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
        lcd.println(buf);

    } else if(mode == Mode::timer_set) {
        time_format_hms.offset = 4;
        lcd.cursor();

        lcd.setCursor(0, 0);
        // lcd.clear();
        lcd.println("Set timer time  ");
        lcd.setCursor(0, 1);

        const auto& t = time_format_hms.get_tm();
        char buf[20] {};
        sprintf(buf, "    %02d:%02d:%02d    ", t.Hour, t.Minute, t.Second);
        lcd.println(buf);

        auto pos {time_format_hms.offset + map2lcd[time_format_hms.cursor]};
        lcd.setCursor(pos, 1);
    } else if(mode == Mode::timer_pause) {
        time_format_hms.offset = 4;
        lcd.noCursor();

        if(second() & 1) {
            lcd.setCursor(0, 0);

            char buf[20] {};
            TimeElements tm;
            breakTime((time_t)max(0, (((int32_t)timer_millis - (int32_t)accum_millis) / 1000L)), tm);
            sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
            lcd.println(buf);

        } else {
            lcd.clear();
        }
    } else if(mode == Mode::timer_start) {
        time_format_hms.offset = 4;
        lcd.noCursor();

        accum_millis += diff;

        lcd.setCursor(0, 0);

        char buf[20] {};
        TimeElements tm;
        breakTime((time_t)max(0, (((int32_t)timer_millis - (int32_t)accum_millis) / 1000L)), tm);
        sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
        lcd.println(buf);

        if(accum_millis >= timer_millis) {
            // end state
            isBuzzing = true;
            mode = Mode::timer_pause;
        }

    }


    // Serial.println(accum_millis);
    // Serial.println(timer_millis);
    // Serial.println(((int32_t)timer_millis - (int32_t)accum_millis) / 1000L);
    // Serial.println();
}

