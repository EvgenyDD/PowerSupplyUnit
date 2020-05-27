/* Includes ------------------------------------------------------------------*/
#include "global.h"
#include "display.h"
#include "sound.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ON              0
#define OFF             1
#define ALARM           2
#define TEMPERATURE     3
#define TURN_ON         4
#define ALARMPROTECT    5
#define DISPLAY         6
#define ALARMPROTECT2   7

#define CALIBRATION     3
#define FAIL            4

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
IND ind;                  //????????? ??? ?????? ?? ?????????
volatile u16 *beepCount;                  //????????? ??? ?????
volatile u8 *beepFlg;                     //????????? ??? ?????
volatile u16 Upwm;                        //???????? PWM ?????????? 2^10  

volatile u8 Debounce[4] = {0, 0, 0, 0};   //??? ??????
volatile u16 counterTemperature = 1525;   //5sec on fan update
volatile u16 protectCurrVal = 500;       //???????? ?????? ?????? ?? ????

volatile struct{                   //?????? ? ???
  u16 U, NTC;
  u16 Temperature;
  u16 I5, I12;
  u16 I10A, I1A;
}adc;
u16 adcError[4]={0,0,0,0};                 //???????? ??? ?????????? ???

volatile u8 visionCurr = 0;               //?????, ??????????? ?????? ???????????? ???/???????? ???
u8 triggerU = 0;                          //bool ??? ?????????/?????????? ?????? ?? ?????????????? ??????????
u8 threshold = 30;
u16 idealU = 180;

static u8 hold=10;


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Fan
* Description    : calculate fan speed
*******************************************************************************/
void Fan(void)
{    
        if(!counterTemperature)
        {
            if     (adc.NTC < 470) OCR1A = 0; 
            else if(adc.NTC >= 470 && adc.NTC < 610) OCR1A = 2*adc.NTC - 620; 
            else {
              OCR1A = 0xFFFF;
              Sound(TEMPERATURE);
            }
            
            //adding some hystheresis
            if(OCR1A>100 && OCR1A < 220)
                counterTemperature = 10500; //~30 sec. 
            else
                counterTemperature = 1700; //5 sec.
            
            if(ind.protectionMode != 2) triggerU = 1;
            if(DIP1) threshold = 65;
        }        
}


/*******************************************************************************
* Function Name  : ADC_U
* Description    : calculate voltage from measured ADC channel
* Return         : voltage in x*0.1Volts
*******************************************************************************/
int ADC_U(void)
{       
        return (int)((float)adc.U * 0.2711 + 1.3205); //?????????? ???????
}


/*******************************************************************************
* Function Name  : ADC_I
* Description    : calculate current from measured ADC channels
* Return         : main channel current in milliAmperes
*******************************************************************************/
int ADC_I(void)
{       
        if(adc.I1A < 980)
        {
            if(adc.I1A > adcError[0]) 
                return (int)((float)(adc.I1A - adcError[0]) * 0.84); //??? ?? 850 ??
            else return 0;
        }
        else
        {
            if(adc.I10A > adcError[1])
                return (int)((float)(adc.I10A - adcError[1]) * 9.226); //??? ????? 850 ??
            else
                return 0;           
        }  
}


/*******************************************************************************
* Function Name  : ADC_I5
* Description    : calculate current from measured ADC channel
* Return         : sub 5V channel current in milliAmperes
*******************************************************************************/
int ADC_I5(void)
{       
        if(adc.I5 > adcError[2])
          return (int)((float)(adc.I5 - adcError[2]) * 2.207); //??? ?? 5? - ??????
        else return 0;
}


/*******************************************************************************
* Function Name  : ADC_I12
* Description    : calculate current from measured ADC channel
* Return         : sub 12V channel current in milliAmperes
*******************************************************************************/
int ADC_I12(void)
{       
        if(adc.I12 > adcError[3])
          return (int)((float)(adc.I12 - adcError[3]) * 2.158); //??? ?? 12? - ??????
        else return 0;
}


