/*
 * Delays.h
 *
 *  Created on: Oct 18, 2019
 *      Author: Tata
 */

#ifndef DELAY_H_
#define DELAY_H_

#include "stm32f4xx_hal.h"

class Delay {

private:
	void DWT_Init(void);
//	uint32_t DWT_Get(void);
//	uint8_t DWT_Compare(int32_t tp);

public:
	Delay();
	virtual ~Delay();

	void hns(uint32_t hns);
	void us(uint32_t);
	void ms(uint32_t);

};

#endif /* DELAY_H_ */
