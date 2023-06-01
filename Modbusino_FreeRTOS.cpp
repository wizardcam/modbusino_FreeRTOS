/*
 * Copyright © 2011-2012 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * License ISC, see LICENSE for more details.
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 *
 */

#include <inttypes.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#include <string.h>
#else
#include "WProgram.h"
#include <pins_arduino.h>
#endif
#include "Modbusino_FreeRTOS.h"
#include "eeprom_buffer.h"
#include "lfs_registers.h"
#include "peripheral_registers.h"

#define delay(ms) vTaskDelay(ms)
#define _MODBUS_RTU_SLAVE 0
#define _MODBUS_RTU_FUNCTION 1
#define _MODBUS_RTU_PRESET_REQ_LENGTH 6
#define _MODBUS_RTU_PRESET_RSP_LENGTH 2

#define _MODBUS_RTU_CHECKSUM_LENGTH 2

/* As reported in https://github.com/stephane/modbusino/issues/6, the code could
segfault for longer ADU */
#define _MODBUSINO_RTU_MAX_ADU_LENGTH 256

/* Supported function codes */
#define _FC_READ_HOLDING_REGISTERS 0x03
#define _FC_WRITE_MULTIPLE_REGISTERS 0x10

#define _SLAVE_MAX_N 3
#define _PERIPHERAL_SLAVE 1
#define _EEPROM_SLAVE 2
#define _LFS_SLAVE 3

uint16_t modbus_peripherals[PERIPHERALS_REGISTER_SIZE] = {0};
uint16_t modbus_lfs[LFS_REGISTER_SIZE] = {0};

enum { _STEP_FUNCTION = 0x01, _STEP_META, _STEP_DATA };

static uint16_t req_counter;
static uint16_t lfs_req_counter;
static uint16_t peripherals_req_counter;
static uint16_t eeprom_req_counter;

static uint16_t crc16(uint8_t *req, uint8_t req_length)
{
    uint8_t j;
    uint16_t crc;

    crc = 0xFFFF;
    while (req_length--) {
        crc = crc ^ *req++;
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = crc >> 1;
        }
    }

    return (crc << 8 | crc >> 8);
}

ModbusinoSlave::ModbusinoSlave(uint8_t slave)
{
    if (slave >= 0 && slave <= 247) {
        _slave = slave;
    }
}

void ModbusinoSlave::setup(long baud)
{
    Serial.begin(baud);
}

static int check_integrity(uint8_t *msg, uint8_t msg_length)
{
    uint16_t crc_calculated;
    uint16_t crc_received;

    if (msg_length < 2)
        return -1;

    crc_calculated = crc16(msg, msg_length - 2);
    crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];

    /* Check CRC of msg */
    if (crc_calculated == crc_received) {
        return msg_length;
    } else {
        return -1;
    }
}

static int build_response_basis(uint8_t slave, uint8_t function, uint8_t *rsp)
{
    rsp[0] = slave;
    rsp[1] = function;

    return _MODBUS_RTU_PRESET_RSP_LENGTH;
}

static void send_msg(uint8_t *msg, uint8_t msg_length)
{
    uint16_t crc = crc16(msg, msg_length);

    msg[msg_length++] = crc >> 8;
    msg[msg_length++] = crc & 0x00FF;

    Serial.write(msg, msg_length);
}

static uint8_t response_exception(uint8_t slave, uint8_t function,
                                  uint8_t exception_code, uint8_t *rsp)
{
    uint8_t rsp_length;

    rsp_length = build_response_basis(slave, function + 0x80, rsp);

    /* Positive exception code */
    rsp[rsp_length++] = exception_code;

    return rsp_length;
}

static void flush(void)
{
    const TickType_t xDelay = 3 / portTICK_PERIOD_MS;
    uint8_t i = 0;

    /* Wait a moment to receive the remaining garbage but avoid getting stuck
     * because the line is saturated */
    while (Serial.available() && i++ < 10) {
        Serial.flush();
        delay(xDelay);
    }
}

