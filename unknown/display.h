#ifndef DISPLAY_H
#define DISPLAY_H

/* Includes ------------------------------------------------------------------*/
#include "global.h"
#include "sound.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define SH_CP PORTC_Bit1  
#define ST_CP PORTC_Bit0  
#define DS    PORTD_Bit6
#define SHIFT 3


/* Exported functions ------------------------------------------------------- */
void Display(IND*);
void ShiftL(IND*);

void ShowDigit(unsigned char, unsigned char, int);
void ShowData(unsigned char, unsigned char);


#endif //DISPLAY_H