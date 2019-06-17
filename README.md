# LPCCode
Este é um projeto para testar a placa LPC1769 utilizando o [FreeRTOS](https://www.freertos.org/) lidando com tarefas com filas de prioridades, sensores da placa Base-Board.

## Sumário:
+ [Descricao Geral](#descricao-geral)
+ [Tecnologias](#tecnologias)
+ [Instalando o ambiente de desenvolvimento](#instalando-o-ambiente-de-desenvolvimento)
+ [Requisitos](#requisitos)
+ [Diagramas](#diagramas)
    + [Diagrama de Blocos](#diagrama-de-blocos)
    + [Camada de Hardware](#camada-de-hardware)
+ [Implementacao](#implementacao)
    + [Tasks](#tasks)
		+ [Contador 1](#contador-1)
		+ [Contador 2](#contador-2)
		+ [Display OLED](#display)
		+ [Sensor de luminosidade](#sensor-de-luminosidade)
		+ [Trimpot](#trimpot)
+ [Bibliotecas de desenvolvimento](#bibliotecas-de-desenvolvimento)
    + [Utilizando as bibliotecas de desenvolvimento](#utilizando-as-bibliotecas-de-desenvolvimento)
+ [Contatos](#contatos)
+ [Licença](#licença)

### Descricao Geral
O projeto consiste na leitura de dados dos sensores de luminosidade e potenciômetro rotativo, cada um com uma tarefa específica. Os dados coletados são colocados em uma fila de mensagem.
A barra de LEDS verde deverá ser acionada conforme o valor do potenciômetro rotativo, 10%, 30%, 60% e 90%, e se acima desse valor, a barra de LEDS vermelha deverá acender.
Duas tarefas com prioridades diferentes incrementam contadores a cada 100ms.
Uma tarefa consiste em mostrar no display os valores de luminosidade, porcentagem do potenciômetro e os dois contadores, sendo essa tarefa executada em background(IdleHook).

### Tecnologias
+ Linguagem C;
+ IDE LPCXpress;
+ Sistema operacional de tempo real (RTOS);
+ Sensor de luminosidade disponível na placa Base-Board;
+ Potênciometro trimpot disponível também na placa.
+ Middleware para o desenvolvimento de aplicações de tempo real.

### Requisitos
+ Processador dual core 2 GHz ou superior; 
+ 4 GB memória RAM;
+ 32 GB livres no Hard-Disk (HD) *Contando também de que o sistema Linux será instalado na máquina;
+ Portas USB para implementação e DEBUGGING na placa;
+ Acesso à internet.

### Instalando o ambiente de desenvolvimento
Para o ambiente de desenvolvimento foi utilizado a distribuição [Ubuntu 18.04](https://ubuntu.com/download/desktop) do Linux recomendamos utilizar esta ou superior.
Abra o terminal e execute o comando abaixo para instalar as depedências necessárias do ambiente de desenvolvimento:
```ssh
sudo apt-get install libgtk2.0-0:i386 libxtst6:i386 libpangox-1.0-0 libpangoxft-1.0-0:i386 libidn11:i386 libglu1-mesa libncurses5:i386 libudev1:i386 libusb-1.0:i386 libusb-0.1 gtk2-engines-murrine:i386 libnss3
```
Após a instalação dos pacotes execute o comando a seguir no terminal:
```ssh
cd /lib/i386-linux-gnu
sudo ln -sf libudev.so.1 libudev.so.0
```
Feche o terminal.

Faça o [download](http://www.mediafire.com/file/l1cy63mgzqape29/Installer_LPCXpresso_8.2.2_650_Linux-x86.zip/file) do arquivo de instalação do LPCXPRESSO e descompacte o *.zip*. A pasta gerada após a descompactação, abra o terminal dentro dela e execute o comando a seguir:
```ssh
./Installer_LPCXpresso_8.2.2_650_Linux-x86
```
Aceite os termos, avançe com a instalação de modo padrão com as configurações e aguarde ser instalado.

Com o término da instalação abra o terminal e execute o comando a seguir:
```ssh
export UBUNTU_MENUPROXY=0
```
Procure nos aplicativos instalados do sistema o LPCXPRESSO, caso não tenha disponível para abertura execute o comando a seguir no terminal:
```ssh
/usr/local/lpcxpresso_8.2.2_650/lpcxpresso/lpcxpresso
```

### Diagramas
#### Diagrama de Blocos

![](https://github.com/FilipeMazzon/lpcCode/blob/master/images/diagrama-de-blocos.jpg)
#### Camada de Hardware
![](https://github.com/FilipeMazzon/lpcCode/blob/master/images/camada-de-hardware.jpeg)
### Implementação
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
##### Display OLED
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
##### Trimpot
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

### Bibliotecas de desenvolvimento
Para auxiliar no desenvolvimento do projeto simples no FreeRTOS, faça o [download](http://www.mediafire.com/file/1mcy1sfbtawt6w7/FreeRTOS-exemplos-CMSISv2p00.zip/file) do *.zip* que contém o código fonte necessário para executar os exemplos disponíveis e tratar os eventos. Para acessar os periféricos da Base-Board, faça [download](http://www.mediafire.com/file/xyqc8j3g4f76k3q/ExemplosBaseBoard-CMSIS2p00.zip/file) do .zip. Como importar e utilizar destes pacotes será abordado no próximo tópico.

### Utilizando as bibliotecas de desenvolvimento
Paragráfo em construção...

### Contatos
[Filipe Firmino Lemos](mailto:filipefirmino@gec.inatel.br)

[Filipe Mazzon Ribeiro](mailto:filipemazzon@gec.inatel.br)

[Matheus Henrique da Silva](mailto:matheushenriquesilva@gec.inatel.br)

### Licença

[MIT](https://github.com/FilipeMazzon/lpcCode/blob/master/LICENSE)
