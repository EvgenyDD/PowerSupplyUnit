#include "init.h"

volatile unsigned int Upwm=500, beepCounter=0;  // (0) = U, (1) = I, (2) = I_5, (3) = I_12, (4) = temp "C
volatile unsigned char beepFlag=0,Debounce[8]={0,0,0,0,0,0,0,0};
unsigned char IStringLeds=2, OverLoadLed=1, DisplayType=0, CountADC=8, ProtectProgNum=2, 
              melodyFlag=0, melodyStep=0, flagBeepReady=0;//, futureSoundBit=0;
//unsigned char beepFutureOCR, beepFlagFuture=1;
//unsigned int beepSoundLen;

unsigned char tempvar1 = 0;//***
/*
struct{
  unsigned int collect[6];
  unsigned char count[6];
  
  unsigned int U, NTC;
  unsigned int Temp;
  unsigned int I5, I12;
  unsigned int I10A, I1A;
}Adc;
 */

__flash unsigned int NULL[];
__flash unsigned int DoubleClick[] = {30,100, 22,100, 18,100, 0,800, 10,500, 0,0};
__flash unsigned int On[] = {30,100, 22,100, 18,100, 0,0};
__flash unsigned int Secund[] = {20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 20,5, 10,5, 0,0};

__flash unsigned int __flash* melodyList[] = {NULL, DoubleClick, On, Secund};
unsigned int __flash *pSong = DoubleClick;

int main(void)
{  
  init();
  //DDRC_Bit2 = 0;//***
  __enable_interrupt(); 
 // beep(18,1000);
  //sound(1);//beep(18,1000); 
while(1){
 //   ProtectProgNum=0;
    OCR1B = Upwm ;//(char)Upwm ;
    OCR1A = Upwm ;
    if (DisplayType == 0) display(melodyStep, 30, VoltReg, IStringLeds, ProtectProgNum, OverLoadLed);
    if (DisplayType == 1) display(5678, 0, Volt5, IStringLeds, ProtectProgNum, OverLoadLed);            //U__5
    if (DisplayType == 2) display(9012, 0, Volt12, IStringLeds, ProtectProgNum, OverLoadLed);           //U_12
    if (DisplayType == 3) display(0, 0, CALIBRATE, IStringLeds, ProtectProgNum, OverLoadLed);           //Calibrate
    
    fan();                     
    adc_U();
    buttons();
    
    OverLoadLed = 0;
    
    
}
}    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void fan(void){    //228 - min fan
}


void adc(void){/*
  CountADC--;
  if(CountADC==0){   CountADC=8;}ADCSRA |= (1<<ADSC);
  ADMUX = (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0);*/
}


void adc_U(void){
  /*
  CountADC--;
  if(CountADC==0){  
    //ADCSRA |= (1<<ADSC); 
    CountADC = 20;
  }
  ADMUX = (0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0);
  //Adc.U = (int)((float)Adc.collect[5] * 0.2711 + 1.3205) ;        //измерение напряжения
  */
}
//TOOL

void buttons(void){       //обработчик нажатий всех кнопок
    if(Debounce[0]){
      Debounce[1]--;
      if(!Debounce[1]) Debounce[0]=0;
    }
    /*if(Debounce[2]){
      Debounce[3]--;
      if(!Debounce[3]){Debounce[2]=0; GICR |= (1<<INT1);} // энкодер
    }*/
    if(Debounce[4]){
      Debounce[5]--;
      if(!Debounce[5]){Debounce[4]=0; GIFR &= ~(1<<INT0); GICR |= (1<<INT0);} // кнопка энкодера
    }
    if(Debounce[6]){
      Debounce[7]--;
      if(!Debounce[7]){Debounce[6]=0; GIFR |= (1<<INTF2); GICR |= (1<<INT2);} // ON/OFF
    }
  //кнопка переключения дисплея
    if(!Debounce[0]){ if(!PIND_Bit1) {
        if(DisplayType != 2) DisplayType++;
        else DisplayType = 0;
        Debounce[0]=1; Debounce[1]=16;
        sound(2);
    }}
    //sound
   // if(futureSoundBit)sound(futureSoundBit);
    
    if(!beepFlag){ 
      if(!melodyFlag){
      if( !pSong[ melodyStep*2+1 ] ) {flagBeepReady=0; melodyFlag=0; beep(0,0);} //закончилась мелодия
      if( pSong[ melodyStep*2+1 ] ) {                                 //закончилось воспроизведение ноты
        beepFlag=0;
        beep( pSong[melodyStep*2], pSong[melodyStep*2+1] );
        melodyStep++;
      }
    }
    }  

    if( beepFlag==3){
    if(!melodyFlag){ 
      if( !pSong[ melodyStep*2+1 ] ) {flagBeepReady=0; melodyFlag=0; beep(0,0);} //закончилась мелодия
      if( pSong[ melodyStep*2+1 ] ) {                                 //закончилось воспроизведение ноты
        beepFlag=0;
        beep( pSong[melodyStep*2], pSong[melodyStep*2+1] );
        melodyStep++;
      }
    } }     
    //if(beepFlag==3) beep(0,0);
}


