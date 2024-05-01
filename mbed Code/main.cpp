#include "mbed.h"
#include "uLCD_4DGL.h"
#include "string"
#include "rtos.h"
#include "math.h"
#include <climits>

DigitalIn clap(p19);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalOut ss1(p24);
DigitalOut ss2(p23);
DigitalOut ss3(p22);
DigitalOut ss0(p21);

DigitalOut disp7(p7);
DigitalOut disp6(p8);
DigitalOut disp5(p6);
DigitalOut disp4(p5);
DigitalOut disp3(p20);
DigitalOut disp2(p18);
DigitalOut disp1(p17);
DigitalOut disp0(p16);

uLCD_4DGL take(p9, p10, p11);
uLCD_4DGL scene(p13, p14, p15);
uLCD_4DGL roll(p28, p27, p26);

volatile int state = 0;
Mutex roll_lock;
Mutex scene_lock;
Mutex take_lock;
Thread clapper;
Thread timecode;

volatile int hour1 = 0, hour0 = 0;
volatile int minute1 = 0, minute0 = 0;
volatile int second1 = 0, second0 = 0;
volatile int deci = 0, centi = 0, milli = 0;

// 0 = init
// 1 = pi ready
// 2 = pi ready, clapper was closed
// 3 = pi ready, clapper was opened
// 4 = pi ready, clapped, waiting

//SPISlave pi(p5, p6, p7, p8);

Serial pi(USBTX, USBRX);

void number_lock(int digit, int lock) {
    switch (digit) {
        case 7:
            disp7 = lock;
            break;
        case 6:
            disp6 = lock;
            break;
        case 5:
            disp5 = lock;
            break;
        case 4:
            disp4 = lock;
            break;
        case 3:
            disp3 = lock;
            break;
        case 2:
            disp2 = lock;
            break;
        case 1:
            disp1 = lock;
            break;
        case 0:
            disp0 = lock;
            break;
    }
}

void set_digit(int digit, int value) {
    number_lock(digit, 0);
    ss3 = value & 8;
    ss2 = value & 4;
    ss1 = value & 2;
    ss0 = value & 1;
    number_lock(digit, 1);
}

void set_display(int d7, int d6, int d5, int d4, int d3, int d2, int d1, int d0) {
    set_digit(7, d7);
    set_digit(6, d6);
    set_digit(5, d5);
    set_digit(4, d4);
    set_digit(3, d3);
    set_digit(2, d2);
    set_digit(1, d1);
    set_digit(0, d0);
}

void clapper_thread() {
    while (1) {
        if (state == 1) {
            if (clap) {
                state = 3;
                while (clap) {}
                state = 4;
                pi.printf("CLAP\n");
                pi.printf("%d%d:%d%d:%d%d.%d%d%d\n", hour1, hour0, minute1, minute0, second1, second0, deci, centi, milli);
                set_display(hour1, hour0, minute1, minute0, second1, second0, deci, centi);

                roll_lock.lock();
                roll.filled_circle(64, 64, 10, RED);
                roll_lock.unlock();

                scene_lock.lock();
                scene.filled_circle(64, 64, 10, RED);
                scene_lock.unlock();

                take_lock.lock();
                take.filled_circle(64, 64, 10, RED);
                take_lock.unlock();

            } else {
                state = 2;
                while (!clap) {}
                state = 3;
                while (clap) {}
                state = 4;
                pi.printf("CLAP\n");
                pi.printf("%d%d:%d%d:%d%d.%d%d%d\n", hour1, hour0, minute1, minute0, second1, second0, deci, centi, milli);
                set_display(hour1, hour0, minute1, minute0, second1, second0, deci, centi);

                roll_lock.lock();
                roll.filled_circle(64, 64, 10, RED);
                roll_lock.unlock();

                scene_lock.lock();
                scene.filled_circle(64, 64, 10, RED);
                scene_lock.unlock();

                take_lock.lock();
                take.filled_circle(64, 64, 10, RED);
                take_lock.unlock();
            }
        }

        if (state == 1) {
            led1 = 1;
            led2 = 0;
            led3 = 0;
            led4 = 0;
        } else if (state == 2) {
            led1 = 0;
            led2 = 1;
            led3 = 0;
            led4 = 0;
        } else if (state == 3) {
            led1 = 0;
            led2 = 0;
            led3 = 1;
            led4 = 0;
        } else if (state == 4) {
            led1 = 0;
            led2 = 0;
            led3 = 0;
            led4 = 1;
        }
    }
}

