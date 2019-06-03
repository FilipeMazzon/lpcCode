# lpcCode
Este e um projeto para testar o lpc1769 utilizando o FreeRTOS lidando com tarefas com filas de prioridades.

## sumario:
+ [Descricao Geral](#descricao-geral)
+ [Tecnologias](#tecnologias)
+ [instalacao](#instalacao)
+ [requisitos](#requisitos)
+ [Implementacao](#implementacao)
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


### requisitos

### implementacao
+ Tasks
    + Contador 1
    ```c
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
    + Contador 2
    ```c
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
    + display
    ```c
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
### contatos

### licensa

[MIT](/blob/master/LICENSE)



