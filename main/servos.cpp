#include <Arduino.h>
#include <servos.hpp>

#include <EEPROM.h>
#include "ESP32PWM.h"
#include "ESP32Servo.h"

#define SERVO_FREQUENCY 50
#define SERVO_MIN_DUTY_US 500
#define SERVO_MAX_DUTY_US 2500
#define EEPROM_SIZE 24

#define LEG_FRONT_RIGHT_INDEX 0
#define LEG_REAR_RIGHT_INDEX 1
#define LEG_FRONT_LEFT_INDEX 2
#define LEG_REAR_LEFT_INDEX 3

#define SERVO_ALPHA_INDEX 0  // Coxa to Femur joint
#define SERVO_BETA_INDEX 1   // Femur to Tibia joint
#define SERVO_GAMMA_INDEX 2  // Body to Coxa joint

/* PCB Header to ESP32 GPIO Mapping */
#define H1 16
#define H2 17
#define H3 18
#define H4 19
#define H5 21
#define H6 22
#define H7 27
#define H8 26
#define H9 25
#define H10 33
#define H11 32
#define H12 23

/* Installation and Adjustment -----------------------------------------------*/
//#define INSTALL  // uncomment only this to install the robot
//#define ADJUST  // uncomment only this to adjust the servos
//#define VERIFY  // uncomment only this to verify the adjustment
//#define TROUBLESHOOT
const float adjust_site[3] = {100, 80, 42};
const float real_site[4][3] = {{106, 87.5, 52.4}, {103.5, 85.5, 47.6}, {100, 88, 47.6}, {111.5, 77, 60}};
/* Servos --------------------------------------------------------------------*/
// define 12 servos for 4 legs
// Servo servo[4][3];
// define servos' ports
uint8_t servo_pin[4][3] = {0};

Servo servo[4][3];
/* Size of the robot ---------------------------------------------------------*/
const float length_a = 47;
const float length_b = 82;
const float length_c = 32.5;
const float length_side = 65;
const float z_absolute = 0;
/* Constants for movement ----------------------------------------------------*/
const float z_default = -73.5, z_up = -33.5, z_boot = z_absolute - 23.5;
const float x_default = 75, x_offset = 0;
const float y_start = 0, y_step = 50;
const float y_default = x_default;
/* variables for movement ----------------------------------------------------*/
typedef struct {
    float site_now[4][3];      // real-time coordinates of the end of each leg
    float site_expect[4][3];   // expected coordinates of the end of each leg
    float temp_speed[4][3];    // each axis' speed, needs to be recalculated before each movement
    int32_t rest_counter = 0;  //+1/0.02s, for automatic rest
} service_status_t;

service_status_t sst;

float move_speed;          // movement speed
float speed_multiple = 1;  // movement speed multiple
const float spot_turn_speed = 4;
const float leg_move_speed = 8;
const float body_move_speed = 3;
const float stand_seat_speed = 1;
// functions' parameter
const float KEEP = 255;
// define PI for calculation
const float pi = 3.1415926;
/* Constants for turn --------------------------------------------------------*/
// temp length
const float temp_a = sqrt(pow(2 * x_default + length_side, 2) + pow(y_step, 2));
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt(pow(2 * x_default + length_side, 2) + pow(2 * y_start + y_step + length_side, 2));
const float temp_alpha = acos((pow(temp_a, 2) + pow(temp_b, 2) - pow(temp_c, 2)) / 2 / temp_a / temp_b);
// site for turn
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin(temp_alpha) - turn_y1 - length_side;
/* ---------------------------------------------------------------------------*/
String lastComm = "";
boolean print_reach = false;
TaskHandle_t Task0;
SemaphoreHandle_t Semaphore;
void servos_service(void* data);
void set_site(int leg, float x, float y, float z);
void cartesian_to_polar(float& alpha, float& beta, float& gamma, float x, float y, float z);

