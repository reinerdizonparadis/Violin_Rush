/*
 *	Violin Rush
 *	CPE 200 Lab Final Project
 *	Authors: Reiner Dizon & Saul Mendoza
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int myrand();
int myrand2();
void write_pixel(int, int, short);
void write_char(int, int, char);
void write_text(int, int, char*);
void clear_screen();
void draw_rectangle(int, int, int, int, short);
void initial(int);
int read_ints (const char*);
void sliding_rectangle(int, int, int);
void control();
void write_score(int, int, int);

int SPEED = 500000;
const short BACK_COLOR = 0x0000; // black
const short BUTTON_COLOR = 0xffff; // white
const short PRESSED_COLOR = 0xf800; // red
const short LINE_DIVIDER_COLOR = 0x8410; // grey

int score = 0;
volatile int * SW_switch_ptr = (int *) 0x10000040; // SW slider switch address

int main(){
	srand(time(NULL));
	int i, x, y, KEY_value, SW_value;
	volatile int * SW_switch_ptr = (int *) 0x10000040; // SW slider switch address
	volatile int * KEY_ptr = (int *) 0x10000050; // pushbutton KEY address
	volatile int * red_LED_ptr = (int *) 0x10000000; // red LED address
	clear_screen();
	initial(15000000);
	while(1){
		SW_value = *(SW_switch_ptr); // read the SW slider switch values
		if(SW_value == 1){
			for(i = 0; i < 4294967296; i++){
				if(myrand() >= 1 && myrand() <= 100){
					sliding_rectangle(10, 106, myrand2());
				}
				else if(myrand() >= 101 && myrand() <= 200){
					sliding_rectangle(116, 206, myrand2());
				}
				else if(myrand() >= 201 && myrand() <= 300){
					sliding_rectangle(216, 306, myrand2());
				}
			}
		}
		else if(SW_value == 0 || SW_value == 2){
			score = 0;
			initial(15000000);
			for (i = 0; i < SPEED * 2; i++){}
		}
	}
	return 0;
}

int myrand(){
	return (1 + rand() % (300 - 1 + 1));
}

int myrand2(){
	return (10 + rand() % (190 - 10 + 1));
}

void write_pixel(int x, int y, short color)
// set a single pixel on the screen at x,y
// x in [0,319], y in [0,239], and color in [0,65535]
{
	volatile short *vga_addr = (volatile short*) (0x08000000 + (y<<10) + (x<<1));
	*vga_addr = color;
}

void write_char(int x, int y, char c)
// write a single character to the character buffer at x,y
// x in [0,79], y in [0,59]
{
	// VGA character buffer
	volatile char * character_buffer = (char *) (0x09000000 + (y<<7) + x);
	*character_buffer = c;
}

void write_text(int x, int y, char* word){
	while (*word) {
		write_char(x, y, *word);
		x++;
		word++;
	}
}

void clear_screen() 
// use write_pixel to set entire screen to black (does not clear the character buffer)
{
	int x, y;
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
		  write_pixel(x,y,0);
		}
	}
}

void draw_rectangle(int x_int, int x_fin, int y_int, int y_fin, short color){
	int x, y;
	for(x = x_int; x < x_fin; x++){
		for(y = y_int; y < y_fin; y++){
			write_pixel(x, y, color);
		}
	}
}

void initial(int delay){
	int i;
	draw_rectangle(0, 320, 0, 240, BACK_COLOR);// background color
	write_text(162, 10, "Project"); // initial text
	draw_rectangle(10, 106, 200, 240, BUTTON_COLOR); // 1st rectangle
	draw_rectangle(116, 206, 200, 240, BUTTON_COLOR); // 2nd rectangle
	draw_rectangle(216, 306, 200, 240, BUTTON_COLOR); // 3rd rectangle
	
	draw_rectangle(0, 7, 0, 240, LINE_DIVIDER_COLOR); // line 0
	draw_rectangle(109, 113, 0, 240, LINE_DIVIDER_COLOR); // line 1
	draw_rectangle(209, 213, 0, 240, LINE_DIVIDER_COLOR); // line 2
	draw_rectangle(314, 320, 0, 240, LINE_DIVIDER_COLOR); // line 3
	
	for(i = 0; i < delay; i++){}	//delay loop
	write_text(162, 10, "            "); // clears the text
}

void sliding_rectangle(int x_int, int x_fin, int start_y){
	int i, j;
	volatile int * KEY_ptr = (int *) 0x10000050; // pushbutton KEY address
	int KEY_value = *(KEY_ptr + 3);
	int SW_value = *(SW_switch_ptr);
	for(i = start_y; i < 240; i++){
		SW_value = *(SW_switch_ptr);
		if(SW_value == 1){
			control();
			draw_rectangle(x_int, x_fin, i, i+15, BUTTON_COLOR); // sliding shape
			if(score % 10 == 0 && score >= 10){
				SPEED = SPEED - 1200;
			}
			for(j = 0; j < SPEED; j++){}	//delay loop
			draw_rectangle(x_int, x_fin, i, i+15, BACK_COLOR);
			KEY_value = *(KEY_ptr + 3);
			if(i >= 190 && i <= 240){
				if((x_int == 10 && (KEY_value & 0x8)) || (x_int == 116 && (KEY_value & 0x4)) || (x_int == 216 && (KEY_value & 0x2))){
					score++;
					write_score(162, 10, score);
				}
			}
		}
		else if(SW_value == 3){
			while(1){
				SW_value = *(SW_switch_ptr);
				draw_rectangle(x_int, x_fin, i, i+15, BUTTON_COLOR); // sliding shape
				write_text(162, 10, "Paused"); // initial text
				if(SW_value == 1){ 
					draw_rectangle(x_int, x_fin, i, i+15, BACK_COLOR);
					write_text(162, 10, "            "); // clears the text
					break;
				}
			}
		}
		else if(SW_value == 0 || SW_value == 2){
			score = 0;
			SPEED = 500000;
			write_text(162, 10, "            "); // clears the text
		}
		
	}
}

void control(){
	volatile int * KEY_ptr = (int *) 0x10000050; // pushbutton KEY address
	int KEY_value;
	draw_rectangle(10, 106, 200, 240, BUTTON_COLOR); // 1st rectangle
	draw_rectangle(116, 206, 200, 240, BUTTON_COLOR); // 2nd rectangle
	draw_rectangle(216, 306, 200, 240, BUTTON_COLOR); // 3rd rectangle
	draw_rectangle(0, 7, 0, 240, LINE_DIVIDER_COLOR); // line 0
	draw_rectangle(109, 113, 0, 240, LINE_DIVIDER_COLOR); // line 1
	draw_rectangle(209, 213, 0, 240, LINE_DIVIDER_COLOR); // line 2
	draw_rectangle(314, 320, 0, 240, LINE_DIVIDER_COLOR); // line 3
	
	KEY_value = *(KEY_ptr + 3); // read the pushbutton KEY values
	
	if(KEY_value & 0x2){ //Key 1
		draw_rectangle(216, 306, 200, 240, PRESSED_COLOR);
		*(KEY_ptr + 3) = 0;
	}
	
	if(KEY_value & 0x4){ //Key 2
		draw_rectangle(116, 206, 200, 240, PRESSED_COLOR);
		*(KEY_ptr + 3) = 0;
	}
	
	if(KEY_value & 0x8){ //Key 3
		draw_rectangle(10, 106, 200, 240, PRESSED_COLOR);
		*(KEY_ptr + 3) = 0;
	}
}

void write_score(int x, int y, int score){
	char *sc, text[23];
	sprintf(text, "%d", score);
	sc = text;
	while (*sc) {
		write_char(x, y, *sc);
		x++;
		sc++;
	}
}
