#include "gd32vf103.h"
#include "lcd.h"
#include "pwm.h"
#include "adc.h"
#include "delay.h"
#include "cordic-math.h"

int shiftArray(int arr[], int n){
	for (int i=n; i>0; i--){														// crossing x axes?..
		if (arr[i] == 40) LCD_DrawPoint(i, arr[i], WHITE); 	// ..YES! draw this dot white..
		else 							LCD_DrawPoint(i, arr[i], BLACK);  // ..else in black
    
		arr[i+1] = arr[i];                      // shift 1 step
    LCD_DrawPoint(i+1, arr[i+1], RED);      // display moved value
  }
}

int main(void){
  uint8_t len = 2;								 
  int values[160] = {0};    //cols
  //int dac=0;
  int16_t adcr, tmpr = 40;

  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  LCD_Clear(BLACK);
  ADC3powerUpInit(1);                     // Initialize ADC0, Ch3 & Ch16

	// draw coordinates
	LCD_DrawLine(0, 0, 1, 79, WHITE);
	LCD_DrawLine(1, 40, 160, 40, WHITE);

  while (1) {
		//idle++;                             
    LCD_WR_Queue();                   // Manage LCD com queue!

    if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
      adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4095
      tmpr = (adcr * 79) >> 12; 									// scale adc value in range of 0-79 with divsion 4096
			LCD_DrawPoint_big(4, tmpr, YELLOW);         // display value on LCD
      values[1] = tmpr;                           // save the value
      adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
    } 

		delay_1ms(20);							// one iteration takes 20 ms
		
		// shifting lcd dots
		shiftArray(values, len);
		if (len <= 160) len++;

		LCD_DrawPoint_big(4, tmpr, BLACK);      // remove trail before reading next value
		adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																	ADC_REGULAR_CHANNEL);
	
		while(!delay_finished()); 		// wait until iteration done
  }
}