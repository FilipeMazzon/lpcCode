/*
===============================================================================
 Name        : ExercicioNP2.c
 Author      : Filipe Firmino (1162), Filipe Mazzon (1177), Matheus da Silva (1136)
 Version     : 1.0.0
===============================================================================
*/

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "LPC17xx.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "rotary.h"
#include "led7seg.h"
#include "oled.h"
#include "rgb.h"
#include "light.h"

/* Demo includes. */
#include "basic_io.h"

// CodeRed - added for use in dynamic side of web page
unsigned int aaPagecounter=0;
unsigned int adcValue = 0;

uint32_t t = 0;
uint32_t lux = 0;
uint32_t trim = 0;
uint32_t potenciometro = 0;
uint32_t trimMax = 4095;
uint32_t msTicks = 0;

static void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
{
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;

}

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
		PINSEL_CFG_Type PinCfg;

		/*
		 * Initialize SPI pin connect
		 * P0.7 - SCK;
		 * P0.8 - MISO
		 * P0.9 - MOSI
		 * P2.2 - SSEL - used as GPIO
		 */
		PinCfg.Funcnum = 2;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 0;
		PinCfg.Pinnum = 7;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 8;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 9;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Funcnum = 0;
		PinCfg.Portnum = 2;
		PinCfg.Pinnum = 2;
		PINSEL_ConfigPin(&PinCfg);

		SSP_ConfigStructInit(&SSP_ConfigStruct);

		// Initialize SSP peripheral with parameter given in structure above
		SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

		// Enable SSP peripheral
		SSP_Cmd(LPC_SSP1, ENABLE);

}

static void init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_adc(void)
{
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init ADC pin connect
	 * AD0.0 on P0.23
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* Configuration for ADC :
	 * 	Frequency at 1Mhz
	 *  ADC channel 0, no Interrupt
	 */
	ADC_Init(LPC_ADC, 1000000);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_0,DISABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);

}

//this handler occurs every 1ms
void SysTick_Handler(void) {
    msTicks++;
}

//get current tick (time)
static uint32_t getTicks(void) {
    return msTicks;
}


#define mainSENDER_1 1
#define mainSENDER_2 2
static uint32_t ch = 0;
static uint32_t contador1 = 0;
static uint32_t contador2 = 0;

/* The tasks to be created.  Two instances are created of the sender task while
only a single instance is created of the receiver task. */
static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );
static void vSenderTaskCount1( void *pvParameters );
static void vSenderTaskCount2( void *pvParameters );
static void vSenderTaskLum( void *pvParameters );
static void vSenderTaskPot( void *pvParameters );
void vApplicationIdleHook( void );
/*
/*-----------------------------------------------------------*/

/* Declare a variable of type xQueueHandle.  This is used to store the queue
that is accessed by all three tasks. */
xQueueHandle xQueue;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
	uint32_t ucValue;
	unsigned char ucSource;
} xData;

/* Declare two variables of type xData that will be passed on the queue. */
xData xStructsToSend[ 2 ] =
{
	{ 0, mainSENDER_1 },
	{ 0, mainSENDER_2 },
};

static uint8_t barPos = 2;
static void moveBar(uint8_t steps, uint8_t dir)
{
    uint16_t ledOn = 0;

    if (barPos == 0)
        ledOn = (1 << 0) | (3 << 14);
    else if (barPos == 1)
        ledOn = (3 << 0) | (1 << 15);
    else
        ledOn = 0x07 << (barPos-2);

    barPos += (dir*steps);
    barPos = (barPos % 16);

    pca9532_setLeds(ledOn, 0xffff);
}


