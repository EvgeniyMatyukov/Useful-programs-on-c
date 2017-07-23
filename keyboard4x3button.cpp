//программа для работы с клавиатурой 4x4
#define keyboardport PORTA
#define keyboardportin PINA

volatile unsigned int key=0xFFFF; //актуальное состояние кнопок
void keyboard_update() //функция проверяет нажали или отпустили клавишу. Должна вызываться из диспетчера каждые 25 мс
{
	static unsigned char row=1;
	static unsigned int keyold=0xFFFF; //предыдущее состояние кнопок
	static unsigned int keyrepeat=0x00; //флаг повторного нажатия кнопки
	static bool fire_mode=0;  //режим пожара включен
	static bool safe_mode=0;  //режим охраны включен
	
	switch (row) {
		case 1:
			keyboardport|=1<<0;
			keyboardport&=~(1<<2);
			
			key=(key&0b1111111111110000)|(keyboardportin>>4);
			
			break;
			
		case 2:
			keyboardport|=1<<1;
			keyboardport&=~(1<<0);
			key=(key&0b1111111100001111)|(keyboardportin&0b11110000);
			break;
		
		case 3:
			keyboardport|=1<<2;
			keyboardport&=~(1<<1);
			key=(key&0b1111000011111111)|int((keyboardportin&0b11110000)<<4);
			break;

	}
	
	if (key!=keyold) {  //состояние кнопок изменилось
		
		if (!fire_mode && !safe_mode) {   //не реагировать если включен режим охраны или режим пожара
			if (!bit(key,0)&&bit(keyold,0)) { //нажали первую кнопку "Дымовой извещатель"
				if (!bit(keyrepeat,0)) { //кнопка нажата впервые
					setbit(detector,0);
					smoke_detector_on();
					setbit(keyrepeat,0);
				}
				else { //кнопка нажата второй раз
					clrbit(detector,0);
					smoke_detector_off();
					clrbit(keyrepeat,0);
				}
			}
			if (!bit(key,1)&&bit(keyold,1)) { //нажали вторую кнопку "Тепловой извещатель"
				if (!bit(keyrepeat,1)) { //кнопка нажата впервые
					setbit(detector,1);
					heat_detector_on();
					setbit(keyrepeat,1);
				}
				else { //кнопка нажата второй раз
					clrbit(detector,1);
					heat_detector_off();
					clrbit(keyrepeat,1);
				}
			}
			if (!bit(key,2)&&bit(keyold,2)) { //нажали третью кнопку "Извещатель комбинированный"
				if (!bit(keyrepeat,2)) { //кнопка нажата впервые
					setbit(detector,2);
					combined_detector_on();
					setbit(keyrepeat,2);
				}
				else { //кнопка нажата второй раз
					clrbit(detector,2);
					combined_detector_off();
					clrbit(keyrepeat,2);
				}
			}
			if (!bit(key,3)&&bit(keyold,3)) { //нажали 4 кнопку "Извещатель пламени"
				if (!bit(keyrepeat,3)) { //кнопка нажата впервые
					setbit(detector,3);
					flame_detector_on();
					setbit(keyrepeat,3);
				}
				else { //кнопка нажата второй раз
					clrbit(detector,3);
					flame_detector_off();
					clrbit(keyrepeat,3);
				}
			}
			if (!bit(key,4)&&bit(keyold,4)) { //нажали 5 кнопку "Извещатель газовый"
				if (!bit(keyrepeat,4)) { //кнопка нажата впервые
					setbit(detector,4);
					gas_detector_on();
					setbit(keyrepeat,4);
				}
				else { //кнопка нажата второй раз
					clrbit(detector,4);
					gas_detector_off();
					clrbit(keyrepeat,4);
				}
			}
			if (!bit(key,5)&&bit(keyold,5)) { //нажали 6 кнопку "Световое табло ВЫХОД"
				if (!bit(keyrepeat,5)) { //кнопка нажата впервые
					display_exit_on();
					setbit(keyrepeat,5);
				}
				else { //кнопка нажата второй раз
					display_exit_off();
					clrbit(keyrepeat,5);
				}
			}

		}
		if (!bit(key,6)&&bit(keyold,6)) { //нажали 7 кнопку "Ручной извещатель 1"
			if (!fire_mode) { //если нет режима пожара, то выключить газовый детектор
				clrout(&led5);
				clrbit(detector,4);
			}
			fire_mode=1; //режим пожара
			manual_detector1_on();
		}
		if (!bit(key,7)&&bit(keyold,7)) { //нажали 8 кнопку "Ручной извещатель 2"
			if (!fire_mode) { //если нет режима пожара, то выключить газовый детектор
				clrout(&led5);
				clrbit(detector,4);
			}
			fire_mode=1; //режим пожара
			manual_detector2_on();
		}
		if (!fire_mode && !bit(key,8) && bit(keyold,8)) { //нажали 9 кнопку "Установка на охрану", если не активен режим пожара
			if (!detector) set_protection_unable(); //если ни один детектор не выбран
			else {
				if (!bit(keyrepeat,8)) { //кнопка нажата впервые
					safe_mode=1;
					set_protection_on();
					setbit(keyrepeat,8);
				}
				else { //кнопка нажата второй раз
					safe_mode=0;
					set_protection_off();
					clrbit(keyrepeat,8);
				}
			}
		}
		if (!fire_mode && !bit(key,9) && bit(keyold,9)) { //если не включен режим пожара и нажали 10 кнопку "Пожар"
			if (!safe_mode) fire_unable(); //если режим защиты выключен
			else {
				fire_mode=1; //режим пожара
				fire_on();
			}
		}
		if (!bit(key,10)&&bit(keyold,10)) { //нажали 11 кнопку "Сброс"
			detector=0; //сбросить состояние датчиков
			keyrepeat=0x00; //сбросить состояние повторного нажатия
			fire_mode=0; //сбросить режим пожара
			safe_mode=0; //сбросить режим охраны
			reset_on();
		}
		
		keyold=key;  //запомним новое состояние
	}
	
	row++;
	if (row==4) row=1;
}