/*******************************************************************************
* Function Name  : Buttons
* Description    : read buttons state
*******************************************************************************/
void Buttons(void)
{       
        if(BitIsSet(Debounce[0], 1)){
          Debounce[1]--;
          if(!Debounce[1]) {ClearBit(Debounce[0], 1); GIFR &= ~(1<<INT0); GICR |= (1<<INT0); } //?????? ? ????????
        }
           
        if(BitIsSet(Debounce[0], 2)){   
          Debounce[2]--;
          if(!Debounce[2]) {ClearBit(Debounce[0], 2); GIFR |= (1<<INTF2); GICR |= (1<<INT2); } //ON/OFF
        }
           
        if(BitIsSet(Debounce[0], 3)){   
          Debounce[3]--;
          if(!Debounce[3]) {ClearBit(Debounce[0], 3); } //disp mode
        }

        //?????? ???????????? ???? ???????
        if(BitIsClear(Debounce[0], 3))
        { 
            if(!PIND_Bit1) 
            {              
                if(ind.displayMode != 2) ind.displayMode++;
                else ind.displayMode = 0;
                  
                SetBit(Debounce[0],3); 
                Debounce[3] = 16;
                  
                ind.displayMode = ind.displayMode;
                Display(&ind);
                Sound(DISPLAY); 
            }
        }   
}


/*******************************************************************************
* Function Name  : Init
* Description    : initialize peripherals
*******************************************************************************/
void Init(void)
{
//I/O
        DDRA = 0x00;  DDRB = 0xF8;  DDRC = 0xC3;  DDRD = 0x70;
        PORTA = 0x00; PORTB = 0x0C; PORTC = 0x0C; PORTD = 0x0F;
        
//ADC
        //??? - Avcc, ???????????? ?????
        ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) | (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);
        //ADC enabled, f=FREQ/128, ADC_int enabled 
        ADCSRA = (1<<ADEN)|(0<<ADSC)|(0<<ADATE)|(1<<ADIE) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); 
        SFIOR = 0; 

//Interrupts        
        GICR |= (1<<INT0)|(1<<INT1)|(1<<INT2);
        MCUCR |= (1<<ISC01)|(0<<ISC00)|(1<<ISC11)|(0<<ISC10);
        MCUCSR &= ~(1<<ISC2);

//Timer        
        TCCR1A |= (1<<COM1A1)|(1<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(1<<WGM10)|(1<<WGM11);
        TCCR1B |= (1<<WGM12)|(0<<WGM13)|(0<<CS12)|(1<<CS11)|(0/*1*/<<CS10);
        TIMSK  |= (1<<TOIE2);       
        TCCR2=0x06; TCNT2=0x00;        
        TCCR0=0x08; TCNT0=0x00; OCR0=0;
        
        Display(&ind);
        
        beepFlg   = RetFlag();
        beepCount = RetCounter();
        
        Upwm = 180; //5V on OUT 
        adc.U = 180;
        __enable_interrupt(); 
        ADCSRA |= (1<<ADSC); //enable ADC interrupt
        
ind.over = 0;
ind.protectionMode=0;
ind.stringLed=0;

        _delay_ms(100);
        for(u8 f=0; f<8; f++){
            adcError[0] += adc.I1A;
            adcError[1] += adc.I10A;
            adcError[2] += adc.I5;
            adcError[3] += adc.I12;
            _delay_ms(50);
        }
        adcError[0] >>= 3;
        adcError[1] >>= 3;
        adcError[2] >>= 3;
        adcError[3] >>= 3;
        
        
        Sound(TURN_ON);
        ind.outEn = OutEN = 0;
        RELAY = 1;
                
}