int main( void )
{
	//Inicializacoes necessarias
	init_i2c();
	init_ssp();
	init_adc();

	rotary_init();
	uint32_t potenciometro = 0;
	uint32_t ch = 0;

	pca9532_init();

	rgb_init();
	oled_init();
	light_init();

	temp_init(&getTicks);

	light_enable();
	light_setRange(LIGHT_RANGE_4000);

	oled_clearScreen(OLED_COLOR_WHITE);

    /* The queue is created to hold a maximum of 4 structures of type xData. */
    xQueue = xQueueCreate( 4, sizeof( xData ) );

	if( xQueue != NULL )
	{
		/* Create two instances of the task that will write to the queue.  The
		parameter is used to pass the structure that the task should write to the
		queue, so one task will continuously send xStructsToSend[ 0 ] to the queue
		while the other task will continuously send xStructsToSend[ 1 ].  Both
		tasks are created at priority 2 which is above the priority of the receiver. */

		xTaskCreate( vSenderTaskLum, "Luminosidade", 240, ( void * ) &( xStructsToSend[ 0 ] ), 3, NULL ); //luminosidade
		xTaskCreate( vSenderTaskPot, "Potenciometro", 240, ( void * ) &( xStructsToSend[ 1 ] ), 2, NULL ); //potenciometro
		xTaskCreate( vSenderTaskCount1, "Contador1", 240, NULL, 4, NULL ); //contador 1
		xTaskCreate( vSenderTaskCount2, "Contador2", 240, NULL, 0, NULL ); //contador 2


		/* Create the task that will read from the queue.  The task is created with
		priority 1, so below the priority of the sender tasks. */
		xTaskCreate( vReceiverTask, "Receiver", 240, NULL, 1, NULL );

		/* Start the scheduler so the created tasks start executing. */
		vTaskStartScheduler();
	}
	else
	{
		/* The queue could not be created. */
	}

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/


static void vSenderTaskPot( void *pvParameters )
{
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 200 / portTICK_RATE_MS;
uint16_t ledOn = 0;
uint16_t ledOff = 0;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* The first parameter is the queue to which data is being sent.  The
		queue was created before the scheduler was started, so before this task
		started to execute.

		The second parameter is the address of the structure being sent.  The
		address is passed in as the task parameter.

		The third parameter is the Block time - the time the task should be kept
		in the Blocked state to wait for space to become available on the queue
		should the queue already be full.  A block time is specified as the queue
		will become full.  Items will only be removed from the queue when both
		sending tasks are in the Blocked state.. */

		/* trimpot*/
		ADC_StartCmd(LPC_ADC,ADC_START_NOW);
		//Wait conversion complete
		while (!(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_0,ADC_DATA_DONE)));
		trim = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0);
		xStructsToSend[ 0 ].ucValue = ((float)trim/4095)*100;

		if (trim <= 409){// ate 10%
			//ledOn = (3 << 0);
			ledOn = (3 << 7)|(3 << 8);
		} else
		if (trim > 409 && trim <= 1228){// ate 30%
			ledOn = (3 << 7)|(3 << 8)|
					(3 << 9)|(3 << 10);
		} else
		if (trim > 1228 && trim <= 2457){//ate 60%
			ledOn = (3 << 7)|(3 << 8)|
					(3 << 9)|(3 << 10)|
					(3 << 11)|(3 << 12);
		} else
		if (trim > 2457 && trim <= 3685){// ate 90%
			ledOn = (3 << 7)|(3 << 8)|
					(3 << 9)|(3 << 10)|
					(3 << 11)|(3 << 12)|
					(3 << 13)|(3 << 14);
		} else
		if (trim > 3685){// acima de 90%
			ledOn = (3 << 0)|(3 << 2)|
					(3 << 4)|(3 << 6);
		}
		//ledOn = (3 << 0) | (3 << 2) |(3 << 4) | (3 << 6);
		pca9532_setLeds(ledOn, 0xffff);

		xStatus = xQueueSendToBack( xQueue, pvParameters, xTicksToWait );

		if( xStatus != pdPASS )
		{
			/* We could not write to the queue because it was full - this must
			be an error as the receiving task should make space in the queue
			as soon as both sending tasks are in the Blocked state. */
			//vPrintString( "Could not send to the queue.\n" );
		}

		/* Allow the other sender task to execute. */
		taskYIELD();

		vTaskDelay( xTicksToWait );

	}
}