void attach_servos() {
    // front right left
    servo_pin[LEG_FRONT_RIGHT_INDEX][SERVO_ALPHA_INDEX] = H7;
    servo_pin[LEG_FRONT_RIGHT_INDEX][SERVO_BETA_INDEX] = H8;
    servo_pin[LEG_FRONT_RIGHT_INDEX][SERVO_GAMMA_INDEX] = H9;

    // front left leg
    servo_pin[LEG_FRONT_LEFT_INDEX][SERVO_ALPHA_INDEX] = H1;
    servo_pin[LEG_FRONT_LEFT_INDEX][SERVO_BETA_INDEX] = H2;
    servo_pin[LEG_FRONT_LEFT_INDEX][SERVO_GAMMA_INDEX] = H3;

    // rear left leg
    servo_pin[LEG_REAR_LEFT_INDEX][SERVO_ALPHA_INDEX] = H4;
    servo_pin[LEG_REAR_LEFT_INDEX][SERVO_BETA_INDEX] = H5;
    servo_pin[LEG_REAR_LEFT_INDEX][SERVO_GAMMA_INDEX] = H6;

    // rear left leg
    servo_pin[LEG_REAR_RIGHT_INDEX][SERVO_ALPHA_INDEX] = H10;
    servo_pin[LEG_REAR_RIGHT_INDEX][SERVO_BETA_INDEX] = H11;
    servo_pin[LEG_REAR_RIGHT_INDEX][SERVO_GAMMA_INDEX] = H12;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            Serial.println("Using Pin: " + String(servo_pin[i][j]));
            servo[i][j].attach(servo_pin[i][j]);
            delay(100);
        }
    }
}

void start_servos_service() {
    Semaphore = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(servos_service,  // Function that should be called
                            "ServoService",  // Name of the task (for debugging)
                            10000,           // Stack size (bytes)
                            (void*)&sst,     // Parameter to pass
                            1,               // Task priority
                            &Task0,          // Task handle
                            1);
}

void install() {
    attach_servos();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            servo[i][j].write(90);
            delay(100);
        }
    }
}

void troubleshoot() {
    attach_servos();
    float a = -45;
    float v = 90 + a;
    servo[LEG_FRONT_RIGHT_INDEX][SERVO_BETA_INDEX].write(v);
    delay(100);
}

/*
 - adjustment function
 - move each leg to adjustment site, so that you can measure the real sites.
 * ---------------------------------------------------------------------------*/
void adjust(void) {
    // initializes eeprom's errors to 0
    // number -100 - +100 is map to 0 - +200 in eeprom
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            EEPROM.write(i * 6 + j * 2, 100);
            EEPROM.write(i * 6 + j * 2 + 1, 100);
        }
    }
    EEPROM.commit();

    // initializes the relevant variables to adjustment position
    for (int i = 0; i < 4; i++) {
        set_site(i, adjust_site[0], adjust_site[1], adjust_site[2] + z_absolute);
        for (int j = 0; j < 3; j++) {
            sst.site_now[i][j] = sst.site_expect[i][j];
        }
    }

    // start servo service
    start_servos_service();

    // initialize servos
    attach_servos();
}

/*
 - verify function
 - verify the adjustment results, it will calculate errors and save to eeprom.
 * ---------------------------------------------------------------------------*/
void verify(void) {
    // calculate correct degree
    float alpha0, beta0, gamma0;
    cartesian_to_polar(alpha0, beta0, gamma0, adjust_site[0], adjust_site[1], adjust_site[2] + z_absolute);

    // calculate real degree and errors
    float alpha, beta, gamma;
    float degree_error[4][3];

    for (int i = 0; i < 4; i++) {
        cartesian_to_polar(alpha, beta, gamma, real_site[i][0], real_site[i][1], real_site[i][2] + z_absolute);
        degree_error[i][0] = alpha0 - alpha;
        degree_error[i][1] = beta0 - beta;
        degree_error[i][2] = gamma0 - gamma;
    }

    // save errors to eeprom
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            EEPROM.write(i * 6 + j * 2, (int)degree_error[i][j] + 100);
            EEPROM.write(i * 6 + j * 2 + 1, (int)(degree_error[i][j] * 100) % 100 + 100);
        }
    }

    EEPROM.commit();

    // initializes the relevant variables to adjustment position
    for (int i = 0; i < 4; i++) {
        set_site(i, adjust_site[0], adjust_site[1], adjust_site[2] + z_absolute);
        for (int j = 0; j < 3; j++) {
            sst.site_now[i][j] = sst.site_expect[i][j];
        }
    }
    // start servo service
    start_servos_service();
    attach_servos();
}

