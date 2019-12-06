# i2cbitbang
Recently I needed bit-bang i2c for creating SCCB communication which is slightly different from i2c as it has ACK bit as Don't care bit. Since I cound't find any lib for my needs, I wrote my own one in C++. It is based on i2c template from https://en.wikipedia.org/wiki/I%C2%B2C.
This is bit-banging version of i2c for STM32F4 with possibility to create many i2c ports (limited by GPIO number). It supports clock stretching and different speeds according to standards. It does not support arbitration lost, but it can be easily added by yourself.
It can be easily adopted to other STM32 microcontrollers. This lib uses Delay class which is based on DWT timer available in STMs.

<p><b>Instruction</b></P>

1. Edit <code>i2cbitbang_board.h</code> file and specify your i2c ports

<p>a. set a total number of i2c ports in the define NUMBER_OF_I2CBB_INSTANCES</p>
<p>b. modify entries in <code>i2cbbConfigTable</code> to specify GPIO port and GPIO pin number</p>
<p>c. provide a name of each i2c instance, for example:</p>
<code>
<p>#define I2CBB_OLED	 0</p>
<p>#define I2CBB_LSM	 1</p>
</code>

<p>2. Use in the code</p>
<p><code>i2cbitbang i2cport(instance, slaveAddress)</code>, for example: <code>i2cport(I2CBB_OLED, 0x3C)</code> for OLED display ssd1306.</p>
<p>Now you can modify some options, e.g</p>
<P><code>i2cport.setSpeed(SPEED_400k); </code> //there are four speeds available SPEED_10k, SPEED_50k, SPEED_100k, SPEED_400k(default)</P>
<P><code>i2cport.setAckMode(ACK_CHECK);</code> //ACK_CHECK (default) is for normal I2C operation, ACK_IGNORE was provided for SCCB protocol e.g. for OV7670 camera</P>
<P><code>i2cport.setI2CMode(MODE_I2C);</code>  //MODE_I2C (default) for normal I2C operation, MODE_SCCB is for SCCB protocol (please use with ACK_IGNORE option)</P>

<P>API is quite typical and can be easily understood by verifying class public components.</p>
<P>E.g. to write to the i2c port: <code>i2cport.writeReg(reg, val);</code></p>
<P>To read data into a register: <code>i2cport.readReg(reg, &val);</code> or <code>val = i2cport.readReg(reg);</code></p>

<P>There is an <code>error</code> member of the class to which you can reach by <code>getError()</code>. The <code>error</code> member 
is set to I2CBB_ERROR_NONE in the i2c_bus_init() private function in the i2cbitbang_driver.cpp file, hece to check communication error 
you need to getError right after any read or write command.</p>

<p>This i2c protocol 
<p>- does NOT support arbitration lost, but it can be easily implemented by yourself.</p>
<P>- supports clock stretching with timeout set to <code>stretchTime_us = 1000; //in us</code> in the i2cbitbang constructor.

Enjoy!.
TB.

