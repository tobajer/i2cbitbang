/*
 * i2cbitbang.cpp
 *
 *  Created on: 24 lis 2019
 *      Author: Tata
 *      based on: https://en.wikipedia.org/wiki/I%C2%B2C#Example_of_bit-banging_the_I.C2.B2C_Master_protocol
 */

#include "i2cbitbang.h"

//extern const i2cbbConfig_t i2cbbConfigTable[NUMBER_OF_I2CBB_INSTANCES];

i2cbitbang::i2cbitbang(uint32_t i, uint8_t slAdd) {

	slaveAddress = slAdd;
	started = false;
	instance = i;
	error = I2CBB_ERROR_NONE;
	i2cMode = MODE_I2C;			//default is I2c Mode. There is also SCCB mode (e.g. for cam OV7670)
	speedSelect = SPEED_400k;	//default speed 100kHz
	stretchTime_us = 1000;		//in microseconds
	/*
	 * ackMode parameter has two options:
	 * 	ACK_CHECK: (default) ACK is check as normal i2c operation
	 * 	ACK_IGNORE: to create SCCB communication (e.g. cam OV7670)
	 */
	ackMode = ACK_CHECK;

	/*initalizing timing according to i2c standards
	 * https://www.analog.com/en/technical-articles/i2c-timing-definition-and-specification-guide-part-2.html#
	 */
	init_timings();

	if (i >= NUMBER_OF_I2CBB_INSTANCES)
		while(1);	//trap in case of bad instance
		// TODO find non-blocking way

	conf_hardware();
}

i2cbitbang::~i2cbitbang()
{
}

void i2cbitbang::i2c_start_cond(void)
{
  if (started)
  {
    // if started, do a restart condition
    // set SDA to 1
    set_SDA();
    I2C_Delay.hns(t.susta);
    set_SCL();

    clock_stretching(stretchTime_us);
    // Repeated start setup time, minimum 4.7us
    I2C_Delay.hns(t.susta);
  }

  //TODO implement arbitration
 /*
  conf_SDA(I2CBB_INPUT);
  if (read_SDA() == 0)
  {
    arbitration_lost();
  }
  conf_SDA(I2CBB_OUTPUT);
*/

  // SCL is high, set SDA from 1 to 0.
  clear_SDA();
  I2C_Delay.hns(t.hdsta);
  clear_SCL();
  I2C_Delay.hns(t.low);
  started = true;
}

void i2cbitbang::i2c_stop_cond(void)
{
  // set SDA to 0
  clear_SDA();
//  I2C_delay();
  I2C_Delay.hns(t.susto);

  set_SCL();

  clock_stretching(stretchTime_us);

  // Stop bit setup time, minimum 4us
  I2C_Delay.hns(t.susto);

  // SCL is high, set SDA from 0 to 1
  set_SDA();
  I2C_Delay.hns(t.buff);

  //TODO implement arbitration
/*  if (read_SDA() == 0) {
    arbitration_lost();
  }
*/

  started = false;
}

// Write a bit to I2C bus
void i2cbitbang::i2c_write_bit(bool bit)
{

  if (bit) {
    set_SDA();
  } else {
    clear_SDA();
  }

  // SDA change propagation delay
  I2C_Delay.hns(t.sudat);

  // Set SCL high to indicate a new valid SDA value is available
  set_SCL();

  // Wait for SDA value to be read by slave, minimum of 4us for standard mode
  I2C_Delay.hns(t.high);

  clock_stretching(stretchTime_us);

  //TODO implement arbitration
  // SCL is high, now data is valid
  // If SDA is high, check that nobody else is driving SDA
/*  conf_SDA(I2CBB_INPUT);
  if (bit && (read_SDA() == 0)) {
    arbitration_lost();
  }
  conf_SDA(I2CBB_OUTPUT);
*/
  // Clear the SCL to low in preparation for next change
  clear_SCL();
  I2C_Delay.hns(t.low);
}

// Read a bit from I2C bus
bool i2cbitbang::i2c_read_bit(void) {
  bool bit;

  // Let the slave drive data
  set_SDA();

  // Wait for SDA value to be written by slave, minimum of 4us for standard mode
  I2C_Delay.hns(t.dvdat);

  // Set SCL high to indicate a new valid SDA value is available
  set_SCL();
  clock_stretching(stretchTime_us);

  // Wait for SDA value to be written by slave, minimum of 4us for standard mode
  I2C_Delay.hns(t.high);

  // SCL is high, read out bit
  bit = read_SDA();

  // Set SCL low in preparation for next operation
  clear_SCL();

  I2C_Delay.hns(t.low);

  return bit;
}

// Write a byte to I2C bus. Return 0 if ack by the slave.
bool i2cbitbang::i2c_write_byte(unsigned char byte)
{
  unsigned bit;
  bool     nack;

  for (bit = 0; bit < 8; ++bit)
  {
    i2c_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }

  nack = i2c_read_bit();

  set_SDA();

  return nack;
}

// Read a byte from I2C bus
unsigned char i2cbitbang::i2c_read_byte(bool nack)
{
  unsigned char byte = 0;
  unsigned char bit;

  for (bit = 0; bit < 8; ++bit) {
    byte = (byte << 1) | i2c_read_bit();
  }

  i2c_write_bit(nack);
  set_SDA();

  return byte;
}

/*
 * timings are in hns units (0.1 um units)
 * e.g. 47 means 4.7 us
 */
#define MARGIN_100	-10
#define MARGIN_400  -6
#define MARGIN_50    10
#define MARGIN_10    0

