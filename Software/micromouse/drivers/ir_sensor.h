#ifndef IR_ADC_SENSOR_H
#define IR_ADC_SENSOR_H

#define FRONT_THRESHOLD 200
#define RIGHT_THRESHOLD 425
#define LEFT_THRESHOLD 425

#define FRONT_THRESHOLD_UPPER 2400
#define FRONT_THRESHOLD_LOWER 2350

#define LEFT_DIFF_THRESHOLD 100
#define RIGHT_DIFF_THRESHOLD 100

#define CALIBRATION_CYCLES 400

#define LF_OFFSET -57
#define RF_OFFSET 180
#define LB_OFFSET -45
#define RB_OFFSET 20

#define IR_CENTERED 1500

typedef union {

	uint32_t adc_data[6];

	struct{
		uint32_t left_front;
		uint32_t right_back;
		uint32_t right_front;
		uint32_t left_back;
		uint32_t junk1;
		uint32_t junk2;
	};

} side_ir_data_t;


typedef struct {

	uint32_t count;
	uint16_t front_sum;
	uint16_t left_sum;
	uint16_t right_sum;

	union {

		uint8_t wall_int;

		struct {
			uint8_t left : 1;
			uint8_t back : 1;
			uint8_t right : 1;
			uint8_t front : 1;
			uint8_t blank : 4;
		} flags;
	};
} walls_t;


void ir_sensor_init(void);
void check_walls(walls_t * walls, side_ir_data_t * side_data);
void side_poll(side_ir_data_t * side_ir_data);
void front_poll(uint32_t * buf);
void update_ir_duty(char * value);
void calibrate_front(void);
void calibrate_left(void);
void calibrate_right(void);

// UART Callback
void stream_ir(char* val);

#endif