void timecode_thread() {
    Timer t;
    t.start();

    int overflow_count = 0;
    int overflow_size = 2000000000;
    int offset = 0;

    int new_ms, new_us;
    while (1) {
        if (state != 4) {
            new_ms = t.read_ms();
            new_us = t.read_us();

            if (new_us >= overflow_size) {
                offset += (new_us - overflow_size) / 1000;
                t.reset();
                overflow_count++;
            }

            unsigned long long total_millis = (overflow_count) * (overflow_size / 1000) + new_ms + offset;

            int hours = min((int)(total_millis / 3600000), 99);
            int minutes = min((int)((total_millis / 60000) % 60), 59);
            int seconds = min((int)((total_millis / 1000) % 60), 59);
            int centi_s = min((int)((total_millis / 10) % 100), 99);
            int mill_s = (total_millis % 1000) / 100;

            hour1 = hours / 10; hour0 = hours % 10;
            minute1 = minutes / 10; minute0 = minutes % 10;
            second1 = seconds / 10; second0 = seconds % 10;
            deci = centi_s / 10; centi = centi_s % 10; milli = mill_s;
        
            if (new_ms % 3 == 0) {
                set_display(hour1, hour0, minute1, minute0, second1, second0, deci, centi);
            }
        }
    }
}

int main() {
    clap.mode(PullUp);
    // roll.background_color(GREEN);
    // scene.background_color(BLUE);
    // take.background_color(RED);
    roll.cls();
    scene.cls();
    take.cls();

    set_time(1704085200);

    disp7 = 1;
    disp6 = 1;
    disp5 = 1;
    disp4 = 1;
    disp3 = 1;
    disp2 = 1;
    disp1 = 1;
    disp0 = 1;

    bool clapped = false;
    bool receiving = false;

    string roll_s("");
    string scene_s("");
    string take_s("");

    string old_roll("");
    string old_scene("");
    string old_take("");

    pi.baud(9600);

    string msg("");
    int adding = 0;
    
    char c = ' ';

    clapper.start(clapper_thread);
    timecode.start(timecode_thread);

    while (1) {
        while (pi.readable()) {
            char c = pi.getc();
            msg += c;
            pi.printf(msg.c_str());

            if (c == '@') {
                state = 0;
                msg = "";
                roll_s = "";
                scene_s = "";
                take_s = "";

                roll_lock.lock();
                roll.cls();
                roll_lock.unlock();

                scene_lock.lock();
                scene.cls();
                scene_lock.unlock();

                take_lock.lock();
                take.cls();
                take_lock.unlock();
                
                continue;
            } else if (c == 'R' && adding == 0) {
                adding = 1;
                roll_s = "";
                msg = "";
                
                continue;
            } else if (c == 'S' && adding == 0) {
                adding = 2;
                scene_s = "";
                msg = "";

                continue;
            } else if (c == 'T' && adding == 0) {
                adding = 3;
                take_s = "";
                msg = "";

                continue;
            }

            if (adding == 1) {
                if (c == '~') {
                    adding = 0;

                    roll_lock.lock();
                    roll.color(RED);
                    roll.filled_rectangle(0, 0, 127, 127, 0x0000);
                    roll.locate(3, 3);
                    roll.text_width(6);
                    roll.text_height(6);
                    roll.printf(roll_s.c_str());
                    roll_lock.unlock();
                    msg = "";
                    
                    continue;
                }

                roll_s += c;
                //pi.printf("Adding to roll");

                led1 = 1;
                led2 = 0;
                led3 = 0;
                led4 = 0;
            } else if (adding == 2) {
                if (c == '~') {
                    adding = 0;

                    scene_lock.lock();
                    scene.color(RED);
                    scene.filled_rectangle(0, 0, 127, 127, 0x0000);
                    scene.locate(3, 3);
                    scene.text_width(6);
                    scene.text_height(6);
                    scene.printf(scene_s.c_str());
                    scene_lock.unlock();
                    msg = "";

                    continue;
                }

                scene_s += c;
                //pi.printf("Adding to scene");

                led1 = 0;
                led2 = 1;
                led3 = 0;
                led4 = 0;
            } else if (adding == 3) {
                if (c == '~') {
                    adding = 0;

                    take_lock.lock();
                    take.color(RED);
                    take.filled_rectangle(0, 0, 127, 127, 0x0000);
                    take.locate(3, 3);
                    take.text_width(6);
                    take.text_height(6);
                    take.printf(take_s.c_str());
                    take_lock.unlock();
                    msg = "";

                    if (state == 0) {
                        state = 1;
                    }

                    continue;
                }

                take_s += c;
                //pi.printf("Adding to take");

                led1 = 0;
                led2 = 0;
                led3 = 1;
                led4 = 0;
            }
        }           
    }
}