/*
  - wait one of end points move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_reach(int leg) {
    while (1) {
        if (print_reach) {
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][0], sst.site_expect[leg][0]);
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][1], sst.site_expect[leg][1]);
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][2], sst.site_expect[leg][2]);
        }

        if (sst.site_now[leg][0] == sst.site_expect[leg][0]) {
            if (sst.site_now[leg][1] == sst.site_expect[leg][1]) {
                if (sst.site_now[leg][2] == sst.site_expect[leg][2]) {
                    break;
                }
            }
        }
        vTaskDelay(1);
    }
}

/*
  - wait all of end points move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_all_reach(void) {
    for (int i = 0; i < 4; i++)
        wait_reach(i);
}

/*
  - set one of end points' expect site
  - this founction will set temp_speed[4][3] at same time
  - non - blocking function
   ---------------------------------------------------------------------------*/
void set_site(int leg, float x, float y, float z) {
    float length_x = 0, length_y = 0, length_z = 0;

    if (x != KEEP)
        length_x = x - sst.site_now[leg][0];
    if (y != KEEP)
        length_y = y - sst.site_now[leg][1];
    if (z != KEEP)
        length_z = z - sst.site_now[leg][2];

    float length = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));

    sst.temp_speed[leg][0] = length_x / length * move_speed * speed_multiple;
    sst.temp_speed[leg][1] = length_y / length * move_speed * speed_multiple;
    sst.temp_speed[leg][2] = length_z / length * move_speed * speed_multiple;

    if (x != KEEP)
        sst.site_expect[leg][0] = x;
    if (y != KEEP)
        sst.site_expect[leg][1] = y;
    if (z != KEEP)
        sst.site_expect[leg][2] = z;
}

/*
  - is_stand
   ---------------------------------------------------------------------------*/
bool is_stand(void) {
    if (sst.site_now[0][2] == z_default)
        return true;
    else
        return false;
}

/*
  - sit
  - blocking function
   ---------------------------------------------------------------------------*/
void sit(void) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_boot);
    }
    wait_all_reach();
}

/*
  - stand
  - blocking function
   ---------------------------------------------------------------------------*/
void stand(void) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_default);
    }
    wait_all_reach();
}

/*
  - Body init
  - blocking function
   ---------------------------------------------------------------------------*/
void b_init(void) {
    // stand();
    set_site(0, x_default, y_default, z_default);
    set_site(1, x_default, y_default, z_default);
    set_site(2, x_default, y_default, z_default);
    set_site(3, x_default, y_default, z_default);
    wait_all_reach();
}

