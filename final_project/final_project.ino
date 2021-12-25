#include <Keypad.h>
#include <LiquidCrystal.h>
#include <TimeLib.h>

// peripheral setting
constexpr byte rows = 4;
constexpr byte cols = 4;
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
constexpr byte buzzer_pin = A4;
LiquidCrystal lcd(12, 11, 10, 9, 8, 13);
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, rows, cols);

void setup() {
    lcd.begin(16, 2);
    Serial.begin(9600);
    // Connect to wifi access point
    // Init time from ntp server
}

enum class Mode {
    clock, set_alarm, stopwatch_start, stopwatch_pause, timer_set, timer_start, timer_pause, weather
};
// Mode Cycle: clock -> set_alarm -> stopwatch_pause -> timer_set -> weather

Mode mode = Mode::clock;

void update();

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
                if(key == 'A')
                    mode = Mode::stopwatch_pause;
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
                if(key == 'A')
                    mode = Mode::weather;
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
                if(key == 'A')
                    mode = Mode::weather;
                if(key == 'D')
                    mode = Mode::timer_start;
                break;
            }
            case Mode::timer_start: {
                if(key == 'A') {
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

