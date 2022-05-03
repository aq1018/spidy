#define SDA_PIN 16
#define SCL_PIN 4
#define LED_BUILTIN 22
#define LED_SPEED 500

#define W_STAND_SIT 0
#define W_FORWARD 1
#define W_BACKWARD 2
#define W_LEFT 3
#define W_RIGHT 4
#define W_SHAKE 5
#define W_WAVE 6
#define W_DANCE 7

void servos_init(void);
void servos_loop(void);
void servos_cmd(int action_mode, int n_step);
bool is_stand();