/*******************************************************************************
* Function Name  : main
* Description    : main routine
*******************************************************************************/
int main(void)
{  
        Init();            
        
        while(1)
        {
            if(ind.displayMode != FAIL)
            {
                OCR1B = Upwm;
                
                ind.voltage = ADC_U();
                
                Display(&ind);
                if(ind.displayMode == CALIBRATION) ind.displayMode = 0;
                
                if(ind.protectionMode == 1 && OutEN) //string of led
                {
                   ind.stringLed = (u8)((6 * ind.current) / protectCurrVal);
                }
                else ind.stringLed = (u8)((6 * ind.current) / 5000);
                
		            if(!ind.displayMode)
		            { 
		            		if(!visionCurr) ind.current = ADC_I();
		                else { visionCurr--; ind.current = protectCurrVal; }
		            }
		            if(ind.displayMode == 1) ind.current = ADC_I5();
		            if(ind.displayMode == 2) ind.current = ADC_I12();              
                
                Fan();   
                Buttons();
            }
            else
            {
                //__disable_interrupt();
                OCR1B=0;
                OutEN = 0;
                ind.outEn = OutEN;
                ind.over = 1;
                ind.stringLed = 6;
                Fan();   
                
                Display(&ind);              
            }
            if(!DIP1) threshold = 250;
        }
}


/*******************************************************************************
* Function Name  : measure
* Description    : ADC Interrupt handler
*******************************************************************************/
#pragma vector = ADC_vect                             
__interrupt void measure(void)  
{     
    static u8 chanel = 0;
    static u16 adcCount = 0;
    static long sum = 0;
    u16 temp = 0;    
        
        temp = ADCL;
        temp |= (ADCH<<8); 
        sum += temp;
        
        adcCount++;    
        if(adcCount == 257) //?????????? 256 ???
        {       
            switch(chanel) //?????? ????? ???
            {
                case 0: 
                      chanel++;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0); //?? 1 ?????
                      adc.I10A = (sum>>8);
                    	break;
                    	
                case 1:
                      chanel++;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0); //?? 2 ?????
                      adc.I5 = (sum>>8);
                  		break;
                  		
                case 2:
                      chanel++;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0); //?? 3 ?????
                      adc.I1A = (sum>>8);                      
                  		break;
                  		
                case 3:
                      chanel++;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0); //?? 4 ?????
                      adc.I12 = (sum>>8);
                  		break;
                  		
                case 4:
                      chanel++;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0); //?? 5 ?????
                      adc.NTC = (sum>>8);
                  		break;
                  		
                case 5:
                      chanel = 0;
                      ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR) 
                      |(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0); //?? 0 ?????
                      adc.U = (sum>>8);
                  		break;
            }
            sum = 0;
            adcCount = 0;
        }
        ADCSRA |= (1<<ADSC); //????? ????????? ?????????????? ???
}


/*******************************************************************************
* Function Name  : time
* Description    : 305Hz Timer2 Interrupt handler
*******************************************************************************/
#pragma vector = TIMER2_OVF_vect         
__interrupt void time(void) 
{
  
        //????????? ?????
        if((*beepFlg) == 1)                                   
        { 
            (*beepCount)--;
            if((*beepCount) == 0) {(*beepFlg) = 2; SoundProcessing(); }   //??????? ????????? ????
        }
        
        //?????? ??? ???????????
        if(counterTemperature) counterTemperature--;          
        
        //??????????? ?? ???? ? ?????? ???????
        if((ADC_I() > protectCurrVal) && (ind.protectionMode == 1))       
        {
            OutEN = 0;
            ind.outEn = OutEN;
            ind.over = 1;
            ind.stringLed = 6;
            Sound(ALARMPROTECT);
        }
        
        //??????????? ?? ???? ? ??????? ??????
        if(ADC_I() > 5500 && (ind.protectionMode == 0))       
        {
            OutEN = 0;
            ind.outEn = OutEN;
            ind.over = 1;
            Sound(ALARMPROTECT2);
        }
   

static u8 time;
if(time++>hold)
{
        //???????????? ????
        if((ind.protectionMode == 2) && OutEN)       
        {
          u8 needCorrect=0;
          
          if(protectCurrVal<500) 
          {
            if(abs(ADC_I() - protectCurrVal) > 2) needCorrect = 1;
          }
          if(protectCurrVal<2000 && protectCurrVal>=500)
          {
            if(abs(ADC_I() - protectCurrVal) > 20) needCorrect = 1;
          }
          if(protectCurrVal>=2000)
          {
            if(abs(ADC_I() - protectCurrVal) > 50) needCorrect = 1;
          }
          
            
          if(needCorrect)
          {
            
            if(ADC_I() > protectCurrVal) 
            {
                Upwm--;
                //while(Upwm >= idealU) {Upwm --;}
            }
            if(ADC_I() < protectCurrVal && Upwm<= idealU)
            {
                //while(Upwm >= idealU) {Upwm --;}
                /*if(Upwm < idealU) */Upwm++;
            }
            
            if(Upwm >= idealU && OutEN) ind.maxOCR=1;
            else ind.maxOCR=0;
            
            if(abs(ADC_I() - protectCurrVal) < 25) hold = 60;
            else hold = 10;
          }
        }
time=0;
}
        
        
        u16 comp = (u16)((float)adc.U * 1.05);        
        if((abs(comp - Upwm) > threshold) && triggerU)        //over-, under - voltage
        {
            OutEN = 0;
            ind.outEn = OutEN;
            ind.over = 1;
            Beep(14, 100);
            ind.displayMode = FAIL;
        }   

}


