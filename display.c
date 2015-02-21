/* Includes ------------------------------------------------------------------*/
#include "display.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//segments: bedhcgaf
__flash unsigned char led_table[28]={
  0xEB,0x88,0xE6,0xAE,0x8D,0x2F,0x6F,0x8A,0xEF,0xAF,
  0xE9,0x20,0x63,0xCF,0x49,0x6D,0x44,0x65,0x67,   //Буквы
  0x40,0x60,0x70,0x78,0x7C,0x7D,                 //шкала 6_LED
  0x00, //0-символ
  0x47, 0x61 };
__flash unsigned char led_table_dot[10]={0xFB,0x98,0xF6,0xBE,0x9D,0x3F,0x7F,0x9A,0xFF,0xBF};
__flash unsigned char Catod[10]={0x84,0x54,0x94,0x64,0x34,0x24,0x14,0x74,0x4,0x44};

u8 previousMode = 5;
u8 digit[18];


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Display
* Description    : display information 
* Input					 : pointer to Indication structure
*******************************************************************************/
void Display(IND *ind){
    u16 leftDisplay  = ind->current;
    u16 rightDisplay = ind->voltage;
    u8 dig_data = 0; 
    static signed int flashingLights=10;
    
        if(previousMode != ind->displayMode) {ShiftL(ind); previousMode = ind->displayMode;}
        
        switch(ind->displayMode){
        case 0:                                   //global disp_out 
            for(u8 k=4; k>0; k--){ 
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              ShowDigit(dig_data, k-1, 0);
            }
            for(u8 k=7; k>3; k--){ 
              if(k==7) ShowDigit(25, k, 0);
              else{
              dig_data = rightDisplay % 10; 
              rightDisplay /= 10;
              ShowDigit(dig_data, k, 0);
              }
            }
          break;
          
        case 1:                                   //U__5
            for(u8 k=4; k>0; k--){ 
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              ShowDigit(dig_data, k-1, 1);
            }
            ShowDigit(10, 4, 1);
            ShowDigit(11, 5, 1);
            ShowDigit(11, 6, 1);
            ShowDigit(5, 7, 1);   
          break;
          
         case 2:                                   //U_12
            for(u8 k=4; k>0; k--){
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              ShowDigit(dig_data, k-1, 2);
            }
            ShowDigit(10, 4, 2);       ShowDigit(11, 5, 2);
            ShowDigit(1, 6, 2);        ShowDigit(2, 7, 2);   
          break;
          
         case 3:                                   //Calibrate
            ShowDigit(12, 0, 3);       ShowDigit(13, 1, 3);   // C + A
            ShowDigit(14, 2, 3);       ShowDigit(15, 3, 3);   //li + b
            ShowDigit(16, 4, 3);       ShowDigit(13, 5, 3);   // r + A
            ShowDigit(17, 6, 3);       ShowDigit(18, 7, 3);   // t + E
          break; 
          
          case 4:                                   //DC FAIL
            ShowDigit(26, 0, 4);       ShowDigit(13, 1, 4);   // F + A
            ShowDigit(1, 2, 4);        ShowDigit(27, 3, 4);   // 1 + L
            ShowDigit(25, 4, 4);       ShowDigit(25, 5, 4);   // r + A
            ShowDigit(25, 6, 4);       ShowDigit(25, 7, 4);   // t + E
          break; 
        }
    //******************************************************************************      
        switch(ind->stringLed){                             //вывод тока порога
         case 1: ShowDigit(19, 8, ind->displayMode); break;
         case 2: ShowDigit(20, 8, ind->displayMode); break;
         case 3: ShowDigit(21, 8, ind->displayMode); break;
         case 4: ShowDigit(22, 8, ind->displayMode); break;
         case 5: ShowDigit(23, 8, ind->displayMode); break;
         case 6: ShowDigit(24, 8, ind->displayMode); break;
        }
        
        dig_data = 0;      
        if(ind->protectionMode == 0) dig_data |= 0x80;       //grn
        if(ind->protectionMode == 1) dig_data |= 0x8;        //red    
        if(ind->protectionMode == 2 && (  (!(ind->maxOCR)) || (flashingLights > 0)) )
          dig_data |= 0x20;       //blu
        
        
        
        if(flashingLights > 0)
        {  
            flashingLights --;
        }
        else
        {
           if(flashingLights > (-9))
             flashingLights--;
           else
             flashingLights = 10;
        }
        
        
        if(ind->over && (flashingLights > 0))          //OverLoad LEDs
        {
            dig_data |= 0x1;  
        }
        
        if(ind->outEn)  dig_data |= 0x4;              //blue LED
        ShowData(dig_data, 9);
    //******************************************************************************       
}


