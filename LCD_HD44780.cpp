//Работа с LCD-дисплеем на контроллере HD44780, через диспетчер задач
//все тайминги указаны в предположении, что диспетчер задач вызывается каждую 1 мс

#define lcdport PORTD

//RS=P2 - сигнал управления ЖКИ
//E=P3  - сигнал управления ЖКИ
//D4=P4 - линия данных 4
//D5=P5 - линия данных 5
//D6=P6 - линия данных 6
//D7=P7 - линия данных 7

void lcd_com();
void lcd_com1();
void lcd_com2();
void lcd_com3();

void lcd_dat();
void lcd_dat1();
void lcd_dat2();
void lcd_dat3();

void lcd_init();
void lcd_init1();
void lcd_init2();
void lcd_init3();
void lcd_init4();
void lcd_init5();
void lcd_init6();
void lcd_init7();

volatile unsigned char lcd;	//для обмена данными с lcd


//Программа передачи команд в ЖКИ
void lcd_com() //выполняется за 8 мс
{
	
	lcdport=(lcd&0b11110000)|(lcdport&0b00000011);
	lcdport|=1<<3; //E
	RTOS_SetTask(lcd_com1,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_com1()
{
	lcdport&=~(1<<3); //E
	RTOS_SetTask(lcd_com2,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_com2()
{
	lcdport=((lcd&0b00001111)<<4)|1<<3|(lcdport&0b00000011);
	RTOS_SetTask(lcd_com3,1, 0);		//следующую функцию вызвать через 1 мс
}

void lcd_com3()
{
	lcdport&=~(1<<3); //E
}

//Программа записи данных в ЖКИ
void lcd_dat() //выполняется за 8 мс
{
	lcdport=(lcd&0b11110000)|1<<3|1<<2|(lcdport&0b00000011);
	RTOS_SetTask(lcd_dat1,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_dat1()
{
	lcdport&=~(1<<3);
	lcdport&=~(1<<2);
	RTOS_SetTask(lcd_dat2,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_dat2()
{
	lcdport=((lcd&0b00001111)<<4)|1<<3|1<<2|(lcdport&0b00000011);
	RTOS_SetTask(lcd_dat3,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_dat3()
{
	lcdport&=~(1<<3);
	lcdport&=~(1<<2);
}


//Программа инициализации ЖКИ
void lcd_init() //выполняется за 30 мс
{
//	lcdport=0x00;
	lcdport=(0b00111000)|(lcdport&0b00000011);
	//7654 E RS X X
	RTOS_SetTask(lcd_init1,1, 0);  //следующую функцию вызвать через 1 мс
}

void lcd_init1()
{
	lcdport&=~(1<<3);
	RTOS_SetTask(lcd_init2,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_init2()
{
	lcdport=(0b00111000)|(lcdport&0b00000011);
	RTOS_SetTask(lcd_init3,1, 0); //следующую функцию вызвать через 1 мс
}

void lcd_init3()
{
	lcdport&=~(1<<3);
	RTOS_SetTask(lcd_init4,1, 0); //следующую функцию вызвать через 1 мс
}


void lcd_init4()
{
	lcdport=(0b00101000)|(lcdport&0b00000011);
	RTOS_SetTask(lcd_init5,1, 0);//следующую функцию вызвать через 1 мс
}

void lcd_init5()
{
	lcdport&=~(1<<3);
	RTOS_SetTask(lcd_init6,1, 0);//следующую функцию вызвать через 1 мс
}

void lcd_init6()
{
	lcd=0b00101000; //001 DL N F Х Х Установка интерфейса(DL), число строк(N) и шрифт символов(F) 
	//DL  = 1:   8 bit   DL = 0: 4 bits
	//N   = 1:   2 lines   N = 0: 1 line
	//F   = 1:   5х10   F = 0: 5х8
	lcd_com();						//выполняется за 4 мс
	RTOS_SetTask(lcd_init7,8, 0);   //следующую функцию вызвать через 8 мс
}

void lcd_init7()
{
	lcd=0b00001100;		//	00001DCB	Вкл/выкл. дисплей(D), курсор(C) и его мерцание(B)
	lcd_com();						//выполняется за 8 мс
}

void lcd_clr()		// Очистить дисплей, выполняется за 8 мс
{
	lcd=1;
	lcd_com();		
}

void lcd_gotoxy(unsigned char addr)	// Переместить курсор в позицию addr, выполняется за 8 мс
{
	lcd=(1<<7 | addr);
	lcd_com();
}


#define LCD_BUFFER 100  //максимальное количество символов в буфере
volatile char lcd_buffer0[LCD_BUFFER];
unsigned char lcd_wr_index0=0,lcd_rd_index0=0,lcd_counter0=0;

void lcd_update() //функция обновляет изображение на экране, если в буфере что-то есть. Должна вызываться из диспетчера каждые 8 мс
{
	static bool is_Addr=1; //первый символ это адрес
	
	if (lcd_counter0) { //если в буфере что-то есть
		lcd_counter0--; //уменьшить указатель количества знаков буфере
		lcd=lcd_buffer0[lcd_rd_index0];  //взять символ из буфера
		
		if (is_Addr) { //значит мы считали адрес
			lcd_gotoxy(lcd); //меняем координату курсора
			is_Addr=0; //следующий символ не адрес
		}
		else if (lcd) { //если не ноль, отобразить на жк экране
			lcd_dat();
		}
		else {	//конец строки
			is_Addr=1; //следующий символ это адрес
		} 
		
		lcd_rd_index0++;  //увеличить указатель на последний отображенный символ
		if (lcd_rd_index0==LCD_BUFFER) lcd_rd_index0=0;
	}
}

void lcd_writeline(char *string) //отправить строку через буфер (на прерываниях), первый символ - адрес, затем строка
{
	unsigned char sym=0;     //указатель на текущий символ
	unsigned char tmp=0;
	
	do {     
		if (lcd_counter0 == LCD_BUFFER) return; //если буфер переполнен, то выход !!!ДОРАБОТАТЬ ПОЗЖЕ, чтобы возвращался через некоторое время и доделывал
		tmp=string[sym];
		lcd_buffer0[lcd_wr_index0]=tmp; //записать в буфер текущий символ
		
		lcd_wr_index0++; //увеличить указатель на последний отправленный в буфер символ
		if (lcd_wr_index0==LCD_BUFFER) lcd_wr_index0=0;
		lcd_counter0++;//увеличить указатель количества знаков буфере
		
		sym++;                 //обработать следующий символ
	} while(!(sym-1) || string[sym-1]); //
	
}

void lcd_writeline_utf16(char *string) //отправить строку через буфер (на прерываниях), первый символ - адрес, затем строка
{
	unsigned char sym=0;     //указатель на текущий символ
	unsigned int tmp=0;
	
	do {     
		if (lcd_counter0 == LCD_BUFFER) return; //если буфер переполнен, то выход !!!ДОРАБОТАТЬ ПОЗЖЕ, чтобы возвращался через некоторое время и доделывал
		tmp=string[sym];
		//преобразуем кодировку UTF8 в кодировку жк-дисплея
		if (string[sym]==0xD0) { //преобразуем только русские буквы
			sym++;
			tmp=string[sym];
			switch(tmp) {
				case 0x81: tmp=162; break; //Ё
				case 0x90: tmp=65; break; //А
				case 0x91: tmp=160; break; //Б
				case 0x92: tmp=66; break; //В
				case 0x93: tmp=161; break; //Г
				case 0x94: tmp=224; break; //Д
				case 0x95: tmp=69; break; //Е
				case 0x96: tmp=163; break; //Ж
				case 0x97: tmp=164; break; //З
				case 0x98: tmp=165; break; //И
				case 0x99: tmp=166; break; //Й
				case 0x9A: tmp=75; break; //К
				case 0x9B: tmp=167; break; //Л
				case 0x9C: tmp=77; break; //М
				case 0x9D: tmp=72; break; //Н
				case 0x9E: tmp=79; break; //О
				case 0x9F: tmp=168; break; //П
				case 0xA0: tmp=80; break; //Р
				case 0xA1: tmp=67; break; //С
				case 0xA2: tmp=84; break; //Т
				case 0xA3: tmp=169; break; //У
				case 0xA4: tmp=170; break; //Ф
				case 0xA5: tmp=88; break; //Х
				case 0xA6: tmp=225; break; //Ц
				case 0xA7: tmp=171; break; //Ч
				case 0xA8: tmp=172; break; //Ш
				case 0xA9: tmp=226; break; //Щ
				case 0xAA: tmp=173; break; //Ъ
				case 0xAB: tmp=174; break; //Ы
				case 0xAC: tmp=98; break; //Ь
				case 0xAD: tmp=175; break; //Э
				case 0xAE: tmp=176; break; //Ю
				case 0xAF: tmp=177; break; //Я
				case 0xB0: tmp=97; break; //а
				case 0xB1: tmp=178; break; //б
				case 0xB2: tmp=179; break; //в
				case 0xB3: tmp=180; break; //г
				case 0xB4: tmp=227; break; //д
				case 0xB5: tmp=101; break; //е
				case 0xB6: tmp=182; break; //ж
				case 0xB7: tmp=183; break; //з
				case 0xB8: tmp=184; break; //и
				case 0xB9: tmp=185; break; //й
				case 0xBA: tmp=186; break; //к
				case 0xBB: tmp=187; break; //л
				case 0xBC: tmp=188; break; //м
				case 0xBD: tmp=189; break; //н
				case 0xBE: tmp=111; break; //о
				case 0xBF: tmp=190; break; //п
			}
		}
		if (string[sym]==0xD1) { //преобразуем только русские буквы
			sym++;
			tmp=string[sym];
			switch(tmp) {
				case 0x91: tmp=181; break; //ё
				case 0x80: tmp=112; break; //р
				case 0x81: tmp=99; break; //с
				case 0x82: tmp=191; break; //т
				case 0x83: tmp=121; break; //у
				case 0x84: tmp=228; break; //ф
				case 0x85: tmp=213; break; //х
				case 0x86: tmp=229; break; //ц
				case 0x87: tmp=192; break; //ч
				case 0x88: tmp=193; break; //ш
				case 0x89: tmp=230; break; //щ
				case 0x8A: tmp=194; break; //ъ
				case 0x8B: tmp=195; break; //ы
				case 0x8C: tmp=196; break; //ь
				case 0x8D: tmp=197; break; //э
				case 0x8E: tmp=198; break; //ю
				case 0x8F: tmp=199; break; //я
			}
		}
		
		lcd_buffer0[lcd_wr_index0]=tmp; //записать в буфер текущий символ
		
		lcd_wr_index0++; //увеличить указатель на последний отправленный в буфер символ
		if (lcd_wr_index0==LCD_BUFFER) lcd_wr_index0=0;
		lcd_counter0++;//увеличить указатель количества знаков буфере
		
		sym++;                 //обработать следующий символ
	} while(!(sym-1) || string[sym-1]); //
	
}
