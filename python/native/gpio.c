/*
Copyright (c) 2012 Ben Croston / 2012-2013 Eric PTAK

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include "gpio.h"

#define GPIO_BASE_BP		(0x01C20000)//Keep pace with Wiringpi 	
#define SUNXI_GPIO_BASE		(0x01C20800)	

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

#define MAP_SIZE	(4096*2)
#define MAP_MASK	(MAP_SIZE - 1)

static volatile uint32_t *gpio_map;

struct tspair {
	struct timespec up;
	struct timespec down;
};

static struct pulse gpio_pulses[GPIO_COUNT];
static struct tspair gpio_tspairs[GPIO_COUNT];
static pthread_t *gpio_threads[GPIO_COUNT];

void short_wait(void)
{
    int i;
    
    for (i=0; i<150; i++)     // wait 150 cycles
    {
		asm volatile("nop");
    }
}

int setup(void)
{
    int mem_fd;
    uint8_t *gpio_mem;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
        return SETUP_DEVMEM_FAIL;
    }

    if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL)
        return SETUP_MALLOC_FAIL;

    if ((uint32_t)gpio_mem % PAGE_SIZE)
        gpio_mem += PAGE_SIZE - ((uint32_t)gpio_mem % PAGE_SIZE);

    gpio_map = (uint32_t *)mmap( (caddr_t)gpio_mem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, mem_fd, GPIO_BASE_BP);

    if(lemakerDebug)
		printf("gpio_mem = 0x%x\t gpio_map = 0x%x\n",gpio_mem,gpio_map);
		
    if ((uint32_t)gpio_map < 0)
        return SETUP_MMAP_FAIL;

    return SETUP_OK;
}

int get_gpio_number(int channel, unsigned int *gpio)
{
    if (*(*pin_to_gpio+channel) == -1)
	{
		//PyErr_SetString(PyExc_ValueError, "The channel sent is invalid on a Banana Pi");
		return 5;
	} else {
		*gpio = *(*pin_to_gpio+channel);	//pin_to_gpio is initialized in py_gpio.c, the last several lines
	}
    
	if(lemakerDebug)
		printf("GPIO = %d\n", *gpio);
    return 0;
}

int get_bcm_number(int gpio)
{
	int channel;
	int len = GPIO_COUNT;
	for (channel = 0; channel < len; channel++)
	{
		if(gpio == *(pinTobcm_BP + channel))
			break;
	}
	
	if(lemakerDebug)
		printf("channel = %d\n", channel);
		
	return channel;
}

uint32_t readl(uint32_t addr)
{
   uint32_t val = 0;
   uint32_t mmap_base = (addr & ~MAP_MASK);
   uint32_t mmap_seek = ((addr - mmap_base) >> 2);
   val = *(gpio_map + mmap_seek);
   
   if(lemakerDebug)
   		printf("mmap_base = 0x%x\t mmap_seek = 0x%x\t gpio_map = 0x%x\t total = 0x%x\n",mmap_base,mmap_seek,gpio_map,(gpio_map + mmap_seek));
		
   return val;
}

void writel(uint32_t val, uint32_t addr)
{
  uint32_t mmap_base = (addr & ~MAP_MASK);
  uint32_t mmap_seek = ((addr - mmap_base) >> 2);
  *(gpio_map + mmap_seek) = val;
}

void set_pullupdn(int gpio, int pud)
{
	 uint32_t regval = 0;
	 int bank = gpio >> 5;
	 int index = gpio - (bank << 5);
	 int sub = index >> 4;
	 int sub_index = index - 16*sub;
	 uint32_t phyaddr = SUNXI_GPIO_BASE + (bank * 36) + 0x1c + 4*sub; // +0x10 -> pullUpDn reg
	 
	if (lemakerDebug)
		printf("func:%s pin:%d,bank:%d index:%d sub:%d phyaddr:0x%x\n",__func__, gpio,bank,index,sub,phyaddr); 

	regval = readl(phyaddr);
	if (lemakerDebug)
		printf("pullUpDn reg:0x%x, pud:0x%x sub_index:%d\n", regval, pud, sub_index);
	regval &= ~(3 << (sub_index << 1));
	regval |= (pud << (sub_index << 1));
	if (lemakerDebug)
		printf("pullUpDn val ready to set:0x%x\n", regval);
	writel(regval, phyaddr);
	regval = readl(phyaddr);
	if (lemakerDebug)
		printf("pullUpDn reg after set:0x%x  addr:0x%x\n", regval, phyaddr);	
}

//updated Eric PTAK - trouch.com
void set_function(int gpio, int function, int pud)
{
	if (function == PWM) {
		function = OUT;
		enablePWM(gpio);
	}
	else {
		//disablePWM(gpio);
	}
	

	uint32_t regval = 0;
	int bank = gpio >> 5;
	int index = gpio - (bank << 5);
	int offset = ((index - ((index >> 3) << 3)) << 2);
	uint32_t phyaddr = SUNXI_GPIO_BASE + (bank * 36) + ((index >> 3) << 2);
	if (lemakerDebug)
		printf("func:%s pin:%d, function:%d bank:%d index:%d phyaddr:0x%x\n",__func__, gpio , function,bank,index,phyaddr); 
		
	regval = readl(phyaddr);
	if (lemakerDebug)
		printf("read reg val: 0x%x offset:%d\n",regval,offset);
		
	set_pullupdn(gpio, pud);
	    
	if(IN == function)
	{
		regval &= ~(7 << offset);
		writel(regval, phyaddr);
		regval = readl(phyaddr);
		if (lemakerDebug)
			printf("Input mode set over reg val: 0x%x\n",regval);
	}
	else if(OUT == function)
	{
	   regval &= ~(7 << offset);
	   regval |=  (1 << offset);
	   if (lemakerDebug)
			printf("Out mode ready set val: 0x%x\n",regval);
	   writel(regval, phyaddr);
	   regval = readl(phyaddr);
	   if (lemakerDebug)
			printf("Out mode set over reg val: 0x%x\n",regval);
	}
	else
	{
		printf("line:%dpin number error\n",__LINE__);
	}
}

//added Eric PTAK - trouch.com
int get_function(int gpio)
{
   	uint32_t regval = 0;
	int bank = gpio >> 5;
	int index = gpio - (bank << 5);
	int offset = ((index - ((index >> 3) << 3)) << 2);
	uint32_t phyaddr = SUNXI_GPIO_BASE + (bank * 36) + ((index >> 3) << 2);
		
	regval = readl(phyaddr);
	if (lemakerDebug)
		printf("read reg val: 0x%x offset:%d\n",regval,offset);
	    
	regval >>= offset;
	regval &= 7;
	if (lemakerDebug)
		printf("read reg val_getFunc: 0x%x\n",regval);
	
	//if(regval != 0 || regval != 1)
	//	regval = 4;
	return regval;// 0=input, 1=output, 4=alt0
}

//updated Eric PTAK - trouch.com
int input(int gpio)
{
	 uint32_t regval = 0;
	 int bank = gpio >> 5;
	 int index = gpio - (bank << 5);
	 uint32_t phyaddr = SUNXI_GPIO_BASE + (bank * 36) + 0x10; // +0x10 -> data reg
	   if (lemakerDebug)
			printf("func:%s pin:%d,bank:%d index:%d phyaddr:0x%x\n",__func__, gpio,bank,index,phyaddr); 

	  regval = readl(phyaddr);
	  regval = regval >> index;
	  regval &= 1;
		if (lemakerDebug)
			printf("***** read reg val: 0x%x,bank:%d,index:%d,line:%d\n",regval,bank,index,__LINE__);
	  return regval;
}

void output(int gpio, int value)
{
	uint32_t regval = 0;
	int bank = gpio >> 5;
	int index = gpio - (bank << 5);
	uint32_t phyaddr = SUNXI_GPIO_BASE + (bank * 36) + 0x10; // +0x10 -> data reg
	if (lemakerDebug)
		printf("func:%s pin:%d, value:%d bank:%d index:%d phyaddr:0x%x\n",__func__, gpio , value,bank,index,phyaddr);
	
	regval = readl(phyaddr);
	if (lemakerDebug)
		printf("before write reg val: 0x%x,index:%d\n",regval,index);
	if(0 == value)
	{
		regval &= ~(1 << index);
		writel(regval, phyaddr);
		regval = readl(phyaddr);
		if (lemakerDebug)
			printf("LOW val set over reg val: 0x%x\n",regval);
	}
	else
	{
		regval |= (1 << index);
		writel(regval, phyaddr);
		regval = readl(phyaddr);
		if (lemakerDebug)
			printf("HIGH val set over reg val: 0x%x\n",regval);
	}	
}

//added Eric PTAK - trouch.com
void outputSequence(int gpio, int period, char* sequence) {
	int i, value;
	struct timespec ts;
	ts.tv_sec = period/1000;
	ts.tv_nsec = (period%1000) * 1000000;

	for (i=0; sequence[i] != '\0'; i++) {
		if (sequence[i] == '1') {
			value = 1;
		}
		else {
			value = 0;
		}
		output(gpio, value);
	    nanosleep(&ts, NULL);
	}
}

void resetPWM(int gpio) {
	int channel;
	channel = get_bcm_number(gpio);
        
	gpio_pulses[channel].type = 0;
	gpio_pulses[channel].value = 0;

	gpio_tspairs[channel].up.tv_sec = 0;
	gpio_tspairs[channel].up.tv_nsec = 0;
	gpio_tspairs[channel].down.tv_sec = 0;
	gpio_tspairs[channel].down.tv_nsec = 0;
}

//added Eric PTAK - trouch.com
void pulseTS(int gpio, struct timespec *up, struct timespec *down) {
      
	if ((up->tv_sec > 0) || (up->tv_nsec > 0)) {
		output(gpio, 1);
		nanosleep(up, NULL);
	}

	if ((down->tv_sec > 0) || (down->tv_nsec > 0)) {
		output(gpio, 0);
		nanosleep(down, NULL);
	}
}

//added Eric PTAK - trouch.com
void pulseOrSaveTS(int gpio, struct timespec *up, struct timespec *down) {
	int channel;
	channel = get_bcm_number(gpio);
        
	if (gpio_threads[channel] != NULL) {
		memcpy(&gpio_tspairs[channel].up, up, sizeof(struct timespec));
		memcpy(&gpio_tspairs[channel].down, down, sizeof(struct timespec));
	}
	else {
		pulseTS(gpio, up, down);
	}
}

//added Eric PTAK - trouch.com
void pulseMilli(int gpio, int up, int down) {
	struct timespec tsUP, tsDOWN;

	tsUP.tv_sec = up/1000;
	tsUP.tv_nsec = (up%1000) * 1000000;

	tsDOWN.tv_sec = down/1000;
	tsDOWN.tv_nsec = (down%1000) * 1000000;
	pulseOrSaveTS(gpio, &tsUP, &tsDOWN);
}

//added Eric PTAK - trouch.com
void pulseMilliRatio(int gpio, int width, float ratio) {
	int up = ratio*width;
	int down = width - up;
	pulseMilli(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseMicro(int gpio, int up, int down) {
	struct timespec tsUP, tsDOWN;

	tsUP.tv_sec = 0;
	tsUP.tv_nsec = up * 1000;

	tsDOWN.tv_sec = 0;
	tsDOWN.tv_nsec = down * 1000;
	pulseOrSaveTS(gpio, &tsUP, &tsDOWN);
}

//added Eric PTAK - trouch.com
void pulseMicroRatio(int gpio, int width, float ratio) {
	int up = ratio*width;
	int down = width - up;
	pulseMicro(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseAngle(int gpio, float angle) {
	int channel;
	channel = get_bcm_number(gpio);
        
	gpio_pulses[channel].type = ANGLE;
	gpio_pulses[channel].value = angle;
	int up = 1520 + (angle*400)/45;
	int down = 20000-up;
	pulseMicro(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseRatio(int gpio, float ratio) {
	int channel;
	channel = get_bcm_number(gpio);
        
	gpio_pulses[channel].type = RATIO;
	gpio_pulses[channel].value = ratio;
	int up = ratio * 20000;
	int down = 20000 - up;
	pulseMicro(gpio, up, down);
}

struct pulse* getPulse(int gpio) {
	int channel;
	channel = get_bcm_number(gpio);
        
	return &gpio_pulses[channel];
}

//added Eric PTAK - trouch.com
void* pwmLoop(void* data) {
	int gpio = (int)data;

	int channel;
	channel = get_bcm_number(gpio);

	//printf("-----pwmLoop %d\n",channel);
	while (1) {
		pulseTS(gpio, &gpio_tspairs[channel].up, &gpio_tspairs[channel].down);
	}
}

//added Eric PTAK - trouch.com
void enablePWM(int gpio) {
	int channel;
	channel = get_bcm_number(gpio);
        
	pthread_t *thread = gpio_threads[channel];
	if (thread != NULL) {
		return;
	}

	resetPWM(gpio);

	thread = (pthread_t*) malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, pwmLoop, (void*)gpio);
	gpio_threads[channel] = thread;
	//printf("----set_function  %d\n", channel);
}

//added Eric PTAK - trouch.com
void disablePWM(int gpio) {
	int channel;
	channel = get_bcm_number(gpio);

	pthread_t *thread = gpio_threads[channel];
	if (thread == NULL) {
		return;
	}

	pthread_cancel(*thread);
	gpio_threads[channel] = NULL;
	output(gpio, 0);
	resetPWM(gpio);
}

//added Eric PTAK - trouch.com
int isPWMEnabled(int gpio) {
	int channel;
	channel = get_bcm_number(gpio);

	return gpio_threads[channel] != NULL;
}


void cleanup(void)
{
    // fixme - set all gpios back to input
    munmap((caddr_t)gpio_map, BLOCK_SIZE);
}
