
#include "gd32vf103.h"
#include "lcd.h"
#include "pwm.h"
#include "adc.h"
#include "delay.h"
#include "fft.h"

int shiftArray(Complex arr[], uint8_t n){
	for (uint8_t i=n; i>0; i--){														// crossing x axes?..
		if (arr[i].real == 40) LCD_DrawPoint(i, arr[i].real, WHITE); 	// ..YES! draw this dot white..
		else 									 LCD_DrawPoint(i, arr[i].real, BLACK);  // ..else in black
    
		arr[i+1] = arr[i];		                       // shift 1 step
    if (i <= 128)
			LCD_DrawPoint(i+1, arr[i+1].real, RED);      // display moved value
  }
}

int main(void){
  uint8_t len = 1;								 
  //int values[128] = {0};    //cols
  int idle =0;//int dac=0;
  int16_t adcr, tmpr = 40;

	Complex samples[128] = { {0, 0} }; 				// initialize all samples to 0 first

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
      samples[1].real = tmpr;                     // save the value
      adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
    } 

		delay_1ms(12);							// one iteration takes 20 ms
		
		// shifting lcd dots
		shiftArray(samples, len);
		if (len <= 128) len++;

		LCD_DrawPoint_big(4, tmpr, BLACK);      // remove trail before reading next value
		adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																	ADC_REGULAR_CHANNEL);
	
		while(!delay_finished()); 		// wait until iteration done

		// if (idle==250){			// wait until 5 seconds have passed
		// 	fft(samples, )
		// }
	}
}