/*
 * atmega328uno.cpp
 *
 * Created: 29.07.2016 10:32:40
 * Author : МатюковЕ
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitwise_operation.cpp" //функции работы с отдельными битами
#include "uart_scrollbox.cpp" //работа с uart
#include "rtos.cpp" //диспетчер задач
#include "MCU_Init.cpp" //инициализация микроконтроллера
#include "scrollboxcontroller.cpp" //Работа со скроллбоксом



//кнопки подключены к этим портам
#define key0 PINF,0


//таймер0 вызывается каждую 1 мс
ISR (TIMER0_COMP_vect)
{
	TCNT0=0x00; //сброс таймера 0
	RTOS_Timer();
	
	
	update_scrollbox(); //для работы скроллбокса
	
}


void settx()
{
	clrout(&TXEN);//   TXEN=0; //даём команду max485 перейти в режим приёма
    writelineen=0;  //предыдущая строка отправилась? разрешаем прием
}

int main()
{
	
	MCU_Init(); //инициализация микроконтроллера

	// Global enable interrupts
	asm("sei");

	RTOS_Init(); //инициализация диспетчера задач

	// Добавление задач (имя функции, до первого запуска, цикличность запуска)
//	RTOS_SetTask(lcd_init,100, 0); //инициализация LCD, выполняется за 30 мс, пауза перед запуском 100 мс

	//RTOS_SetTask(lcd_clr, 112, 0); //8ms
	
	//RTOS_SetTask(printat, 130, 0); //8мс
//	RTOS_SetTask(lcd_update, 138, 8); //отрисовывать из буфера, 8мс
//	RTOS_SetTask(keyboard_update, 250, 25); //клавиатура
//	RTOS_SetTask(lightroad_update, 250, 250); //световая дороожка
//	RTOS_SetTask(led10_update, 250, 500); //светодиод Led10
	
	

//	RTOS_SetTask(print1, 140, 0); //записать строку в буфер
//	RTOS_SetTask(print2, 150, 0); //записать строку в буфер
//	RTOS_SetTask(print2, 0, 500); //записать строку в буфер


	
//	RTOS_SetTask(led0inv, 0, 0);
//	RTOS_SetTask(led1inv, 0, 100);
	//RTOS_SetTask(lcd_clr, 5316, 5000);

	//считываем настройки из eeprom
//	MainAddr=eeMainAddr; //адрес этого устройства (0,4,8,12,16..252)
//	tUP=eetUP; //если состояние меняется с нуля на единицу, счетчик должен зарегистрировать более 95 раз состояние лог.единицы за 100 мс, каждую 1 мс
//	tDN=eetDN; //если состояние меняется с единицы на нуль, счетчик должен зарегистрировать более 10 раз состояние лог.единицы за 100 мс, каждую 1 мс
//	TimeOut=eeTimeOut; //время ожидания опускания или поднятия рольставни

//	TXEN=0; //даём команду max485 перейти в режим приёма


//	char hello[]="Scrollbox/r";
//	writeline(hello); //отправить приветствие

	//PORTD=0xFF;    //симуляция концевых выключателей

	


    while (1) 
    {
		RTOS_DispatchTask();  //Диспетчер РТОС
		
		
	    if (readlineen) readline();  //если установлен флаг принятой строки, то обработать эту строку
	    if (!(tx_counter0 || ((UCSR0A & DATA_REGISTER_EMPTY)==0))) {
	//	    pause(WAIT_TX_END_SYMBOL);  //задержка на отправку последнего символа 1/(скорость_передачи)*10(бит) при скорость 1200 бод 8.33 мс, при 19200 - 0.53 мс
			RTOS_SetTask(settx, 1, 0);
		}
	
	
	}

		
	
}



