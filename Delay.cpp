/*
 * Delays.cpp
 *
 *  Created on: Oct 18, 2019
 *      Author: Tata
 */

#include <Delay.h>

static int32_t initDone = 0;

Delay::Delay() {
	// TODO Auto-generated constructor stub
	if (!initDone)
	{
	//  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	  {
	    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	    DWT->CYCCNT = 0;
	    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	  }
	  initDone = 1;	//initialize only once regardless of number of objects
	}
}

Delay::~Delay() {
	// TODO Auto-generated destructor stub
}

/*
 * hns delay in 0.1 us (or 100ns) units
 * eg. Delay.hns(47) means 4.7 us delay
 */
void Delay::hns(uint32_t hns) // hundreds of nanosec delay
{
	uint32_t curDwt = DWT->CYCCNT;
	if (hns > 0)
	{
		uint32_t tp = ( hns * (SystemCoreClock/1000000UL) + 1)/10;
		while ( (DWT->CYCCNT - curDwt) < tp);
	}
}

void Delay::us(uint32_t us) // microseconds
{
	uint32_t curDwt = DWT->CYCCNT;
	if (us > 0)
	{
		uint32_t tp = us * (SystemCoreClock/1000000UL);
		while ( (DWT->CYCCNT - curDwt) < tp);
	}
}

void Delay::ms(uint32_t ms) // microseconds
{
	uint32_t m = ms;
	while(m--) us(1000UL);
}
