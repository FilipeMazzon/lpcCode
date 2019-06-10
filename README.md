# lpcCode
Este e um projeto para testar o lpc1769 utilizando o [FreeRTOS](https://www.freertos.org/) lidando com tarefas com filas de prioridades.

## sumario:
+ [Descricao Geral](#descricao-geral)
+ [Tecnologias](#tecnologias)
+ [instalacao](#instalacao)
+ [requisitos](#requisitos)
+ [diagramas](#diagramas)
    + [Diagrama de Blocos](#diagrama-de-blocos)
    + [Camada de Hardware](#camada-de-hardware)
+ [Implementacao](#implementacao)
    + [Tasks](#tasks)
		+ [Contador 1](#contador-1)
		+ [Contador 2](#contador-2)
		+ [display](#display)
		+ [Sensor de luminosidade](#sensor-de-luminosidade)
		+ [trimpot](#trimpot)
+ [contatos](#contatos)
+ [licensa](#licensa)

### Descricao Geral
O projeto consiste na leitura de dados dos sensores de luminosidade e potenciômetro rotativo, cada um com uma tarefa específica. Os dados coletados são colocados em uma fila de mensagem.
A barra de LEDS verde deverá ser acionada conforme o valor do potenciômetro rotativo, 10%, 30%, 60% e 90%, e se acima desse valor, a barra de LEDS vermelha deverá acender.
Duas tarefas com prioridades diferentes incrementam contadores a cada 100ms.
Uma tarefa consiste em mostrar no display os valores de luminosidade, porcentagem do potenciômetro e os dois contadores, sendo essa tarefa executada em background(IdleHook).

### tecnologias
+ linguagem C.
+ IDE LPCExpress
### instalacao
Recomendamos utilizar o Ubuntu 18.04 ou superior.
Abra o terminal e execute o comando abaixo:
```ssh
sudo apt-get install libgtk2.0-0:i386 libxtst6:i386 libpangox-1.0-0 libpangoxft-1.0-0:i386 libidn11:i386 libglu1-mesa libncurses5:i386 libudev1:i386 libusb-1.0:i386 libusb-0.1 gtk2-engines-murrine:i386 libnss3
```
Após a instalação dos pacotes faça o download do arquivo e dentro da pasta dele execute:
```
ssh
./Installer_LPCXpresso_8.2.2_650_Linux-x86
```
Aceite os termos e aguarde ser instalado a aplicação.
### requisitos


### Diagramas
#### Diagrama de Blocos

![](https://github.com/FilipeMazzon/lpcCode/blob/master/images/diagrama-de-blocos.jpg)
#### Camada de Hardware
![](https://github.com/FilipeMazzon/lpcCode/blob/master/images/camada-de-hardware.jpeg)
### implementacao
#### Tasks
##### Contador 1
```
    static void vSenderTaskCount1( void *pvParameters )
    {
    //Definindo o tempo de aguardo de 100ms
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
    
    	for(;;){
    		contador1++;
    
    		vTaskDelay( xTicksToWait );
    	}
    }
 ```
##### Contador 2
```
    static void vSenderTaskCount2( void *pvParameters )
    {
    //Definindo o tempo de aguardo de 100ms
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
    
    	for(;;){
    		contador2++;
    
    		vTaskDelay( xTicksToWait );
    	}
    }
```
##### display
```
    static void vReceiverTask( void *pvParameters )
    {
    xData xReceivedStructure;
    portBASE_TYPE xStatus;
    	for( ;; )
    	{
   
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
```
##### Sensor de luminosidade
```
static void vSenderTaskLum( void *pvParameters )
{
    portBASE_TYPE xStatus;
    const portTickType xTicksToWait = 600 / portTICK_RATE_MS;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
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
```
##### trimpot
```
static void vSenderTaskPot( void *pvParameters )
{
    portBASE_TYPE xStatus;
    const portTickType xTicksToWait = 200 / portTICK_RATE_MS;
    uint16_t ledOn = 0;
    uint16_t ledOff = 0;
	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
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
```
### contatos
[Filipe Firmino](mailto:filipefirmino@gec.inatel.br)

[Filipe Mazzon Ribeiro](mailto:filipemazzon@gec.inatel.br)

[Matheus Henrique da Silva](mailto:matheushenriquesilva@gec.inatel.br)

### licensa

[MIT](https://github.com/FilipeMazzon/lpcCode/blob/master/LICENSE)