void sound(unsigned char melodyNum){
  if(!melodyFlag){
    pSong = melodyList[melodyNum];
    melodyStep=0;
    melodyFlag=1; 
    //futureSoundBit=0;
  }
}


void beep(unsigned char toneOCR,unsigned int mSec){
  if (!mSec){
    TCCR0 = 0x28;
    beepFlag=0;
    beepCounter=0;
    DDRB_Bit3 = 0;
  }
  else{
    if(!beepFlag){
    DDRB_Bit3 = 1;
    TCNT0 = 0x00; 
    if(!toneOCR){TCCR0 = 0x28;DDRB_Bit3 = 0;}
    if(toneOCR) {OCR0 = toneOCR; TCCR0 = 0x1C;}
    beepCounter = (int)((float)mSec*0.305);
    beepFlag=1;
    }
  }
}











#pragma vector = TIMER2_OVF_vect
__interrupt void time(void)  // 305 Hz
{
  if(beepFlag==1){ //music
    beepCounter--;
    if(!beepCounter) {beepFlag=3;} 
  }
  //===
  tempvar1--;
  if(!tempvar1) {
    tempvar1=46;
    IStringLeds++;
    if(IStringLeds==7) IStringLeds=0;
  }

}

/*
#define GetResult(num);   Adc.count[num]++;\
                          adcTemp = ADCL;\
                          adcTemp |= (ADCH<<8);\
                          Adc.collect[num] += adcTemp;\
                          if(Adc.count[num]==64){\
                            //Adc.U = adcCollect[num];\
                            adcCollect[num]=0;\
                          }
*/

#pragma vector = ADC_vect                             //АЦП
__interrupt void measure(void)  
{  
  unsigned int adcTemp=0;
 // static unsigned char count = 0;
 // static unsigned int adcValue = 0;
  
  PORTC_Bit2 = ~PORTC_Bit2;//***
 /* 
  adcTemp = ADCL;
  adcTemp |=(ADCH<<8);  
  adcValue += adcTemp;

  count++;
  if (count == 64){
    //MassADC[0] = (adcValue>>6);
    adcValue = 0;
    count = 0;
  }
  */
 /* switch(ADMUX){
  case 0x65:  //5-U
    Adc.count[5]++;
                          adcTemp = ADCL;
                          adcTemp |= (ADCH<<8);
                          Adc.collect[5] += adcTemp;
                          if(Adc.count[5]==64){
    Adc.U = Adc.collect[5];
    Adc.collect[5]=0;}
    ADMUX=0x65;
    break;
  *case 0x60:  //0-I10A
    GetResult(0);
    Adc.I10A = Adc.collect[0];
    Adc.collect[0]=0;}
    ADMUX=0x62;
    break;
  case 0x62:  //2-I1A
    GetResult(2);
    Adc.I1A = Adc.collect[2];
    Adc.collect[2]=0;}
    ADMUX=0x61;
    break;
  case 0x61:  //1-I5
    GetResult(1);
    Adc.I5 = Adc.collect[1];
    Adc.collect[1]=0;}
    ADMUX=0x63;
    break;
  case 0x63:  //3-I12
    GetResult(3);
    Adc.I12 = Adc.collect[3];
    Adc.collect[3]=0;}
    ADMUX=0x64;
    break;
  case 0x64:  //4-NTC
    GetResult(4);
    Adc.NTC = Adc.collect[4];
    Adc.collect[4]=0;}
    ADMUX=0x65;
    break;
  }
  */
 // ADCSRA |= (1<<ADSC); 
}


#pragma vector = INT1_vect                            //энкодер
__interrupt void Encoder(void)  
{  
  delay_us(800);
  if (PIND_Bit0 == 0) {if(Upwm <= 1019) Upwm += 4;}
  else {if(Upwm >= 4) Upwm -= 4;} 
  
  //Debounce[2]=1;Debounce[3]=5;
  //GICR &= ~(1<<INT1);
  GIFR &= ~(1<<INT1); //выключаем прерывание INT1 
}


#pragma vector = INT0_vect                            //переключение режима защиты
__interrupt void ButtonEncoder(void)  
{  
  delay_us(800);
  if (ProtectProgNum == 2) ProtectProgNum = 0;
  else ProtectProgNum++;  
  Debounce[4]=1;Debounce[5]=10;
  GICR &= ~(1<<INT0); //выключаем прерывание INT0
}


#pragma vector = INT2_vect                            //on/off выхода регулируемого источника
__interrupt void EnaOUT(void)  
{  
  delay_us(800);
  OutEnaMOSFET = ~OutEnaMOSFET;
  
  if(OutEnaMOSFET){
   // beepFutureOCR=18;beepSoundLen=200;beepFlagFuture=1;
  }
  else {
   // beepFutureOCR=30;beepSoundLen=200;beepFlagFuture=1;
  }
  //futureSoundBit=1;
  Debounce[6]=1;Debounce[7]=16; 
  GICR &= ~(1<<INT2); //выключаем прерывание INT2
  //GIFR |= (1<<INTF2);
}