/*******************************************************************************
* Function Name  : ShowDigit
* Description    : display number on 7seg display
* Input					 : number to display, it's position, mode
*******************************************************************************/
void ShowDigit(unsigned char num, unsigned char pos, int mod){
    u8 temp;
  
        if     ((pos==0)  && (mod == 0))  temp = led_table_dot[num];
        else if((pos==5)  && (mod == 0))  temp = led_table_dot[num];
        else if((pos==0)  && (mod == 1))  temp = led_table_dot[num];
        else if((pos==0)  && (mod == 2))  temp = led_table_dot[num];
        else temp = led_table[num];
        
        if(previousMode == mod ) digit[pos] = temp; //для эффекта сдвиг влево
        
        ShowData(temp, pos);
}


/*******************************************************************************
* Function Name  : ShowData
* Description    : display digit?
* Input					 : number to display, it's position
*******************************************************************************/
void ShowData(unsigned char num, unsigned char pos){
        ST_CP = 0;  
        for (char i=0; i<8; i++)
        {   
          DS = num & 1;
          SH_CP = 1;
          SH_CP = 0;
          num >>= 1;
        }
        ST_CP = 1;     
        PORTB = Catod[pos];
        _delay_ms(2);
}


/*******************************************************************************
* Function Name  : ShiftL
* Description    : shift display data to the left
* Input					 : pointer to indication structure
*******************************************************************************/
__monitor void ShiftL(IND *ind){ 
    u8 dig_data;
    u16 leftDisplay = ind->current;
    u16 rightDisplay = ind->voltage;
    digit[8] = led_table[25];
    digit[9] = led_table[25];
        
        switch(ind->displayMode){
        case 0:                                   //global disp_out 
            for(u8 k=4; k>0; k--){ 
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              if(k==0) digit[k+8+2-1] = led_table_dot[dig_data];
              else digit[k+8+2-1] = led_table[dig_data];
            }
            for(u8 k=6; k>3; k--){ 
              dig_data = rightDisplay % 10; 
              rightDisplay /= 10;
              if(k==6) digit[k+8+2] = led_table_dot[dig_data];
              else digit[k+8+2] = led_table[dig_data];
            }
            digit[17]=led_table[25];
          break;
          
        case 1:                                   //U__5
            for(u8 k=4; k>0; k--){ 
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              if(k==0) digit[k+8+2-1] = led_table_dot[dig_data];
              else digit[k+8+2-1] = led_table[dig_data];
            }
              digit[4+8+2] = led_table[10];
              digit[5+8+2] = led_table[11];
              digit[6+8+2] = led_table[11];

              digit[7+8+2] = led_table[5];  
          break;
          
         case 2:                                   //U_12
            for(u8 k=4; k>0; k--){ 
              dig_data = leftDisplay % 10; 
              leftDisplay /= 10;
              if(k==0) digit[k+8+2-1] = led_table_dot[dig_data];
              else digit[k+8+2-1] = led_table[dig_data];
            }
              digit[4+8+2] = led_table[10];
              digit[5+8+2] = led_table[11];
              digit[6+8+2] = led_table[1];

              digit[7+8+2] = led_table[2];   
          break;
          
         case 3:                                   //Calibrate
            digit[0+8+2] = led_table[12];
            digit[1+8+2] = led_table[13];
            digit[2+8+2] = led_table[14];
            digit[3+8+2] = led_table[15];
            digit[4+8+2] = led_table[16];
            digit[5+8+2] = led_table[13];
            digit[6+8+2] = led_table[17];
            digit[7+8+2] = led_table[18];
          break; 
        }
       
  
        u8 out[8];
        
	for(u8 i = 0; i<11; i++)     //сдвигаем влево текущие цифры
        {   
            out[0] = digit[i];
            out[1] = digit[i+1];
            out[2] = digit[i+2];
            out[3] = digit[i+3];
            
            out[4] = digit[i+4];
            out[5] = digit[i+5];
            out[6] = digit[i+6];
            out[7] = digit[i+7];
        
            for(u8 x = SHIFT; x; x--)
            {
                for(u8 j=0; j<8; j++) ShowData(out[j], j); 
                
                
                switch(ind->stringLed){                             //вывод тока порога
                    case 1: ShowDigit(19, 8, ind->displayMode); break;
                    case 2: ShowDigit(20, 8, ind->displayMode); break;
                    case 3: ShowDigit(21, 8, ind->displayMode); break;
                    case 4: ShowDigit(22, 8, ind->displayMode); break;
                    case 5: ShowDigit(23, 8, ind->displayMode); break;
                    case 6: ShowDigit(24, 8, ind->displayMode); break;
                    }
                    
                    dig_data = 0;      
                    if(ind->protectionMode == 0) dig_data |= 0x80;       //grn
                    if(ind->protectionMode == 1) dig_data |= 0x8;        //red
                    if(ind->protectionMode == 2) dig_data |= 0x20;       //blu
                    
                    if(ind->over)   dig_data |= 0x1;                    //OverLoad LEDs
                    if(ind->outEn)  dig_data |= 0x4;                    //blue LED
                    ShowData(dig_data, 9);
            }
            
            
	}        
}