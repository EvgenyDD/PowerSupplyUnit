#ifndef SOUND_H
#define SOUND_H

/* Includes ------------------------------------------------------------------*/
#include "global.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
__monitor void Beep(u8 toneOCR, u16 mSec);
__monitor void Sound(u8 songNumber);
void SoundProcessing(void);

volatile u8* RetFlag(void);
volatile u16* RetCounter(void);

#endif //SOUND_H