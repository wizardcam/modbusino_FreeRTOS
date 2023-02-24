#include <stdint.h>
#define MODBUS_REGISTER_SIZE 100
extern SemaphoreHandle_t xPeripheralMutex;
extern uint16_t peripherals[];

int readReg(int regNumber);
void writeReg(int regNumber, int val);

/* BLOCK STARTS */
#define _GENERAL 0
#define _LASER (_GENERAL + 32)
#define _TEMP (_GENERAL + 64)
#define _GYRO (_GENERAL + 80)
#define _ROBOT (_GENERAL + 92)

/* GENERAL REGISTER ADDRESSES 0-31*/
#define _REQ_TIMESTAMP1 (_GENERAL)
#define _REQ_TIMESTAMP2 (_GENERAL + 1)
#define _REQ_COUNTER (_GENERAL + 2)
#define _EVENT_FLAGS (_GENERAL + 3)
#define _SLAVE_ID (_GENERAL + 4)
#define _MODBUS_FUNCTION (_GENERAL + 5)
#define _REGISTER_ADRESS (_GENERAL + 6)
#define _REGISTER_NUMBER (_GENERAL + 7)

/* LASER TASK REGISTER ADDRESSES 32-63*/
#define _LASER_STATUS (_LASER)
#define _LASER_TOTAL_ACTIVATIONS (_LASER + 1) // 32bit
#define _LASER_TOTAL_RETRYS (_LASER + 3)      // 32bit
#define _LASER_CURRENT_RETRYS (_LASER + 5)
#define _LASER_TOTAL_UPTIME1 (_LASER + 6)   // 32bit
#define _LASER_TOTAL_UPTIME2 (_LASER + 8)   // 32bit
#define _LASER_TOTAL_UPTIME3 (_LASER + 10)  // 32bit
#define _LASER_TOTAL_UPTIME4 (_LASER + 12)  // 32bit
#define _LASER_CURRENT_UPTIME (_LASER + 14) // 32bit
#define _LASER_TIMESTAMP (_LASER + 16)
#define _LASER_MODE (_LASER + 18)
#define _LASER_INTENSITY (_LASER + 19)
#define _SYNC_CONTROL (_LASER + 20)
#define _HDR_CONTROL (_LASER + 21)
#define _FRAME_COUNTER (_LASER + 22) // 32bit
#define _LASER_DELAY (_LASER + 24)
#define _LASER_PROTECTION (_LASER + 25)

/* TEMPERATURE TASK REGISTER ADDRESSES 64-79*/
#define _TEMP_TIMESTAMP (_TEMP) // 32bit
#define _TEMPERATURE (_TEMP + 2)
#define _HUMIDITY (_TEMP + 3)

/* GYRO TASK REGISTER ADDRESSES 80-91*/
#define _GYRO_TS (_GYRO)
#define _GYRO_ROT_X (_GYRO + 1)
#define _GYRO_ROT_Y (_GYRO + 2)
#define _GYRO_ROT_Z (_GYRO + 3)
#define _GYRO_ACCEL_X (_GYRO + 4)
#define _GYRO_ACCEL_Y (_GYRO + 5)
#define _GYRO_ACCEL_Z (_GYRO + 6)

/* ROBOT START PACKET TIMESTAMP REGISTER ADDRESSES 92-109*/
#define _ROBOT_START_1 (_ROBOT)
#define _ROBOT_START_2 (_ROBOT + 1)
#define _ROBOT_START_3 (_ROBOT + 2)
#define _ROBOT_START_4 (_ROBOT + 3)
#define _ROBOT_START_5 (_ROBOT + 4)
#define _ROBOT_START_6 (_ROBOT + 5)
#define _ROBOT_START_7 (_ROBOT + 6)

/* ROBOT END PACKET TIMESTAMP TASK REGISTER ADDRESSES 110-119*/
#define _ROBOT_END_1 (_ROBOT + 7)
#define _ROBOT_END_2 (_ROBOT + 8)
#define _ROBOT_END_3 (_ROBOT + 9)
#define _ROBOT_END_4 (_ROBOT + 10)
#define _ROBOT_END_5 (_ROBOT + 11)
#define _ROBOT_END_6 (_ROBOT + 12)
#define _ROBOT_END_7 (_ROBOT + 13)
