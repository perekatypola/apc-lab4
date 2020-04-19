																																																													#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define BASE_FREQUENCY 1193180L
#define NUMBER_OF_NOTES 19

void playSound();
void showStatesWords();
void showDivisionsRatios();
void getRandomNumber();

void main() {
	clrscr();
	while (1) {
		printf("1 - Play sound;\n"
			"2 - Show states words;\n"
			"3 - Show divisions ratios;\n"
			"4 - Get random number;\n"
			"0 - Exit.\n");
		switch (getch()) {
		case '1':
			playSound();
			clrscr();
			break;
		case '2':
			clrscr();
			showStatesWords();
			break;
		case '3':
			clrscr();
			showDivisionsRatios();
			break;
		case '4':
			clrscr();
			getRandomNumber();
			break;
		case '0':
			clrscr();
			return;
		default:
			clrscr();
		}
	}
}


void playSound() {
	
	//The show must go on 8-bit
	int frequency[NUMBER_OF_NOTES] = { 262, 0,262 ,0, 247,0, 262 , 0,262 , 0, 262, 0, 247 ,0, 247 ,0,220, 0, 220 };
	int duration[NUMBER_OF_NOTES] = { 200, 100, 200, 100,200, 100 ,300, 400 ,200, 100, 200,100, 200, 100, 300, 200  , 200 , 100, 300 };
	int divisionRatio;

	for (int i = 0; i < NUMBER_OF_NOTES; i++) {
		if (frequency[i] == 0) {
			delay(duration[i]);
			continue;
		}
		
	outp(0x61, inp(0x61) | 3);  //«вук от таймера теперь будет проходить на динамик (младшие два бита установлены в  единицу без сброса остальных)
	outp(0x43, 0xB6); // устанавливаем 1011 0110 в управл€ющий канал(чтение-запись младшего, затем старшего байта, второй канал,3 режим)

	divisionRatio = BASE_FREQUENCY / frequency[i];
	outp(0x42, divisionRatio % 256); //сначала младший байт в регистр счетчика таймера
	divisionRatio /= 256;
	outp(0x42, divisionRatio); //затем старший
	delay(duration[i]); // вызываем соотвествующую задержку
	outp(0x61, inp(0x61) & 0xFC); //выключение динамика (1111 1100)(младшие биты установлены в 0, остальные не потер€ны)
	}
	delay(50);
}

void showStatesWords() {
	clrscr();
	unsigned char state, stateWord[9];
	int ports[] = { 0x40, 0x41, 0x42 };
	int controlWords[] = { 0xE2, 0xE4, 0xE8 };

	//1110 0010 0 - always 0, 001 - first port, 0 - read state,
		//1 - don't fix counter, 11 - read command
	//1110 0100
	//1110 1000

	stateWord[8] = '\0';
	for (int iChannel = 0; iChannel < 3; iChannel++) {
		outp(0x43, controlWords[iChannel]); //select a channel
		state = inp(ports[iChannel]);
		for (int i = 7; i >= 0; i--) {
			stateWord[i] = state % 2 + '0';
			state /= 2;
		}
		printf("Channel %d: %s \n", iChannel, stateWord);
	}
}

void showDivisionsRatios() {
	int ports[] = { 0x40, 0x41, 0x42 };
	int controlWords[] = { 0x00, 0x40, 0x80 };
	//0000 0000 //1-3 - ignor, 4-5 - block command(need 0), 6-7 - port
	//0100 0000
	//1000 0000
	unsigned  divisionRatio, maxDivisionRatio, divisionRatioHigh, divisionRatioLow;
	printf("Division ratio:\n");

	for (int iChannel = 0; iChannel < 3; iChannel++) {
		divisionRatio = 0;
		maxDivisionRatio = 0;

		if (iChannel == 2) {
			outp(0x61, inp(0x61) | 1); //turn ON for kd2
		}

		for (unsigned long j = 0; j < 1000000; j++) {
			outp(0x43, controlWords[iChannel]); //select a channel
			divisionRatioLow = inp(ports[iChannel]);
			divisionRatioHigh = inp(ports[iChannel]);
			divisionRatio = divisionRatioHigh * 256 + divisionRatioLow;
			if (divisionRatio > maxDivisionRatio) {
				maxDivisionRatio = divisionRatio;
			}
		}

		if (iChannel == 2) {
			outp(0x61, inp(0x61) & 0xFC); ////turn OFF for kd2
		}

		printf("Channel %d - %u\n", iChannel, maxDivisionRatio);
	}
}



void getRandomNumber() {
	unsigned short int limit; 
	unsigned int number, numberLow, numberHigh;
	clrscr();
	
	outp(0x61, inp(0x61) | 1);  //включение без динамика
	do {
		printf("1 - Set random limit;\n"
			"2 - Get a number;\n"
			"0 - Exit.\n");
		fflush(stdin);
		switch (getch()) {
		case '1':
				do
				{
					printf("Enter number limit: from 1 to 65535\n");
					fflush(stdin);
				} while (!scanf("%u", &limit));

			outp(0x43, 0xB6); // устанавливаем 1011 0110 в управл€ющий канал(чтение-запись младшего, затем старшего байта, второй канал,3 режим)
			outp(0x42, limit % 256); //устанавливаем младший байт счетчика таймера
			limit /= 256;
			outp(0x42, limit); //старший бит
			break;
		case '2':
			printf("Random number:\n");
			outp(0x43, 0x86); //выставл€ем режим CLC дл€ второго канала(дл€ фиксировани€ текущего значени€) 3 режим 
			//1000 0110 (2 канал, 3 режим, 67 байты указывают в нуле, указывают на фиксацию текущего значени€)
			numberLow = inp(0x42);
			numberHigh = inp(0x42);
			number = numberHigh * 256 + numberLow;
			printf("Number %u\n", number);
			break;

		case '0':
			outp(0x61, inp(0x61) & 0xFC); //turn OFF
			clrscr();
			return;
		}
	} while (1);
}