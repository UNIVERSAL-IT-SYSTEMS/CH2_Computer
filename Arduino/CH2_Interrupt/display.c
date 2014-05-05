/**
*
* @file display.c
*
* @brief display driver
*
* @author ChrisMicro
* @copyright (c) 2014 ChrisMicro
*
* The CH2-Computer can be used as a shield for Ardduinos with an Atmega168 or Atmega328
* microcontroller ( mainly Arduino Uno ).
*
*/
#include "display.h"
#include <util/delay.h>

inline void ledOn()
{
  DDRC|=1;

  PORTC|=1;
}

inline void ledOff()
{
  PORTC&=~1;
}

void initDisplay()
{
	  // set row lines as output
	  DDRD|=(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3);
	  // set col lines as output
	  DDRB|=(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<0);
	  DDRD|=(1<<2);
	  DDRC|=(1<<3);
}
void displayOff()
{
	  // set row lines as output
	  DDRD&=~((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3));
	  // set col lines as output
	  DDRB&=~((1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<0));
	  DDRD&=~((1<<2));
	  DDRC&=~(1<<3);
}

uint8_t DisplayMem[NUMCOLUMNS];

/**
* @brief Set one column line of the 5x7 matrix active
*
* @param uint8_t col: 0..6
*
* example:
* setCol(2) turns the second column line ON
* to light one pixel there has to be also a active row
* e.g. with setRow(0).
*/
// col 0..6 , anodes
inline void setCol(uint8_t col)
{
	// the columns are the anodes of the 5x7 matrix
	PORTD&=~(1<<2);
	PORTC&=~(1<<3);
	PORTB&=~(1<<5);
	PORTB&=~(1<<4);
	PORTB&=~(1<<3);
	PORTB&=~(1<<2);
	PORTB&=~(1<<0);

	// binary tree for faster decision
	if(col>2)
	{
	  if(col>4)
	  {
		  if(col==6)PORTD|=(1<<2);
		  else PORTC|=(1<<3); // 5
	  }else
	  {
		  if(col==4) PORTB|=(1<<5);
		  else PORTB|=(1<<4);// 3
	  }
	}else
	{
	  if(col==2)PORTB|=(1<<3);
	  else if(col==1)PORTB|=(1<<2);
	  else PORTB|=(1<<0);
	}
}
#define ROWSOFF() {  PORTD|=(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3);} // clear all row pixel
/**
* @brief Set one row line of the 5x7 matrix active
*
* @param uint8_t col: 0..4
*
*/
// row 0..4, cathodes
void setRow(uint8_t row)
{
  // the rows are cathodes of the matrix and therefore active low
  ROWSOFF();

  PORTD&=~(1<<(3+row));
}

/**
* @brief Set multiple row lines, used to switch on multiple LEds
*
* @param uint8_t col: bits to be set, range: b00000..b11111
*
*/
inline void setRowPattern(uint8_t bitPattern)
{
  PORTD|=0b11111000; // turn all leds off
  PORTD&=~(bitPattern<<3);
}

inline void _showMatrix()
{
  static uint8_t column=0;

  ROWSOFF(); // avoid smearing
  setCol(column);
  setRowPattern(DisplayMem[column]);

  column++;
  if(column>=NUMCOLUMNS)column=0;

}
/***************************************************************************

    show the 5x7 matrix for ms (milli seconds )

***************************************************************************/
void showMatrix(uint16_t ms)
{
  uint16_t n;
  initDisplay();
  for(n=0;n<ms;n++)
  {
    _showMatrix();
    _delay_ms(1);
  }
  displayOff();
}

void _printChar(uint16_t c)
{
	uint8_t n,temp;

	temp=0;
	for(n=0;n<5;n++)
	{
	  DisplayMem[n]=smallbitmap[c][n];

	}
}
/***************************************************************************

    void setPixel(uint8_t x, uint8_t yy, uint8_t intensity)

    set pixel on screen with desired intensity

    intensity: 0,1

***************************************************************************/
void setPixel(uint8_t x, uint8_t y, uint8_t intensity)
{
  if((x<NUMCOLUMNS)&&(y<NUMROWS))
  {
    if(intensity>0)
    {
      DisplayMem[NUMCOLUMNS-1-x]|=1<<y;
    }else DisplayMem[NUMCOLUMNS-1-x]&=~(1<<y);
  }
}
// clear screen
void cls()
{
  uint8_t n;
  for(n=0;n<NUMCOLUMNS;n++)
  {
    DisplayMem[n]=0;
  }

}
#define SHIFTLEFT 4
void printCode(uint16_t c)
{
  uint8_t n,k,temp;
  cls();
  for(k=0;k<3;k++)
  {
    temp=0;
    for(n=0;n<NUMROWS;n++)
    {
      if((smallbitmap[c][n]<<k)&0x80)setPixel(k+SHIFTLEFT,NUMROWS-1-n,1);
    }
  }
}
/***************************************************************************

  print one hex digit

  input: uint8_t 0..F

***************************************************************************/
void hex1(uint8_t x)
{
  if(x<10)printCode(x);
  else printCode(x+3);
}

/***************************************************************************

  print ASCII code to matrix display

***************************************************************************/
void _putchar(int8_t c)
{
  uint8_t k;
  if((c>='0')&&(c<='9'))k=c-'0'; // 0..9
  if((c>='A')&&(c<='Z'))k=c-'A'+13;
  if((c>='a')&&(c<='z'))k=c-'a'+13;
  printCode(k);
}
/***************************************************************************

  Use the leftmost pixels of the 5x7 matrix display as "virtual leds".

  led0  x    x    x    x    x    x
  led1  x    x    x    x    x    x
  led2  x    x    x    x    x    x
  led3  x    x    x    x    x    x
  led4  led5 led6 x    x    x    x

  led7: external ultra bright LED

***************************************************************************/
void showLeds(uint8_t k)
{
  uint8_t n;
  for(n=0;n<5;n++)
  {
    if((k>>n)&1)setPixel(0,4-n,1);
    else setPixel(0,4-n,0);
  }
  if((k>>5)&1)setPixel(1,0,1);
  else setPixel(1,0,0);
  if((k>>6)&1)setPixel(2,0,1);
  else setPixel(2,0,0);
  if((k>>7)&1)ledOn();
  else ledOff();
}
/*
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/