/*
  - spot turn to left
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_left(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            // leg 3&1 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start, z_up);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            // leg 0&2 move
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_up);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - spot turn to right
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_right(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            // leg 2&0 move
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_up);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            // leg 1&3 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

void step_forward_dynamic(unsigned int step) {
    move_speed = leg_move_speed;

    while (step-- > 0) {
        if (sst.site_now[LEG_REAR_RIGHT_INDEX][1] == y_start) {
            // push back FL and RR legs
            set_site(LEG_FRONT_LEFT_INDEX, x_default + x_offset, y_start, z_default);
            set_site(LEG_REAR_RIGHT_INDEX, x_default + x_offset, y_start + 2 * y_step, z_default);

            // swing up FR and RL legs
            set_site(LEG_FRONT_RIGHT_INDEX, x_default + x_offset, y_start + 2 * y_step, z_up);
            set_site(LEG_REAR_LEFT_INDEX, x_default + x_offset, y_start, z_up);
            wait_all_reach();

        } else {
            // push back FR and RL legs
            set_site(LEG_FRONT_RIGHT_INDEX, x_default + x_offset, y_start, z_default);
            set_site(LEG_REAR_LEFT_INDEX, x_default + x_offset, y_start + 2 * y_step, z_default);

            // swing up FL and RR legs
            set_site(LEG_FRONT_LEFT_INDEX, x_default + x_offset, y_start + 2 * y_step, z_up);
            set_site(LEG_REAR_RIGHT_INDEX, x_default + x_offset, y_start, z_up);
            wait_all_reach();
        }
    }
}

/*
  - go forward
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_forward(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - go back
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_back(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            // leg 3&0 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            // leg 1&2 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

// add by RegisHsu

void body_left(int i) {
    set_site(0, sst.site_now[0][0] + i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] + i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] - i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] - i, KEEP, KEEP);
    wait_all_reach();
}

void body_right(int i) {
    set_site(0, sst.site_now[0][0] - i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] - i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] + i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] + i, KEEP, KEEP);
    wait_all_reach();
}

void hand_wave(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(2, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(0, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void hand_shake(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(2, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(0, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void head_up(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] - i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] + i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] - i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] + i);
    wait_all_reach();
}

void head_down(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] + i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] - i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] + i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] - i);
    wait_all_reach();
}

void body_dance(int i) {
    float body_dance_speed = 2;
    sit();
    move_speed = 1;
    set_site(0, x_default, y_default, KEEP);
    set_site(1, x_default, y_default, KEEP);
    set_site(2, x_default, y_default, KEEP);
    set_site(3, x_default, y_default, KEEP);
    print_reach = true;
    wait_all_reach();
    stand();
    set_site(0, x_default, y_default, z_default - 20);
    set_site(1, x_default, y_default, z_default - 20);
    set_site(2, x_default, y_default, z_default - 20);
    set_site(3, x_default, y_default, z_default - 20);
    wait_all_reach();
    move_speed = body_dance_speed;
    head_up(30);
    for (int j = 0; j < i; j++) {
        if (j > i / 4)
            move_speed = body_dance_speed * 2;
        if (j > i / 2)
            move_speed = body_dance_speed * 3;
        set_site(0, KEEP, y_default - 20, KEEP);
        set_site(1, KEEP, y_default + 20, KEEP);
        set_site(2, KEEP, y_default - 20, KEEP);
        set_site(3, KEEP, y_default + 20, KEEP);
        wait_all_reach();
        set_site(0, KEEP, y_default + 20, KEEP);
        set_site(1, KEEP, y_default - 20, KEEP);
        set_site(2, KEEP, y_default + 20, KEEP);
        set_site(3, KEEP, y_default - 20, KEEP);
        wait_all_reach();
    }
    move_speed = body_dance_speed;
    head_down(30);
    b_init();
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char* command) {
    Serial.println("What?");
}

void servos_cmd(int action_mode, int n_step) {
    switch (action_mode) {
        case W_FORWARD:
            Serial.println("Step forward");
            lastComm = "FWD";
            if (!is_stand())
                stand();
            step_forward(n_step);
            break;

        case W_BACKWARD:
            Serial.println("Step back");
            lastComm = "BWD";
            if (!is_stand())
                stand();
            step_back(n_step);
            break;

        case W_LEFT:
            Serial.println("Turn left");
            lastComm = "LFT";
            if (!is_stand())
                stand();
            turn_left(n_step);
            break;

        case W_RIGHT:
            Serial.println("Turn right");
            lastComm = "RGT";
            if (!is_stand())
                stand();
            turn_right(n_step);
            break;

        case W_STAND_SIT:
            Serial.println("1:up,0:dn");
            lastComm = "";
            if (n_step)
                stand();
            else
                sit();
            break;

        case W_SHAKE:
            Serial.println("Hand shake");
            lastComm = "";
            hand_shake(n_step);
            break;

        case W_WAVE:
            Serial.println("Hand wave");
            lastComm = "";
            hand_wave(n_step);
            break;

        case W_DANCE:
            Serial.println("Lets rock baby");
            lastComm = "";
            body_dance(10);
            break;

        default:
            Serial.println("Error");
            break;
    }
}

/*
  - trans site from cartesian to polar
  - mathematical model 2/2
   ---------------------------------------------------------------------------*/
void cartesian_to_polar(float& alpha, float& beta, float& gamma, float x, float y, float z) {
    // calculate w-z degree
    float v, w;
    w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
    v = w - length_c;
    alpha = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) / 2 / length_a /
                               sqrt(pow(v, 2) + pow(z, 2)));
    beta = acos((pow(length_a, 2) + pow(length_b, 2) - pow(v, 2) - pow(z, 2)) / 2 / length_a / length_b);
    // calculate x-y-z degree
    gamma = (w >= 0) ? atan2(y, x) : atan2(-y, -x);
    // trans degree pi->180
    alpha = alpha / pi * 180.0;
    beta = beta / pi * 180.0;
    gamma = gamma / pi * 180.0;
}

