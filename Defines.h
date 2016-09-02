//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de>, <dirk@hfma.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _DEFINES_H
	#define _DEFINES_H

	#define AS_RESET                                      1
	#define AS_RESET_CLEAR_EEPROM                         2

	// Register list0 values
	#define AS_REG_L0_INTERNAL_KEYS_VISIBLE               0x02
	#define AS_REG_L0_HMKEY_1                             0x0A
	#define AS_REG_L0_HMKEY_2                             0x0B
	#define AS_REG_L0_HMKEY_3                             0x0C
	#define AS_REG_L0_CONF_BUTTON_TIME                    0x15
	#define AS_REG_L0_LOCAL_RESET_DISABLE                 0x18

	// Register list1 values
	#define AS_REG_L1_AES_ACTIVE                          0x08
	#define AS_REG_L1_TRANSMIT_TRY_MAX                    0x30
	#define AS_REG_L1_STATUSINFO_MINDELAY                 0x57
	#define AS_REG_L1_REFERENCE_RUNNING_TIME_TOP_BOTTOM   0x0B
	#define AS_REG_L1_REFERENCE_RUNNING_TIME_BOTTOM_TOP   0x0D
	#define AS_REG_L1_CHANGE_OVER_DELAY                   0x0F
	#define AS_REG_L1_REFERENCE_RUN_COUNTER               0x10

	// Byte 2: communication bit field
	#define AS_WAKEUP                                     0x01
	#define AS_WAKE_ME_UP                                 0x02
	#define AS_BROADCAST                                  0x04
//	#define AS_UNKNOWN                                    0x08
	#define AS_BURST                                      0x10
	#define AS_ACK_REQ                                    0x20
	#define AS_REPEATED_MSG                               0x40
	#define AS_MESSAGE_TO_MASTER                          0x80

	// Byte 3: message types
	#define AS_MESSAGE_DEVINFO                            0x00
	#define AS_MESSAGE_CONFIG                             0x01					// signed answer needed on request
	#define AS_MESSAGE_RESPONSE                           0x02					// signed answer needed if byte 10 = AS_RESPONSE_AES_CHALLANGE
	#define AS_MESSAGE_RESPONSE_AES                       0x03					// signed answer needed
	#define AS_MESSAGE_KEY_EXCHANGE                       0x04					// signed answer needed
	#define AS_MESSAGE_INFO                               0x10
	#define AS_MESSAGE_ACTION                             0x11					// signed answer needed on request
	#define AS_MESSAGE_HAVE_DATA                          0x12
	#define AS_MESSAGE_SWITCH_EVENT                       0x3E					// signed answer needed on request
	#define AS_MESSAGE_TIMESTAMP                          0x3F					// signed answer needed on request
	#define AS_MESSAGE_REMOTE_EVENT                       0x40					// signed answer needed on request
	#define AS_MESSAGE_SENSOR_EVENT                       0x41					// signed answer needed on request
	#define AS_MESSAGE_SENSOR_DATA                        0x53					// signed answer needed on request
	#define AS_MESSAGE_CLIMATE_EVENT                      0x58
	#define AS_MESSAGE_CLIMATECTRL_EVENT                  0x5A
	#define AS_MESSAGE_POWER_EVENT                        0x5E
	#define AS_MESSAGE_POWER_EVENT_CYCLIC                 0x5F
	#define AS_MESSAGE_WEATHER_EVENT                      0x70

	// Byte 11: config values
	#define AS_CONFIG_PEER_ADD                            0x01
	#define AS_CONFIG_PEER_REMOVE                         0x02
	#define AS_CONFIG_PEER_LIST_REQ                       0x03
	#define AS_CONFIG_PARAM_REQ                           0x04
	#define AS_CONFIG_START                               0x05
	#define AS_CONFIG_END                                 0x06
	#define AS_CONFIG_WRITE_INDEX                         0x08
	#define AS_CONFIG_SERIAL_REQ                          0x09
	#define AS_CONFIG_PAIR_SERIAL                         0x0A
	#define AS_CONFIG_STATUS_REQUEST                      0x0E

	// Byte 10: response values
	#define AS_RESPONSE_ACK                               0x00
	#define AS_RESPONSE_ACK_STATUS                        0x01
	#define AS_RESPONSE_ACK2                              0x02
	#define AS_RESPONSE_AES_CHALLANGE                     0x04
	#define AS_RESPONSE_NACK                              0x80
	#define AS_RESPONSE_NACK_TARGET_INVALID               0x84

	// Byte 10: info values
	#define AS_INFO_SERIAL                                0x00
	#define AS_INFO_PEER_LIST                             0x01
	#define AS_INFO_PARAM_RESPONSE_PAIRS                  0x02
	#define AS_INFO_PARAM_RESPONSE_SEQ                    0x03
	#define AS_INFO_PARAMETER_CHANGE                      0x04
	#define AS_INFO_ACTUATOR_STATUS                       0x06
	#define AS_INFO_RT_STATUS                             0x0A

	// Byte 10: action values
	#define AS_ACTION_SET                                 0x02
	#define AS_ACTION_STOP_CHANGE                         0x03
	#define AS_ACTION_RESET                               0x04
	#define AS_ACTION_LED                                 0x80
	#define AS_ACTION_LEDALL                              0x81
	#define AS_ACTION_LEVEL                               0x81
	#define AS_ACTION_SLEEPMODE                           0x82
	#define AS_ACTION_ENTER_BOOTLOADER                    0xCA

	#define AS_BUTTON_BYTE_KEY_BITS                       0b00111111
	#define AS_BUTTON_BYTE_LONGPRESS_BIT                  0b01000000
	#define AS_BUTTON_BYTE_LOWBAT_BIT                     0b10000000

	#define AS_STATUS_KEYCHANGE_INACTIVE                  0xFF
	#define AS_STATUS_KEYCHANGE_ACTIVE1                   0x00
	#define AS_STATUS_KEYCHANGE_ACTIVE2                   0x08
#endif
