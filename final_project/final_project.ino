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
}





enum class Mode {
    clock, set_alarm, stopwatch_start, stopwatch_pause, timer_set, timer_start, timer_pause, weather
};
// Mode Cycle: clock -> set_alarm -> stopwatch_pause -> timer_set -> weather

Mode mode {Mode::clock};

time_t time;
long init_millis;

void update();

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
} time_format_hm;

struct {
    constexpr static int map2lcd[] {0, 1, 3, 4, 6, 7};
    constexpr static int digits {6};
    int offset {};
    int cursor {};
} time_format_hms;

void loop() {

    // state machine
    char key {};
    if(key = kp.getKey()) {
        switch(mode) {
            case Mode::clock: {
                if(key == 'A')
                    mode = Mode::set_alarm;
                break;
            }
            case Mode::set_alarm: {
                if(key == 'A') {
                    mode = Mode::stopwatch_pause;
                    time_format_hm = {};
                }
                if(key >= '0' && key <= '9') {
                    // replace digit
                    
                }
                if(key == 'B') {
                    
                    // move cursor to previous digit
                }
                if(key == 'C') {
                    // move cursor to next digit
                }
                if(key == 'D') {
                    // save alarm, back to clock
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

                }
                if(key == 'B') {
                    
                }
                if(key == 'C') {

                }
                if(key == 'D') {
                    mode = Mode::timer_start;
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

            default:
                break;
        }
    }
    update();
    delay(100);
}


void update() {
    if(mode == Mode::clock) {
        // Update time on LCD
        // lcd.noBlink();
        lcd.noCursor();
        lcd.setCursor(0, 0);
        
        char buf[20] {};
        sprintf(buf, "");
        lcd.println(buf);

        sprintf(buf, "");
        lcd.println(buf);

    } else if(mode == Mode::set_alarm) {
        

        // Display date and time with a cursor, let the user set alarm time
    } else if(mode == Mode::stopwatch_pause) {

    } else if(mode == Mode::stopwatch_start) {
        // Display time with 0 initially, starts when user pressed, stop when user pressed again
    } else if(mode == Mode::timer_start) {
        // Display time with 5 mins initially with cursor, starts counting down when user pressed
    } else if(mode == Mode::weather) {
        // Display today's temp/humidity/raining prob etc
    }
}