static void vSenderTaskLum( void *pvParameters )
{
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 600 / portTICK_RATE_MS;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* The first parameter is the queue to which data is being sent.  The
		queue was created before the scheduler was started, so before this task
		started to execute.

		The second parameter is the address of the structure being sent.  The
		address is passed in as the task parameter.

		The third parameter is the Block time - the time the task should be kept
		in the Blocked state to wait for space to become available on the queue
		should the queue already be full.  A block time is specified as the queue
		will become full.  Items will only be removed from the queue when both
		sending tasks are in the Blocked state.. */
		xStatus = xQueueSendToBack( xQueue, pvParameters, xTicksToWait );

		if( xStatus != pdPASS )
		{
			/* We could not write to the queue because it was full - this must
			be an error as the receiving task should make space in the queue
			as soon as both sending tasks are in the Blocked state. */
			//vPrintString( "Could not send to the queue.\n" );
		}
		//Leitura do sensor de luminosidade
		lux = light_read();
		xStructsToSend[ 1 ].ucValue = lux;
		/* Allow the other sender task to execute. */
		taskYIELD();

		vTaskDelay( xTicksToWait );
	}
}

static void vSenderTaskCount1( void *pvParameters )
{
//Definindo o tempo de aguardo de 100ms
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	for(;;){
		contador1++;

		vTaskDelay( xTicksToWait );
	}
}

static void vSenderTaskCount2( void *pvParameters )
{
//Definindo o tempo de aguardo de 100ms
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	for(;;){
		contador2++;

		vTaskDelay( xTicksToWait );
	}
}
/*-----------------------------------------------------------*/

static void vReceiverTask( void *pvParameters )
{
/* Declare the structure that will hold the values received from the queue. */
xData xReceivedStructure;
portBASE_TYPE xStatus;
static uint8_t bufContadores[10];

	/* This task is also defined within an infinite loop. */
	for( ;; )
	{
		/* As this task only runs when the sending tasks are in the Blocked state,
		and the sending tasks only block when the queue is full, this task should
		always find the queue to be full.  3 is the queue length. */

		/*if( uxQueueMessagesWaiting( xQueue ) != 7 )
		{
			vPrintString( "Queue should have been full!\n" );
		}*/

		/* The first parameter is the queue from which data is to be received.  The
		queue is created before the scheduler is started, and therefore before this
		task runs for the first time.

		The second parameter is the buffer into which the received data will be
		placed.  In this case the buffer is simply the address of a variable that
		has the required size to hold the received structure.

		The last parameter is the block time - the maximum amount of time that the
		task should remain in the Blocked state to wait for data to be available
		should the queue already be empty.  A block time is not necessary as this
		task will only run when the queue is full so data will always be available. */
		xStatus = xQueueReceive( xQueue, &xReceivedStructure, 0 );

		if( xStatus == pdPASS )
		{
			/* Data was successfully received from the queue, print out the received
			value and the source of the value.*/
			if(xReceivedStructure.ucSource == mainSENDER_1)
			{
				static uint8_t buf[10];
				vPrintStringAndNumber( "Trimpot = ", xReceivedStructure.ucValue );
				oled_putString(1,9,  (uint32_t*)"Trimpot (%): ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
				intToString(xReceivedStructure.ucValue, buf, 10, 10);
				oled_fillRect(67,9, 90, 15, OLED_COLOR_WHITE);
				oled_putString(67,9, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
			} else
			if( xReceivedStructure.ucSource == mainSENDER_2 )
			{
				static uint8_t buf[10];
				intToString(xReceivedStructure.ucValue, buf, 10, 10);
				vPrintStringAndNumber( "Luminosidade = ", xReceivedStructure.ucValue );
				oled_putString(1,1,  (uint32_t*)"Lum: ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
				oled_fillRect(67,1, 90, 8, OLED_COLOR_WHITE);
				oled_putString(67,1, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

			}


			//Contador 1
			intToString(contador1, bufContadores, 10, 10);
			vPrintStringAndNumber( "Contador 1 = ", contador1);
			oled_putString(1,18,  (uint32_t*)"Contador1: ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
			oled_fillRect(67,1, 90, 8, OLED_COLOR_WHITE);
			oled_putString(67,18, bufContadores, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

			//Contador 2
			intToString(contador2, bufContadores, 10, 10);
			vPrintStringAndNumber( "Contador 2 = ", contador2);
			oled_putString(1,27,  (uint32_t*)"Contador2: ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
			oled_fillRect(67,1, 90, 8, OLED_COLOR_WHITE);
			oled_putString(67,27, bufContadores, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
		}
		else
		{
			/* We did not receive anything from the queue.  This must be an error
			as this task should only run when the queue is full. */
			//vPrintString( "Could not receive from the queue.\n" );
		}
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}
