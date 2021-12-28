#include <Keypad.h>
#include <LiquidCrystal.h>
#include <TimeLib.h>
// #include <ESP8266WiFi.h>
// #include <WiFiUdp.h>

// peripheral setting
constexpr byte rows {4};
constexpr byte cols {4};
byte colPins[] {7, 6, 5, 4};
byte rowPins[] {A0, A1, A2, A3};
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
constexpr byte buzzer_pin {A4};
// LiquidCrystal lcd(9, 8, 10, 11, 12, 13);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, rows, cols);

void setup() {
    lcd.begin(16, 2);
    Serial.begin(9600);
    // Connect to wifi access point
    // Init time from ntp server
    lcd.clear();
    lcd.print("test");
    lcd.print("test");
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


void lcdUpdate();

struct {
    constexpr static int map2lcd[] {0, 1, 3, 4};
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
        tm.Hour = time[0] - '0' * 10 + time[1] - '0';
        tm.Minute = time[2] - '0' * 10 + time[3] - '0';
        return tm;
    }
} time_format_hm;

struct {
    constexpr static int map2lcd[] {0, 1, 3, 4, 6, 7};
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
        tm.Hour = time[0] - '0' * 10 + time[1] - '0';
        tm.Minute = time[2] - '0' * 10 + time[3] - '0';
        return tm;
    }
} time_format_hms;

bool isBuzzing {false};
void buzz() {

}

void loop() {

    // state machine
    char key {};
    if(key = kp.getKey()) {
        switch(mode) {
            case Mode::clock: {
                if(key == 'A') {
                    mode = Mode::set_alarm;
                    time_format_hm = {};
                }
                if(key == 'D' && isBuzzing) {
                    // stop alarm
                    isBuzzing = false;
                }
                break;
            }
            case Mode::set_alarm: {
                if(key == 'A') {
                    mode = Mode::stopwatch_pause;
                    
                    time_format_hm = {};
                }
                if(key >= '0' && key <= '9') {
                    // replace digit
                    time_format_hm.set_digit(key);
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
                    time_format_hm = {};
                }
                break;
            }
            case Mode::stopwatch_pause: {
                if(key == 'A')
                    mode = Mode::timer_set;
                if(key == 'D')
                    mode = Mode::stopwatch_start;
                break;
            }
            case Mode::stopwatch_start: {
                if(key == 'D')
                    mode = Mode::stopwatch_pause;
                break;
            }
            case Mode::timer_set: {
                if(key == 'A') {
                    time_format_hms = {};
                    mode = Mode::weather;
                    // fetch weather data
                }
                if(key >= '0' && key <= '9') {
                    time_format_hms.set_digit(key);
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
                    time_format_hms = {};
                    timer_millis = makeTime(time_format_hms.get_tm());
                }
                break;
            }
            case Mode::timer_pause: {
                if(key == 'A') {
                    time_format_hms = {};
                    mode = Mode::weather;
                    // fetch weather data
                }
                if(key == 'D')
                    mode = Mode::timer_start;
                break;
            }
            case Mode::timer_start: {
                if(key == 'A') {
                    time_format_hms = {};
                    mode = Mode::weather;
                    // fetch weather data
                }
                if(key == 'D')
                    mode = Mode::timer_pause;
                break;
            }
            case Mode::weather: {
                if(key == 'A')
                    mode = Mode::clock;
                break;
            }
        }
    }
    lcdUpdate();
    buzz();
    delay(100);
}


void lcdUpdate() {
    if(mode == Mode::clock) {
        // Update time on LCD
        // lcd.noBlink();
        lcd.noCursor();
        lcd.setCursor(0, 0);
        

        char buf[20] {};
        sprintf(buf, "%04d-%02d-%02d %s  ", year(), month(), day(), dayShortStr(dayOfWeek(now())));
        lcd.println(buf);

        sprintf(buf, "    %02d:%02d:%02d    ", hour(), minute(), second());
        lcd.println(buf);

    } else if(mode == Mode::set_alarm) {
        // Display date and time with a cursor, let the user set alarm time
        lcd.cursor();

        lcd.setCursor(0, 0);
        // lcd.clear();
        lcd.println("Set alarm time  ");
        // lcd.println("Once|Every|WEEK_OF_DAY");


        const auto& t = time_format_hm.get_tm();
        char buf[20] {};
        sprintf(buf, "     %02d:%02d      ", t.Hour, t.Minute);
        lcd.println(buf);

        auto pos {time_format_hm.offset + time_format_hm.map2lcd[time_format_hm.cursor]};
        lcd.setCursor(pos, 1);

    } else if(mode == Mode::stopwatch_pause) {
        // Display time and text blinking
        lcd.noCursor();

        track_millis = millis();
        if(second() & 1) {
            lcd.setCursor(0, 0);

            char buf[20] {};
            TimeElements tm;
            breakTime((time_t)accum_millis, tm);
            sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
            lcd.println(buf);
        } else {
            lcd.clear();
        }
    } else if(mode == Mode::stopwatch_start) {
        lcd.noCursor();

        auto diff = millis() - track_millis;
        track_millis += diff;
        accum_millis += diff;
        lcd.setCursor(0, 0);

        char buf[20] {};
        TimeElements tm;
        breakTime((time_t)accum_millis, tm);
        sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
        lcd.println(buf);

    } else if(mode == Mode::timer_set) {
        lcd.cursor();

        lcd.setCursor(0, 0);
        // lcd.clear();
        lcd.println("Set timer time  ");


        const auto& t = time_format_hms.get_tm();
        char buf[20] {};
        sprintf(buf, "    %02d:%02d:%02d    ", t.Hour, t.Minute, t.Second);
        lcd.println(buf);

        auto pos {time_format_hms.offset + time_format_hms.map2lcd[time_format_hms.cursor]};
        lcd.setCursor(pos, 1);
    } else if(mode == Mode::timer_pause) {
        lcd.noCursor();

        track_millis = millis();
        if(second() & 1) {
            lcd.setCursor(0, 0);

            char buf[20] {};
            TimeElements tm;
            breakTime((time_t)max(0, timer_millis - accum_millis), tm);
            sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);
            lcd.println(buf);
        } else {
            lcd.clear();
        }
    } else if(mode == Mode::timer_start) {
        lcd.noCursor();

        auto diff = millis() - track_millis;
        track_millis += diff;
        accum_millis += diff;

        lcd.setCursor(0, 0);

        char buf[20] {};
        TimeElements tm;
        breakTime((time_t)max(0, timer_millis - accum_millis), tm);
        sprintf(buf, "    %02d:%02d:%02d    ", tm.Hour, tm.Minute, tm.Second);

        if(accum_millis >= timer_millis) {
            // end state
        }

    } else if(mode == Mode::weather) {
        lcd.clear();
        lcd.println("Weather");
        // Display today's temp/humidity/raining prob etc
    }
}

