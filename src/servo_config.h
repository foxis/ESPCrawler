#define NUMBER_OF_MOTORS 12
#define SERVO_FREQUENCY 100
#define ACTUAL_FREQUENCY 97.29
//102.4

#define PERIOD (1000.0 / ACTUAL_FREQUENCY)
#define MIN_TIME 1
#define MAX_TIME 2
#define MID_TIME (((MIN_TIME) + (MAX_TIME)) / 2.0)
#define MAX_PWM 4095
#define MIN_PWM_ (MIN_TIME / PERIOD)
#define MAX_PWM_ (MAX_TIME / PERIOD)
#define MID_PWM_ (MID_TIME / PERIOD)

#define SERVO_K (uint16_t)(MAX_PWM * (MAX_PWM_ - MIN_PWM_) / M_PI)
#define SERVO_B (uint16_t)(MAX_PWM * MID_PWM_)

ServoConfig_t servo_config[] = {
	{	// left front leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = SERVO_K,
		.b = SERVO_B,
	},
	{ 	// left middle leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = SERVO_K,
		.b = SERVO_B,
	},
	{	// left back leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = SERVO_K,
		.b = SERVO_B,
	},
	{	// right back leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{	// right middle leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{	// right front leg
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{	// head
		.k = -SERVO_K,
		.b = SERVO_B,
	},
	{
		.k = SERVO_K,
		.b = SERVO_B,
	},
};

PCA9685 pca(&Wire, 0x40);
ServoHAL_PCA9685<12> servos(servo_config, &pca, SERVO_FREQUENCY);