void print_final_PWM(int pin, uint16_t off) {
#ifdef TIMER_INTERRUPT_DEBUG
    Serial.printf("[PWM]\tP:%i\toff:%u\n", pin, off);
#endif
}

/*
  - trans site from polar to microservos
  - mathematical model map to fact
  - the errors saved in eeprom will be add
   ---------------------------------------------------------------------------*/
void polar_to_servo(int leg, float alpha, float beta, float gamma) {
    float alpha_error = EEPROM.read(leg * 6 + 0) - 100 + ((float)EEPROM.read(leg * 6 + 1) - 100) / 100;
    float beta_error = EEPROM.read(leg * 6 + 2) - 100 + ((float)EEPROM.read(leg * 6 + 3) - 100) / 100;
    float gamma_error = EEPROM.read(leg * 6 + 4) - 100 + ((float)EEPROM.read(leg * 6 + 5) - 100) / 100;

    alpha += alpha_error;
    beta += beta_error;
    gamma += gamma_error;

    if (leg == 0) {
        alpha = 90 - alpha;
        beta = beta;
        gamma += 90;
    } else if (leg == 1) {
        alpha += 90;
        beta = 180 - beta;
        gamma = 90 - gamma;
    } else if (leg == 2) {
        alpha += 90;
        beta = 180 - beta;
        gamma = 90 - gamma;
    } else if (leg == 3) {
        alpha = 90 - alpha;
        beta = beta;
        gamma += 90;
    }

    servo[leg][0].write(alpha);
    servo[leg][1].write(beta);
    servo[leg][2].write(gamma);
}

void servos_service(void* data) {
    // service_status_t sst = *(service_status_t *)data;
    for (;;) {
        float alpha, beta, gamma;
        xSemaphoreTake(Semaphore, portMAX_DELAY);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                if (abs(sst.site_now[i][j] - sst.site_expect[i][j]) >= abs(sst.temp_speed[i][j]))
                    sst.site_now[i][j] += sst.temp_speed[i][j];
                else
                    sst.site_now[i][j] = sst.site_expect[i][j];
            }
            cartesian_to_polar(alpha, beta, gamma, sst.site_now[i][0], sst.site_now[i][1], sst.site_now[i][2]);
            polar_to_servo(i, alpha, beta, gamma);
        }
        sst.rest_counter++;
        xSemaphoreGive(Semaphore);
        vTaskDelay(10 / portTICK_PERIOD_MS);

#ifdef TIMER_INTERRUPT_DEBUG
        Serial.printf("%05lu counter: %lu\n", (unsigned long)millis(), (unsigned long)sst.rest_counter);
        Serial.printf("[OUT]\tA:%f\tB:%f\tG:%f\n", alpha, beta, gamma);
#endif
    }
}

String getLastComm() {
    return lastComm;
}

void servos_start() {
    sit();
    b_init();
}

void servos_init() {
    int cpuSpeed = getCpuFrequencyMhz();
    Serial.println("CPU Running at " + String(cpuSpeed) + "MHz");
    Serial.println("starting PWM Library..");
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    EEPROM.begin(EEPROM_SIZE);

#ifdef INSTALL
    install();
    while (1) {
        delay(1000);
    }
#endif

#ifdef ADJUST
    adjust();
    while (1) {
        delay(1000);
    }
#endif

#ifdef VERIFY
    verify();
    while (1) {
        delay(1000);
    }
#endif

#ifdef TROUBLESHOOT
    troubleshoot();
    while (1) {
        delay(1000);
    }
#endif

    // initialize default parameter
    Serial.println("servo parameters:");
    set_site(0, x_default - x_offset, y_start + y_step, z_boot);
    set_site(1, x_default - x_offset, y_start + y_step, z_boot);
    set_site(2, x_default + x_offset, y_start, z_boot);
    set_site(3, x_default + x_offset, y_start, z_boot);
    Serial.printf("X:%f\tY:%f\tZ:%f\n", x_default, y_start, z_boot);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            sst.site_now[i][j] = sst.site_expect[i][j];
        }
    }
    Serial.println("starting servos service..");
    start_servos_service();

    // initialize servos
    attach_servos();
    servos_start();
    Serial.println("Servos initialized.");
    Serial.println("Robot initialization Complete");
}
