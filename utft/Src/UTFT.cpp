/*
  UTFT.cpp - Multi-Platform library support for Color TFT LCD Boards
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino, chipKit boards and select 
  TI LaunchPads. For a full list of tested display modules and controllers,
  see the document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can find the latest version of the library at 
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/

#include "itoa.h"
#include "dtostrf.h"
#include <pgmspace.h>
#include <UTFT.h>

// Include hardware-specific functions

#include "main.h"
#include "stm32f1xx_hal.h"

#include "WString.h"
#include "wiring_constants.h"
#include "memorysaver.h"

UTFT::UTFT()
{
}

UTFT::UTFT(int RS, int WR, int CS, int RST, int RD)
{ 

	disp_x_size =			239;
	disp_y_size =			319;

}

void UTFT::LCD_Write_COM(char VL)  
{   
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); //clear(P_RS, B_RS);
		LCD_Writ_Bus(0x00,VL);

}

void UTFT::LCD_Write_DATA(char VH,char VL)
{
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); //set(P_RS, B_RS);
		LCD_Writ_Bus(VH,VL);

}

void UTFT::LCD_Write_DATA(char VL)
{
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); //set(P_RS, B_RS);
		LCD_Writ_Bus(0x00,VL);
}

void UTFT::LCD_Write_COM_DATA(char com1,int dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA(dat1>>8,dat1);
}

void UTFT::InitLCD(byte orientation)
{
	orient=orientation;

	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET); //set(P_RST, B_RST);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET); //clear(P_RST, B_RST);
	HAL_Delay(15);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET); //set(P_RST, B_RST);
	HAL_Delay(15);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET); // set(P_RD, B_RD);

//***** display settings ****************************************************************

    LCD_Write_COM(0x01);	//Software Reset
    HAL_Delay(200);

    LCD_Write_COM(0x28);    // Display off

    LCD_Write_COM(0x3A);    // Pixel Format
    LCD_Write_DATA(0x55,0x00);   // 565, 16-bit

    LCD_Write_COM(0xBB);    // VCOMS
    LCD_Write_DATA(0x2B,0x00);   //

    LCD_Write_COM(0xC3);    // VRHS
    LCD_Write_DATA(0x11,0x00);   //

    LCD_Write_COM(0xD0);    // PWCTRL
    LCD_Write_DATA(0xA4,0xA1);   //

    LCD_Write_COM(0x11);    //Exit Sleep
    HAL_Delay(120);

    LCD_Write_COM(0x29);    //Display on

    LCD_Write_COM(0x2c); 	//Memory Write

// *************************************************************************************************

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set (P_CS, B_CS);

	setColor(255, 255, 255);
	setBackColor(0, 0, 0);
	cfont.font=0;
	_transparent = false;
}

void UTFT::setXY(word x1, word y1, word x2, word y2)
{
	if (orient==LANDSCAPE)
	{
		swap(word, x1, y1);
		swap(word, x2, y2)
		y1=disp_y_size-y1;
		y2=disp_y_size-y2;
		swap(word, y1, y2)
	}

//***** display settings *******************************************************

	LCD_Write_COM(0x2a);
	LCD_Write_DATA(x1>>8,x1);
	LCD_Write_DATA(x2>>8,x2);
	LCD_Write_COM(0x2b);
	LCD_Write_DATA(y1>>8,y1);
	LCD_Write_DATA(y2>>8,y2);
	LCD_Write_COM(0x2c);

//*************************************************************************************
}

void UTFT::clrXY()
{
	if (orient==PORTRAIT)
		setXY(0,0,disp_x_size,disp_y_size);
	else
		setXY(0,0,disp_y_size,disp_x_size);
}

void UTFT::drawRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}

void UTFT::drawRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		drawPixel(x1+1,y1+1);
		drawPixel(x2-1,y1+1);
		drawPixel(x1+1,y2-1);
		drawPixel(x2-1,y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}

void UTFT::fillRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if(fch==fcl)
		{
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
		setXY(x1, y1, x2, y2);
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // 	set(P_RS, B_RS);
		_fast_fill_8(fch,((long(x2-x1)+1)*(long(y2-y1)+1)));
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // 	set(P_CS, B_CS)
		}
	else
		{
			if (orient==PORTRAIT)
			{
				for (int i=0; i<((y2-y1)/2)+1; i++)
				{
					drawHLine(x1, y1+i, x2-x1);
					drawHLine(x1, y2-i, x2-x1);
				}
			}
			else
			{
				for (int i=0; i<((x2-x1)/2)+1; i++)
				{
					drawVLine(x1+i, y1, y2-y1);
					drawVLine(x2-i, y1, y2-y1);
				}
			}
		};

}

void UTFT::fillRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				drawHLine(x1+2, y1+i, x2-x1-4);
				drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				drawHLine(x1+1, y1+i, x2-x1-2);
				drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void UTFT::drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA(fch,fcl);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA(fch,fcl);
	setXY(x + radius, y, x + radius, y);
	LCD_Write_DATA(fch,fcl);
	setXY(x - radius, y, x - radius, y);
	LCD_Write_DATA(fch,fcl);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_Write_DATA(fch,fcl);
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::fillCircle(int x, int y, int radius)
{
	for(int y1=-radius; y1<=0; y1++) 
		for(int x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius) 
			{
				drawHLine(x+x1, y+y1, 2*(-x1));
				drawHLine(x+x1, y-y1, 2*(-x1));
				break;
			}
}

void UTFT::clrScr()
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	clrXY();

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // set(P_RS, B_RS);

	_fast_fill_8(0x0,((disp_x_size+1)*(disp_y_size+1)));

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::fillScr(byte r, byte g, byte b)
{
	word color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
	fillScr(color);
}

void UTFT::fillScr(word color)
{
	long i;
	char ch, cl;
	
	ch=byte(color>>8);
	cl=byte(color & 0xFF);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	clrXY();

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // set(P_RS, B_RS);

	if(ch==cl)
		{
		_fast_fill_8(ch,((disp_x_size+1)*(disp_y_size+1)));
		}
	else
		{
			for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
				LCD_Writ_Bus(ch,cl);
		}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::setColor(byte r, byte g, byte b)
{
	fch=((r&248)|g>>5);
	fcl=((g&28)<<3|b>>3);
}

void UTFT::setColor(word color)
{
	fch=byte(color>>8);
	fcl=byte(color & 0xFF);
}

word UTFT::getColor()
{
	return (fch<<8) | fcl;
}

void UTFT::setBackColor(byte r, byte g, byte b)
{
	bch=((r&248)|g>>5);
	bcl=((g&28)<<3|b>>3);
	_transparent=false;
}

void UTFT::setBackColor(uint32_t color)
{
	if (color==VGA_TRANSPARENT)
		_transparent=true;
	else
	{
		bch=byte(color>>8);
		bcl=byte(color & 0xFF);
		_transparent=false;
	}
}

word UTFT::getBackColor()
{
	return (bch<<8) | bcl;
}

void UTFT::setPixel(word color)
{
	LCD_Write_DATA((color>>8),(color&0xFF));	// rrrrrggggggbbbbb
}

void UTFT::drawPixel(int x, int y)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	setXY(x, y, x, y);
	setPixel((fch<<8)|fcl);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::drawLine(int x1, int y1, int x2, int y2)
{
	if (y1==y2)
		drawHLine(x1, y1, x2-x1);
	else if (x1==x2)
		drawVLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;

		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (true)
			{
				setXY (col, row, col, row);
				LCD_Write_DATA (fch, fcl);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t   -= dy;
				}
			} 
		}
		else
		{
			int t = - (dx >> 1);
			while (true)
			{
				setXY (col, row, col, row);
				LCD_Write_DATA (fch, fcl);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			} 
		}
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	}
	clrXY();
}

void UTFT::drawHLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		x -= l;
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	setXY(x, y, x+l, y);
	if (fch==fcl)
		{
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // set(P_RS, B_RS);
		_fast_fill_8(fch,l);
		}
	else
		{
		for (int i=0; i<l+1; i++)
				{
					LCD_Write_DATA(fch, fcl);
				}
		}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::drawVLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		y -= l;
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
	setXY(x, y, x, y+l);
	if (fch==fcl)
		{
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // set(P_RS, B_RS);
		_fast_fill_8(fch,l);
		}
	else
		{
		for (int i=0; i<l+1; i++)
				{
					LCD_Write_DATA(fch, fcl);
				}
		}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::printChar(byte c, int x, int y)
{
	byte i,ch;
	word j;
	word temp; 

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
  
	if (!_transparent)
	{
		if (orient==PORTRAIT)
		{
			setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);
	  
			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j++)
			{
				ch=pgm_read_byte(&cfont.font[temp]);
				for(i=0;i<8;i++)
				{   
					if((ch&(1<<(7-i)))!=0)   
					{
						setPixel((fch<<8)|fcl);
					} 
					else
					{
						setPixel((bch<<8)|bcl);
					}   
				}
				temp++;
			}
		}
		else
		{
			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j+=(cfont.x_size/8))
			{
				setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));
				for (int zz=(cfont.x_size/8)-1; zz>=0; zz--)
				{
					ch=pgm_read_byte(&cfont.font[temp+zz]);
					for(i=0;i<8;i++)
					{   
						if((ch&(1<<i))!=0)   
						{
							setPixel((fch<<8)|fcl);
						} 
						else
						{
							setPixel((bch<<8)|bcl);
						}   
					}
				}
				temp+=(cfont.x_size/8);
			}
		}
	}
	else
	{
		temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for(j=0;j<cfont.y_size;j++) 
		{
			for (int zz=0; zz<(cfont.x_size/8); zz++)
			{
				ch=pgm_read_byte(&cfont.font[temp+zz]); 
				for(i=0;i<8;i++)
				{   
				
					if((ch&(1<<(7-i)))!=0)   
					{
						setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
						setPixel((fch<<8)|fcl);
					} 
				}
			}
			temp+=(cfont.x_size/8);
		}
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::rotateChar(byte c, int x, int y, int pos, int deg)
{
	byte i,j,ch;
	word temp; 
	int newx,newy;
	double radian;
	radian=deg*0.0175;  

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0;j<cfont.y_size;j++) 
	{
		for (int zz=0; zz<(cfont.x_size/8); zz++)
		{
			ch=pgm_read_byte(&cfont.font[temp+zz]); 
			for(i=0;i<8;i++)
			{   
				newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

				setXY(newx,newy,newx+1,newy+1);
				
				if((ch&(1<<(7-i)))!=0)   
				{
					setPixel((fch<<8)|fcl);
				} 
				else  
				{
					if (!_transparent)
						setPixel((bch<<8)|bcl);
				}   
			}
		}
		temp+=(cfont.x_size/8);
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	clrXY();
}

void UTFT::print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (orient==PORTRAIT)
	{
	if (x==RIGHT)
		x=(disp_x_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_x_size+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x==RIGHT)
		x=(disp_y_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_y_size+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg==0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void UTFT::print(String st, int x, int y, int deg)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	print(buf, x, y, deg);
}

void UTFT::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	print(st,x,y);
}

void UTFT::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg=false;

	if (dec<1)
		dec=1;
	else if (dec>5)
		dec=5;

	if (num<0)
		neg = true;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (uint i=0; i<sizeof(st); i++)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (uint i=1; i<sizeof(st); i++)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (uint i=0; i<sizeof(st); i++)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	print(st,x,y);
}

void UTFT::setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.offset=fontbyte(2);
	cfont.numchars=fontbyte(3);
}

uint8_t* UTFT::getFont()
{
	return cfont.font;
}

uint8_t UTFT::getFontXsize()
{
	return cfont.x_size;
}

uint8_t UTFT::getFontYsize()
{
	return cfont.y_size;
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;

	if (scale==1)
	{
		if (orient==PORTRAIT)
		{
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
			setXY(x, y, x+sx-1, y+sy-1);
			for (tc=0; tc<(sx*sy); tc++)
			{
				col=pgm_read_word(&data[tc]);
				LCD_Write_DATA(col>>8,col & 0xff);
			}
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
		}
		else
		{
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+ty, x+sx-1, y+ty);
				for (tx=sx-1; tx>=0; tx--)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					LCD_Write_DATA(col>>8,col & 0xff);
				}
			}
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
		}
	}
	else
	{
		if (orient==PORTRAIT)
		{
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				for (tsy=0; tsy<scale; tsy++)
					for (tx=0; tx<sx; tx++)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
			}
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
		}
		else
		{
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				for (tsy=0; tsy<scale; tsy++)
				{
					setXY(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					for (tx=sx-1; tx>=0; tx--)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
				}
			}
			HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
		}
	}
	clrXY();
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		drawBitmap(x, y, sx, sy, data);
	else
	{
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				col=pgm_read_word(&data[(ty*sx)+tx]);

				newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				setXY(newx, newy, newx, newy);
				LCD_Write_DATA(col>>8,col & 0xff);
			}
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
	}
	clrXY();
}

void UTFT::lcdOff()
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::lcdOn()
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::setContrast(char c)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

int UTFT::getDisplayXSize()
{
	if (orient==PORTRAIT)
		return disp_x_size+1;
	else
		return disp_y_size+1;
}

int UTFT::getDisplayYSize()
{
	if (orient==PORTRAIT)
		return disp_y_size+1;
	else
		return disp_x_size+1;
}

void UTFT::setBrightness(byte br)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::setDisplayPage(byte page)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

void UTFT::setWritePage(byte page)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // clear(P_CS, B_CS);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // set(P_CS, B_CS);
}

/***** Hardware specific bus defines ***************************************************/

void UTFT::LCD_Writ_Bus(char VH,char VL)
{

// Pins depend on your display or wiring
//	printf("bus: %X, %X\n", VH, VL);
	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (VH & 1) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, (VH & 2) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, (VH & 4) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, (VH & 8) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (VH & 16) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (VH & 32) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (VH & 64) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (VH & 128) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);

	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (VL & 1) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, (VL & 2) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, (VL & 4) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, (VL & 8) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (VL & 16) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (VL & 32) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (VL & 64) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (VL & 128) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);

}

void UTFT::_fast_fill_8(int ch, long pix)
{
	long blocks;

	HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (ch & 1) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, (ch & 2) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, (ch & 4) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, (ch & 8) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (ch & 16) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (ch & 32) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (ch & 64) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (ch & 128) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
//printf("fastbus: %X\n",ch);

	blocks = pix/16;
	for (int i=0; i<blocks; i++)
	{
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);

	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16)+1; i++)
		{
			pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin); pulse_low(LCD_WR_GPIO_Port, LCD_WR_Pin);
		}

}
void UTFT::_convert_float(char *buf, double num, int width, byte prec)
{
	dtostrf(num, width, prec, buf);
}