/*******************************************************************************
* Function Name  : Encoder
* Description    : INT1 Interrupt handler (Encoder handler)
*******************************************************************************/
#pragma vector = INT1_vect                            
__interrupt void Encoder(void)  
{  
        //_delay_us(800);
        if(ind.protectionMode == 0)
        {        
          if (PIND_Bit0 == 0) {if(Upwm <= 1019) {Upwm += 4; if(Upwm>350){RELAY=0;} Beep(20, 8);}}
          else {if(Upwm >= 4) {Upwm -= 4; if(Upwm<300){RELAY=1;} Beep(20, 8);}} 
        }
        if(ind.protectionMode)
        {
            if (PIND_Bit0 == 0) 
            {
                if(protectCurrVal < 4951.) 
                {
                  if(protectCurrVal>=100)
                  {
                    protectCurrVal += 50; Beep(30, 8);
                  }
                  else
                  {
                    protectCurrVal += 2; Beep(32, 8);
                  }
                }
            }
            else 
            {
                if(protectCurrVal > 100) 
                {
                  protectCurrVal -= 50; 
                  Beep(30, 8); 
                }
                else
                  if(protectCurrVal > 3)
                  {
                    protectCurrVal -= 2; Beep(32, 8);
                  }
            } 
            visionCurr = 150;
        }
        
        threshold = 140;
        counterTemperature = 1525;
        
        GIFR &= ~(1<<INT1); //????????? ?????????? INT1 
        
}


/*******************************************************************************
* Function Name  : ButtonEncoder
* Description    : INT0 Interrupt handler (Encoder button handler)
*******************************************************************************/
#pragma vector = INT0_vect                            
__interrupt void ButtonEncoder(void)  
{  
        _delay_us(800);
        
        if (ind.protectionMode == 2) ind.protectionMode = 0;
        else ind.protectionMode++; 
        
        if(ind.protectionMode == 2) 
        { idealU = Upwm; triggerU = 0;}
        else
        { triggerU = 1;}
          
        ind.outEn = OutEN = 0;
        SetBit(Debounce[0], 1); Debounce[1] = 12; //enc btn
        GICR &= ~(1<<INT0); //????????? ?????????? INT0
}


/*******************************************************************************
* Function Name  : EnaOUT
* Description    : INT2 Interrupt handler (Enable/Disable Button handler)
*******************************************************************************/
#pragma vector = INT2_vect                            
__interrupt void EnaOUT(void)  
{  
        _delay_us(800);
        
        OutEN = ~OutEN;
        ind.outEn = OutEN;
        ind.over = 0;
        ind.stringLed = 0;
        
        if(ind.outEn) Sound(ON);
        else Sound(OFF);        
        
        SetBit(Debounce[0], 2); Debounce[2] = 20;      
        GICR &= ~(1<<INT2); //????????? ?????????? INT2
}