static int receive(uint8_t *req)
{
    uint8_t i;
    uint8_t length_to_read;
    uint8_t req_index;
    uint8_t step;
    uint8_t function;
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;

    /* We need to analyse the message step by step.  At the first step, we want
     * to reach the function code because all packets contain this
     * information. */
    step = _STEP_FUNCTION;
    length_to_read = _MODBUS_RTU_FUNCTION + 1;

    req_index = 0;
    while (length_to_read != 0) {

        /* The timeout is defined to ~10 ms between each bytes.  Precision is
           not that important so I rather to avoid millis() to apply the KISS
           principle (millis overflows after 50 days, etc) */
        if (!Serial.available()) {
            i = 0;
            while (!Serial.available()) {
                if (++i == 10) {
                    /* Too late, bye */
                    return -1 - MODBUS_INFORMATIVE_RX_TIMEOUT;
                }
                delay(xDelay);
            }
        }

        req[req_index] = Serial.read();

        /* Moves the pointer to receive other data */
        req_index++;

        /* Computes remaining bytes */
        length_to_read--;

        if (length_to_read == 0) {
            if (req[_MODBUS_RTU_SLAVE] > _SLAVE_MAX_N
                && req[_MODBUS_RTU_SLAVE != MODBUS_BROADCAST_ADDRESS]) {
                flush();
                return -1 - MODBUS_INFORMATIVE_NOT_FOR_US;
            }

            switch (step) {
            case _STEP_FUNCTION:
                /* Function code position */
                function = req[_MODBUS_RTU_FUNCTION];
                if (function == _FC_READ_HOLDING_REGISTERS) {
                    length_to_read = 4;
                } else if (function == _FC_WRITE_MULTIPLE_REGISTERS) {
                    length_to_read = 5;
                } else {
                    /* Wait a moment to receive the remaining garbage */
                    flush();
                    if (req[_MODBUS_RTU_SLAVE] <= _SLAVE_MAX_N
                        || req[_MODBUS_RTU_SLAVE] == MODBUS_BROADCAST_ADDRESS) {
                        /* It's for me so send an exception (reuse req) */
                        uint8_t rsp_length = response_exception(
                            _SLAVE_MAX_N, function,
                            MODBUS_EXCEPTION_ILLEGAL_FUNCTION, req);
                        send_msg(req, rsp_length);
                        return -1 - MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
                    }

                    return -1;
                }
                step = _STEP_META;
                break;
            case _STEP_META:
                length_to_read = _MODBUS_RTU_CHECKSUM_LENGTH;

                if (function == _FC_WRITE_MULTIPLE_REGISTERS)
                    length_to_read += req[_MODBUS_RTU_FUNCTION + 5];

                if ((req_index + length_to_read)
                    > _MODBUSINO_RTU_MAX_ADU_LENGTH) {
                    flush();
                    if (req[_MODBUS_RTU_SLAVE] <= _SLAVE_MAX_N
                        || req[_MODBUS_RTU_SLAVE] == MODBUS_BROADCAST_ADDRESS) {
                        /* It's for me so send an exception (reuse req) */
                        uint8_t rsp_length = response_exception(
                            _SLAVE_MAX_N, function,
                            MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, req);
                        send_msg(req, rsp_length);
                        return -1 - MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
                    }
                    return -1;
                }
                step = _STEP_DATA;
                break;
            default:
                break;
            }
        }
    }
    return check_integrity(req, req_index);
}

