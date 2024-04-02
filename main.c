#include "reg52.h"
#include "intrins.h"
#include "onewire.h"
#include "iic.h"
#include "ds1302.h"

unsigned char code t_display[]={                       //±ê×¼×Ö¿â
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e};
unsigned char code t_display_dot[]={0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

sbit S1=P3^0;
sbit S2=P3^1;


unsigned char Read_addr[]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d,0x8f,0x91};
unsigned char Write_addr[]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90};
unsigned char Timer[]={0x55,0x59,0x23,0x02,0x04,0x03,0x24};	
unsigned char Timer_count=0;
unsigned char mode=1;
unsigned int temp;
unsigned char light;



void _Light();
void SMG_1();
void SMG_bit(unsigned char pos,unsigned char num);
void Delay1ms();
void SelectHC138(unsigned char channel,unsigned char dat);
void InitSys();
void DS1302_Read();
void DS1302_Write();
void DS1302();
void InitTimer0();
void ds18b20();
void SMG_2();
void ScanKey();
void Delay(unsigned char t);
void SMG_3();


void main()
{
	InitSys();
	DS1302_Write();
	InitTimer0();
	
	while(1)
	{
		ScanKey();
		
		
		switch(mode)
		{
			case 1:
				DS1302();
			break;
			case 2:
				SMG_2();
				ds18b20();
			break;
			case 3:
				SMG_3();
				_Light();
			break;
		}
	}
}





void SelectHC138(unsigned char channel,unsigned char dat)
{
	P0=dat;
	switch(channel)
	{
		case 0:
			P2=P2&0x1f|0x00;
		break;
		case 4:
			P2=P2&0x1f|0x80;
		break;
		case 5:
			P2=P2&0x1f|0xa0;
		break;
		case 6:
			P2=P2&0x1f|0xc0;
		break;
		case 7:
			P2=P2&0x1f|0xe0;
		break;
	}
	P2=P2&0x1f|0x00;
}

void InitSys()
{
	SelectHC138(4,0xff);
	SelectHC138(5,0x00);
	SelectHC138(6,0xff);
	SelectHC138(7,0xff);
	SelectHC138(0,0);
}

void SMG_bit(unsigned char pos,unsigned char num)
{
	SelectHC138(6,0x01<<pos);
	SelectHC138(7,num);
	Delay1ms();
	SelectHC138(7,0xff);
}

void Delay1ms()		//@11.0592MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}

void SMG_1()
{
	SMG_bit(7,~t_display[Timer[0]%16]);
	SMG_bit(6,~t_display[Timer[0]/16]);
	SMG_bit(5,~0x40);
	SMG_bit(4,~t_display[Timer[1]%16]);
	SMG_bit(3,~t_display[Timer[1]/16]);
	SMG_bit(2,~0x40);
	SMG_bit(1,~t_display[1]);
	SMG_bit(0,~0x40);
}

void DS1302_Write()
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for (i=0;i<3;i++)
	{
		Write_Ds1302_Byte(Write_addr[i],Timer[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}

void DS1302_Read()
{
	unsigned char i;
	for (i=0;i<3;i++)
	{
		Timer[i]=Read_Ds1302_Byte (Read_addr[i]);
	}
}

void DS1302()
{
	DS1302_Read();
	if(Timer[0]==0x00)
	{
		TR0=1;
		SelectHC138(5,0x10);
		
	}
	SMG_1();
}

void InitTimer0()
{
	EA=1;
	ET0=1;
	TMOD=0x01;
	TR0=0;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
}

void Timer0() interrupt 1
{
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	Timer_count++;
	if(Timer_count==40)
	{
			SelectHC138(5,0x00);
			Timer_count=0;
			TR0=0;
	}
}

void ds18b20()
{
	unsigned char MSB,LSB;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	Delay(500);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	
	temp=MSB;
	temp=temp<<8;
	temp=temp|LSB;
	temp=temp>>4;
	temp=temp*100;
	LSB=LSB*625;
	temp=temp+LSB;
}

void Delay(unsigned char t)
{
	while(t--)
	{
		if(mode==2)
			SMG_2();
		else if(mode==1)
			SMG_1();
		else
			SMG_3();
	}
}

void SMG_2()
{
	SMG_bit(7,~t_display[temp%10]);
	SMG_bit(6,~t_display[temp%100/10]);
	SMG_bit(5,~t_display_dot[temp%1000/100]);
	SMG_bit(4,~t_display[temp/1000]);
	SMG_bit(2,~0x40);
	SMG_bit(1,~t_display[2]);
	SMG_bit(0,~0x40);
}

void ScanKey()
{
	if(S1==0)
	{
		Delay(20);
		if(S1==0)
		{
			while(S1==0)
			{
				if(mode==2)
					SMG_2();
				else if(mode==1)
					SMG_1();
				else
					SMG_3();
			}
			if(mode==3)
				mode=1;
			else
				mode++;
		}
		
	}
	if(S2==0)
	{
		Delay(20);
		if(S2==0)
		{
			while(S2==0)
			{
				if(mode==2)
					SMG_2();
				else if(mode==1)
					SMG_1();
				else
					SMG_3();
			}
			if(mode==1)
				mode=3;
			else
				mode--;
		}
	}
}

void SMG_3()
{
	SMG_bit(7,~t_display[light%10]);
	SMG_bit(6,~t_display[light%100/10]);
	SMG_bit(5,~t_display[light/100]);
	SMG_bit(2,~0x40);
	SMG_bit(1,~t_display[3]);
	SMG_bit(0,~0x40);
}

void _Light()
{
	light=read_light();
	//light=light/5*255;
}