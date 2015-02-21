/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile u16 beepCounter = 0; //счетчик длительности звука
volatile u8 beepFlag = 0;  

u8 melodyFlag = 0;            //если =1 - проигрывается мелодия
u8 melodyStep = 0;            //шаг в мелодии [нота по-порядку]

//мелодии
__flash u16 On[] = {20,100, 14,100,  0,0};       //включение выхода
__flash u16 Off[] = {14,100, 20,100,  0,0};      //выключение выхода

__flash u16 Alarm[] = {10,1000,  0,0};    //сработала защита
__flash u16 OverTemp[] = {20,300, 21,300,  0,0}; //сработала защита по температуре    

__flash u16 TurnOn[] = {20,100, 19,100, 18,100,  0,0}; //включение  
__flash u16 OverProtect[] = {17,300, 0,300, 17,300, 0,300, 17,300, 0,300, 17,300, 0,0};

__flash u16 DispChange[] = {25,150, 0,100, 25,150, 0,0};
__flash u16 OverProtect2[] = {17,200, 0,200, 17,200, 0,200, 17,200, 0,200, 17,200, 0,200, 13,200, 0,0};

u16 __flash *pSong;           //указатель на мелодию
__flash u16 __flash* melodyList[] = {On, Off,  Alarm, OverTemp,  TurnOn, OverProtect, DispChange, OverProtect2};  //список мелодий

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Beep 
* Description    : play note
* Input					 : note tone, note length
*******************************************************************************/
__monitor void Beep(u8 toneOCR, u16 mSec)
{ 
        if (!mSec)
        {
            TCCR0 = 0x28;
            DDRB_Bit3 = 0;  //вырубаем выход звука        
            beepFlag = 0;
            beepCounter = 0;
        }
        else
        {
            if(!beepFlag)
            {
                DDRB_Bit3 = 1; 
                TCNT0 = 0x00;
                beepFlag = 1; //если == 1 - то работает декремент beepCounter
                
                if(!toneOCR) {TCCR0 = 0x28; DDRB_Bit3 = 0;} 
                if(toneOCR)  {OCR0 = toneOCR; TCCR0 = 0x1C;}
                
                beepCounter = (u16)((float)mSec*0.305);
            }
        }
}


/*******************************************************************************
* Function Name  : SoundProcessing
* Description    : play melodies
*******************************************************************************/
__monitor void SoundProcessing(void)
{   
        if(beepFlag == 2) //если закончилось воспроизведение звука
        {
            if(melodyFlag == 1) //если у нас поигрывается песня
            { 
                if( !pSong[ melodyStep*2+1 ] )    //[длительность ноты = 0] если песня закончилась
                {
                    melodyFlag = 0; 
                    melodyStep = 0;
                    beepFlag = 0;
                    Beep(0,0);
                }
                else    //закончилось воспроизведение ноты
                {                                 
                    beepFlag = 0;
                    Beep( (u8)(pSong[ melodyStep*2 ]), pSong[ melodyStep*2+1 ] );
                    melodyStep++;
                }
            }
            else
            {
                Beep(0, 0);  //в случае проигрывания одного звука
            }    
        }            
}


/*******************************************************************************
* Function Name  : Sound
* Description    : start melody playing
* Input					 : song(melody) number
*******************************************************************************/
__monitor void Sound(u8 songNumber)
{
            pSong = melodyList[songNumber]; //выбираем мелодию
            
            melodyStep = 0; //шаг в мелодии
            melodyFlag = 1; //выбрано воспроизведение мелодии   
            beepFlag = 2;
            
            SoundProcessing();
}


/*******************************************************************************
* Function Name  : RetFlag
* Description    : start melody playing
* Return				 : pointer to beepFlag
*******************************************************************************/
volatile u8* RetFlag(void)
{
  return &beepFlag;
}


/*******************************************************************************
* Function Name  : RetCounter
* Description    : 
* Return				 : pointer to beepCounter
*******************************************************************************/
volatile u16* RetCounter(void)
{
  return &beepCounter;  
}