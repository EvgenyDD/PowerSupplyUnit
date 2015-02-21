#ifndef GLOBAL_H
#define GLOBAL_H

/* Includes ------------------------------------------------------------------*/
#include <ioavr.h>
#include <intrinsics.h>
#include <stdlib.h>
//#include <math.h>


/* Exported types ------------------------------------------------------------*/
typedef unsigned char u8;
typedef unsigned int u16;   

typedef struct{
  u16 current;         //����� ����� ������
  u16 voltage;         //������ ����� ������
  
  u8 displayMode;     //0-?  - ��� ���������
  u8 protectionMode;  //0-2  - ����� ������
  u8 outEn;           //bool - ����� LED
  u8 over;            //bool - ������� ���������� ����������  
  u8 stringLed;       //0-6  - ���-�� ����������� � �������
  u8 maxOCR;
}IND;


/* Exported constants --------------------------------------------------------*/
#define VoltReg     0
#define Volt5       1
#define Volt12      2
#define CALIBRATE   3

#define FREQ 20000000UL


/* Exported macro ------------------------------------------------------------*/
//#define SET_BIT(ADDRESS,BIT)  ((ADDRESS) |= (1<<(BIT)))
//#define RES_BIT(ADDRESS,BIT)  ((ADDRESS) |= (0<<(BIT)))
#define BitIsSet(reg, bit)        ((reg & (1<<bit)) != 0)
#define BitIsClear(reg, bit)      ((reg & (1<<bit)) == 0)
#define ClearBit(reg, bit)        reg &= (~(1<<(bit))) 
#define SetBit(reg, bit)          reg |= (1<<(bit)) 

#define _delay_us(us)          __delay_cycles((FREQ/1000000)*(us))
#define _delay_ms(ms)          __delay_cycles((FREQ/1000)*(ms))
#define _delay_s(s)            __delay_cycles((FREQ)*(s))

/* Exported define -----------------------------------------------------------*/
#define OutEN PORTC_Bit7          //���������� ������
#define RELAY PORTC_Bit6          //���� ������������ ���������� (15 ��� 26 �����) 
#define DIP1  PINC_Bit3
#define DIP2  PINC_Bit2


/* Exported functions ------------------------------------------------------- */

//extern IND ind;

#endif //GLOBAL_H