#if (MARGIN_400 < -6)
#error "In I2Cbitbang class: MARGIN_100 need to be larger than -6"
#endif
#if (MARGIN_100 < -36)
#error "In I2Cbitbang class: MARGIN_100 need to be larger than 36"
#endif


void i2cbitbang::init_timings(void)
{
	switch (speedSelect )
	{
		case SPEED_400k:
			t.hdsta = 6 + MARGIN_400;
			t.susto = 6 + MARGIN_400;
			t.susta = 6 + MARGIN_400;
			t.sudat = 1;
			t.dvdat = 9 + MARGIN_400;
			t.dvack = 9 + MARGIN_400;
			t.high = 6 + MARGIN_400;
			t.low = 13 + MARGIN_400;
			t.buff = 13 + MARGIN_400;
			break;
		case SPEED_10k:
			t.hdsta = 400 + MARGIN_10;
			t.susto = 400 + MARGIN_10;
			t.susta = 470 + MARGIN_10;
			t.sudat = 30;
			t.dvdat = 360 + MARGIN_10;
			t.dvack = 360 + MARGIN_10;
			t.high = 400 + MARGIN_10;
			t.low = 470 + MARGIN_10;
			t.buff = 470 + MARGIN_10;
			break;
		case SPEED_50k:
			t.hdsta = 80 + MARGIN_50;
			t.susto = 80 + MARGIN_50;
			t.susta = 94 + MARGIN_50;
			t.sudat = 6;
			t.dvdat = 72 + MARGIN_50;
			t.dvack = 72 + MARGIN_50;
			t.high = 80 + MARGIN_50;
			t.low = 94 + MARGIN_50;
			t.buff = 94 + MARGIN_50;
			break;
		case SPEED_100k:
		default:
			t.hdsta = 40 + MARGIN_100;
			t.susto = 40 + MARGIN_100;
			t.susta = 47 + MARGIN_100;
			t.sudat = 3;
			t.dvdat = 36 + MARGIN_100;
			t.dvack = 36 + MARGIN_100;
			t.high = 40 + MARGIN_100;
			t.low = 47 + MARGIN_100;
			t.buff = 47 + MARGIN_100;
			break;
	}
}

uint32_t i2cbitbang::getInstance(void)
{
	return instance;
}
ackMode_t i2cbitbang::getAckMode(void)
{
	return ackMode;
}

void i2cbitbang::setAckMode(ackMode_t ack)
{
	ackMode = ack;
}

void i2cbitbang::writeData(uint8_t reg, uint8_t *pData, uint16_t size)
{
	bool nack;
	uint8_t *ptr = pData;

	i2c_bus_init();

	i2c_start_cond();

	/*send slave addres*/
	nack = i2c_write_byte(slaveAddress & 0xFE);

	if ( (ackMode == ACK_CHECK) && nack)	error |= I2CBB_ERROR_NACK;


	/*send register addres*/
	nack = i2c_write_byte(reg);

	if ( (ackMode == ACK_CHECK) && nack)	error |= I2CBB_ERROR_NACK;

	/*send data addres*/
	while(size)
	{
		nack = i2c_write_byte(*ptr++);
		if (nack) break;
		size--;
	}

	i2c_stop_cond();

	if ( (ackMode == ACK_CHECK) && nack)	error |= I2CBB_ERROR_NACK;


}

void i2cbitbang::writeReg(uint8_t reg, uint8_t val)
{
	writeData(reg, &val, 1);
}

void i2cbitbang::readData(uint8_t reg, uint8_t *pData, uint16_t size)
{
	bool nack;
	uint8_t *ptr;

	ptr = pData;

	i2c_bus_init();

	i2c_start_cond();
	nack = i2c_write_byte(slaveAddress & 0xFE);
	if ( (ackMode == ACK_CHECK) && nack)
	{
		error |= I2CBB_ERROR_NACK;
	}

	nack = i2c_write_byte(reg);
	if ( (ackMode == ACK_CHECK) && nack)
	{
		error |= I2CBB_ERROR_NACK;
	}

	if (i2cMode == MODE_SCCB)
	{
		i2c_stop_cond();
	}

	i2c_start_cond();

	nack = i2c_write_byte(slaveAddress | 0x01);
	if ( (ackMode == ACK_CHECK) && nack)
	{
		error |= I2CBB_ERROR_NACK;
	}
	while(size)
	{
		*ptr++ = i2c_read_byte( (size>1) ? ACK : NACK );
		size--;
	}

	i2c_stop_cond();

}

void i2cbitbang::readReg(uint8_t reg, uint8_t *pVal)
{
	readData(reg, pVal, 1);
}

uint8_t i2cbitbang::readReg(uint8_t reg)
{
	uint8_t val;

	readData(reg, &val, 1);

	return val;
}

i2cMode_t i2cbitbang::getI2CMode(void)
{
	return i2cMode;
}

void i2cbitbang::setI2CMode(i2cMode_t i2cm)
{
	i2cMode = i2cm;
}


uint32_t i2cbitbang::getError(void)
{
	return error;
}

void i2cbitbang::setSpeed(i2cbbSpeed_t fHz)
{
	speedSelect = fHz;
	init_timings();
}

void i2cbitbang::clock_stretching(uint32_t t_us)
{
	  while ( (read_SCL() == 0) && (t_us > 0) )
	  {
		I2C_Delay.us(1);
		t_us--;
		if (t_us == 0)
			error |= I2CBB_ERROR_STRETCH_TOUT;
	  }
}
