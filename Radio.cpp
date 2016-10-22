//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin cc1101 functions -----------------------------------------------------------------------------------------------
//- with a lot of copy and paste from culfw firmware
//- -----------------------------------------------------------------------------------------------------------------------

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "Debug.h"
#include "Radio.h"

namespace as {

// this is the global radio object
CC1101 radio;

// private:		//---------------------------------------------------------------------------------------------------------
CC1101::CC1101() : crc_ok(0), rss(0), lqi(0), intread(0), sending(false) {}

void CC1101::init(void) {																// initialize CC1101
  DPRINT(F("CC init"));

	ccInitHw();																			// init the hardware to get access to the RF modul

  DPRINT(F("1"));

	ccDeselect();																		// some deselect and selects to init the TRX868modul
	_delay_us(5);
	ccSelect();
	_delay_us(10);
	ccDeselect();
	_delay_us(41);

	strobe(CC1101_SRES);																// send reset
	_delay_ms(10);

	// define init settings for TRX868
	static const uint8_t initVal[] PROGMEM = {
		CC1101_IOCFG2,    0x2E,	// 												// non inverted GDO2, high impedance tri state
//		CC1101_IOCFG1,    0x2E,	// (default)									// low output drive strength, non inverted GD=1, high impedance tri state
		CC1101_IOCFG0,    0x06,	// packet CRC ok								// disable temperature sensor, non inverted GDO0,
		CC1101_FIFOTHR,   0x0D,													// 0 ADC retention, 0 close in RX, TX FIFO = 9 / RX FIFO = 56 byte
		CC1101_SYNC1,     0xE9,													// Sync word
		CC1101_SYNC0,     0xCA,
		CC1101_PKTLEN,    0x3D,													// packet length 61
		CC1101_PKTCTRL1,  0x0C,													// PQT = 0, CRC auto flush = 1, append status = 1, no address check
		CC1101_FSCTRL1,   0x06,													// frequency synthesizer control

		// 868.299866 MHz
		//CC1101_FREQ2,     0x21,
		//CC1101_FREQ1,     0x65,
		//CC1101_FREQ0,     0x6A,

		// 868.2895508
		CC1101_FREQ2,     0x21,
		CC1101_FREQ1,     0x65,
		CC1101_FREQ0,     0x50,

		CC1101_MDMCFG4,  0xC8,
		CC1101_MDMCFG3,  0x93,
		CC1101_MDMCFG2,  0x03,
		CC1101_DEVIATN,  0x34,													// 19.042969 kHz
		CC1101_MCSM2,    0x01,
//		CC1101_MCSM1,    0x30,	// (default)									// always go into IDLE
		CC1101_MCSM0,    0x18,
		CC1101_FOCCFG,   0x16,
		CC1101_AGCCTRL2, 0x43,
		//CC1101_WOREVT1, 0x28,													// tEVENT0 = 50 ms, RX timeout = 390 us
		//7CC1101_WOREVT0, 0xA0,
		//CC1101_WORCTRL, 0xFB,													//EVENT1 = 3, WOR_RES = 0
		CC1101_FREND1,  0x56,
		CC1101_FSCAL1,  0x00,
		CC1101_FSCAL0,  0x11,
		CC1101_TEST1,   0x35,
		CC1101_PATABLE, 0xC3,
	};
	for (uint8_t i=0; i<sizeof(initVal); i+=2) {										// write init value to TRX868
		writeReg(pgm_read_byte(&initVal[i]), pgm_read_byte(&initVal[i+1]));
	}

	DPRINT(F("2"));

	strobe(CC1101_SCAL);																// calibrate frequency synthesizer and turn it off
	while (readReg(CC1101_MARCSTATE, CC1101_STATUS) != 1) {								// waits until module gets ready
		_delay_us(1);
	  DPRINT(F("."));
	}

  DPRINT(F("3"));

	writeReg(CC1101_PATABLE, PA_MaxPower);												// configure PATABLE
	strobe(CC1101_SRX);																	// flush the RX buffer
	strobe(CC1101_SWORRST);																// reset real time clock

  DPRINTLN(F(" - ready"));
}

uint8_t CC1101::sndData(uint8_t *buf, uint8_t size, uint8_t burst) {										// send data packet via RF

  sending = true;

	// Going from RX to TX does not work if there was a reception less than 0.5
	// sec ago. Due to CCA? Using IDLE helps to shorten this period(?)
	strobe(CC1101_SIDLE);																// go to idle mode
	strobe(CC1101_SFRX );																// flush RX buffer
	strobe(CC1101_SFTX );																// flush TX buffer

	//dbg << "tx\n";

	if (burst) {																		// BURST-bit set?
		strobe(CC1101_STX  );															// send a burst
		_delay_ms(360);																	// according to ELV, devices get activated every 300ms, so send burst for 360ms
		//dbg << "send burst\n";
	} else {
		_delay_ms(1);																	// wait a short time to set TX mode
	}

	writeReg(CC1101_TXFIFO, size);
	writeBurst(CC1101_TXFIFO, buf, size);						// write in TX FIFO

	strobe(CC1101_SFRX);																// flush the RX buffer
	strobe(CC1101_STX);																	// send a burst

	for(uint8_t i = 0; i < 200; i++) {													// after sending out all bytes the chip should go automatically in RX mode
		if( readReg(CC1101_MARCSTATE, CC1101_STATUS) == MARCSTATE_RX)
			break;																		//now in RX mode, good
		if( readReg(CC1101_MARCSTATE, CC1101_STATUS) != MARCSTATE_TX) {
			break;																		//neither in RX nor TX, probably some error
		}
		_delay_us(10);
	}

  sending = false;

  return true;
}

uint8_t CC1101::rcvData(uint8_t *buf,uint8_t size) {														// read data packet from RX FIFO
	uint8_t rxBytes = readReg(CC1101_RXBYTES, CC1101_STATUS);							// how many bytes are in the buffer
	//dbg << rxBytes << ' ';

	if ((rxBytes & 0x7F) && !(rxBytes & 0x80)) {										// any byte waiting to be read and no overflow?
		rxBytes = readReg(CC1101_RXFIFO, CC1101_CONFIG);									// read data length
		if (rxBytes > size) {													// if packet is too long
			rxBytes = 0;																	// discard packet
		}
		else {
			readBurst(buf, CC1101_RXFIFO, rxBytes);									// read data packet
			rss = readReg(CC1101_RXFIFO, CC1101_CONFIG);								// read RSSI
			if (rss >= 128) rss = 255 - rss;
			rss /= 2; rss += 72;
			uint8_t val = readReg(CC1101_RXFIFO, CC1101_CONFIG);						// read LQI and CRC_OK
			lqi = val & 0x7F;
			crc_ok = bitRead(val, 7);
			if( crc_ok == 0 ) {
			  rxBytes = 0;
			}
		}
	}
	else {
	  rxBytes = 0;																	// nothing to do, or overflow
	}
	strobe(CC1101_SFRX);																// flush Rx FIFO
	strobe(CC1101_SIDLE);																// enter IDLE state
	strobe(CC1101_SRX);																	// back to RX state
	strobe(CC1101_SWORRST);																// reset real time clock
	//	trx868.rfState = RFSTATE_RX;													// declare to be in Rx state
//  DPRINT("-> ");
//	DHEX(buf,buf[0]);
	return rxBytes;	// return number of byte in buffer
}

void    CC1101::setIdle() {																	// put CC1101 into power-down state
	strobe(CC1101_SIDLE);																// coming from RX state, we need to enter the IDLE state first
	//strobe(CC1101_SFRX);
	strobe(CC1101_SPWD);																// enter power down state
	//dbg << "pd\n";
}

void CC1101::wakeup () {
  ccSelect();                                     // wake up the communication module
    waitMiso();
    ccDeselect();
}

uint8_t CC1101::detectBurst(void) {
	// 10 7/10 5 in front of the received string; 33 after received string
	// 10 - 00001010 - sync word found
	// 7  - 00000111 - GDO0 = 1, GDO2 = 1
	// 5  - 00000101 - GDO0 = 1, GDO2 = 1
	// 33 - 00100001 - GDO0 = 1, preamble quality reached
	// 96 - 01100000 - burst sent
	// 48 - 00110000 - in receive mode
	//
	// Status byte table:
	//	0 current GDO0 value
	//	1 reserved
	//	2 GDO2
	//	3 sync word found
	//	4 channel is clear
	//	5 preamble quality reached
	//	6 carrier sense
	//	7 CRC ok
	//
	// possible solution for finding a burst is to check for bit 6, carrier sense

	// power on cc1101 module and set to RX mode
	ccSelect();																			// wake up the communication module
	waitMiso();
	ccDeselect();

	for(uint8_t i = 0; i < 200; i++) {													// instead of delay, check the really needed time to wakeup
		if (readReg(CC1101_MARCSTATE, CC1101_STATUS) != 0xff) break;
		_delay_us(10);
	}
	
	strobe(CC1101_SRX);																	// set RX mode again

	uint8_t bTmp;
	for (uint8_t i = 0; i < 200; i++) {													// check if we are in RX mode
		bTmp = readReg(CC1101_PKTSTATUS, CC1101_STATUS);								// read the status of the line
		if ((bTmp & 0x10) || (bTmp & 0x40)) break;										// check for channel clear, or carrier sense
		_delay_us(10);																	// wait a bit
	}
	return (bTmp & 0x40)?1:0;															// return carrier sense bit
}

void CC1101::strobe(uint8_t cmd) {														// send command strobe to the CC1101 IC via SPI
	ccSelect();																			// select CC1101
	waitMiso();																			// wait until MISO goes low
	ccSendByte(cmd);																	// send strobe command
	ccDeselect();																		// deselect CC1101
}

void CC1101::readBurst(uint8_t *buf, uint8_t regAddr, uint8_t len) {						// read burst data from CC1101 via SPI
	ccSelect();																			// select CC1101
	waitMiso();																			// wait until MISO goes low
	ccSendByte(regAddr | READ_BURST);													// send register address
	for(uint8_t i=0 ; i<len ; i++) {
		buf[i] = ccSendByte(0x00);														// read result byte by byte
		//dbg << i << ":" << buf[i] << '\n';
	}
	ccDeselect();																		// deselect CC1101
}

void CC1101::writeBurst(uint8_t regAddr, uint8_t *buf, uint8_t len) {					// write multiple registers into the CC1101 IC via SPI
	ccSelect();																			// select CC1101
	waitMiso();																			// wait until MISO goes low
	ccSendByte(regAddr | WRITE_BURST);													// send register address
	for(uint8_t i=0 ; i<len ; i++) ccSendByte(buf[i]);									// send value
	ccDeselect();																		// deselect CC1101
}

uint8_t CC1101::readReg(uint8_t regAddr, uint8_t regType) {									// read CC1101 register via SPI
	ccSelect();																			// select CC1101
	waitMiso();																			// wait until MISO goes low
	ccSendByte(regAddr | regType);														// send register address
	uint8_t val = ccSendByte(0x00);														// read result
	ccDeselect();																		// deselect CC1101
	return val;
}

void CC1101::writeReg(uint8_t regAddr, uint8_t val) {									// write single register into the CC1101 IC via SPI
	ccSelect();																			// select CC1101
	waitMiso();																			// wait until MISO goes low
	ccSendByte(regAddr);																// send register address
	ccSendByte(val);																	// send value
	ccDeselect();																		// deselect CC1101
}

void CC1101::handleGDO0Int() {
  if( sending == false ) {
    // DPRINTLN("*");
    intread = 1;
  }
}

uint8_t CC1101::read (Message& msg) {
  if( intread == 0 )
    return 0;
  intread = 0;

  uint8_t len = rcvData(buffer.buffer(),buffer.buffersize());
  if( len > 0 ) {
    buffer.length(len);
    // decode the message
    buffer.decode();
    // copy buffer to message
    memcpy(msg.buffer(),buffer.buffer(),len);
  }
  msg.length(len);
  // reset buffer
  buffer.clear();
  return msg.length();
}

bool CC1101::readAck (const Message& msg) {
  if( intread == 0 )
    return 0;
  intread = 0;

  bool ack=false;
  uint8_t len = rcvData(buffer.buffer(),buffer.buffersize());
  if( len > 0 ) {
    buffer.length(len);
    // decode the message
    buffer.decode();
    ack = buffer.isAck() &&
         (buffer.from() == msg.to()) &&
         (buffer.to() == msg.from()) &&
         (buffer.count() == msg.count());
    // reset buffer
    buffer.clear();
  }
  return ack;
}

uint8_t CC1101::read (Message& msg, uint32_t timeout) {
  uint8_t num = 0;
  uint32_t time=0;
  do {
    num = read(msg);
    if( num == 0 ) {
      delay(50); // wait 50ms
      time += 50;
    }
  }
  while( num == 0 && time < timeout );
  return num;
}

bool CC1101::write (const Message& msg, uint8_t burst) {
  memcpy(sbuffer.buffer(),msg.buffer(),msg.length());
  sbuffer.length(msg.length());
  sbuffer.encode();
  return radio.sndData(sbuffer.buffer(),sbuffer.length(),burst);
}

bool CC1101::write (const Message& msg, uint8_t burst, uint8_t maxretry, uint32_t timeout) {
  return false;
}

}