static void reply(uint8_t *req, uint8_t req_length)
{
    uint16_t *tab_reg;
    uint16_t nb_reg;
    uint8_t slave = req[_MODBUS_RTU_SLAVE];
    uint8_t function = req[_MODBUS_RTU_FUNCTION];
    uint16_t address =
        (req[_MODBUS_RTU_FUNCTION + 1] << 8) + req[_MODBUS_RTU_FUNCTION + 2];
    uint16_t nb =
        (req[_MODBUS_RTU_FUNCTION + 3] << 8) + req[_MODBUS_RTU_FUNCTION + 4];
    uint8_t rsp[_MODBUSINO_RTU_MAX_ADU_LENGTH];
    uint8_t rsp_length = 0;

    // Slave out of range
    if (slave > _SLAVE_MAX_N && slave != MODBUS_BROADCAST_ADDRESS) {
        return;
    }

    if (slave == _PERIPHERAL_SLAVE) {
        tab_reg = modbus_peripherals;
        nb_reg = PERIPHERALS_REGISTER_SIZE;
        req_counter = peripherals_req_counter += 1;
    }
    if (slave == _EEPROM_SLAVE) {
        tab_reg = eeprom.getBuffer16();
        nb_reg = EEPROM_SIZE;
        req_counter = eeprom_req_counter += 1;
    }
    if (slave == _LFS_SLAVE) {
        tab_reg = modbus_lfs;
        nb_reg = LFS_REGISTER_SIZE;
        req_counter = lfs_req_counter += 1;
    }

    if (address + nb > nb_reg) {
        rsp_length = response_exception(
            slave, function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
    } else {
        req_length -= _MODBUS_RTU_CHECKSUM_LENGTH;

        uint32_t TimeNow = millis();
        // Custom modbus info registers.
        taskENTER_CRITICAL();
        tab_reg[_REQ_TIMESTAMP1] = (TimeNow >> 16) & 0x0000FFFF; // HIGH
        tab_reg[_REQ_TIMESTAMP2] = TimeNow & 0x0000FFFF;         // LOW
        tab_reg[_REQ_COUNTER] = req_counter;
        tab_reg[_SLAVE_ID] = slave;
        tab_reg[_MODBUS_FUNCTION] = req[_MODBUS_RTU_FUNCTION];
        tab_reg[_REGISTER_ADRESS] = address;
        tab_reg[_REGISTER_NUMBER] = nb;
        taskEXIT_CRITICAL();

        if (function == _FC_READ_HOLDING_REGISTERS) {
            uint16_t i;

            rsp_length = build_response_basis(slave, function, rsp);
            rsp[rsp_length++] = nb << 1;
            for (i = address; i < address + nb; i++) {
                taskENTER_CRITICAL();
                rsp[rsp_length++] = tab_reg[i] >> 8;
                rsp[rsp_length++] = tab_reg[i] & 0xFF;
                taskEXIT_CRITICAL();
                /* reset event flag register after it is read */
                if (i == _EVENT_FLAGS) {
                    tab_reg[_EVENT_FLAGS] = 0;
                }
            }
        } else {
            uint16_t i, j;

            if (slave == _EEPROM_SLAVE) {
                address = EEPROM_DATA_START + address;
            }
            taskENTER_CRITICAL();
            for (i = address, j = 6; i < address + nb; i++, j += 2) {
                /* 6 and 7 = first value */
                tab_reg[i] = (req[_MODBUS_RTU_FUNCTION + j] << 8)
                             + req[_MODBUS_RTU_FUNCTION + j + 1];
            }
            taskEXIT_CRITICAL();

            rsp_length = build_response_basis(slave, function, rsp);
            /* 4 to copy the address (2) and the no. of registers */
            memcpy(rsp + rsp_length, req + rsp_length, 4);
            rsp_length += 4;
        }
    }
    send_msg(rsp, rsp_length);
}

int ModbusinoSlave::loop()
{
    int rc = 0;
    uint8_t req[_MODBUSINO_RTU_MAX_ADU_LENGTH];

    if (Serial.available()) {
        rc = receive(req);
        if (rc > 0) {
            reply(req, rc);
        }
    }
    if (rc <= 0) {
        return rc;
    } else {
        return req[_MODBUS_RTU_SLAVE];
    }
    /* Returns the slave number if successful,
       0 if a slave filtering has occured,
       -1 if an undefined error has occured,
       -2 for MODBUS_EXCEPTION_ILLEGAL_FUNCTION
       etc
    */
}
