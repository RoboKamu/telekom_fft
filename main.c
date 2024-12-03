#include "gd32vf103.h"
#include "lcd.h"
#include "adc.h"
#include "delay.h"
#include "fft.h"
#include "cordic-math.h"

#define LCD_HEIGHT 79
#define LCD_WIDTH 160
#define FFT_WIDTH 128

#define DRAW_X_Y(COLOR) \
		LCD_DrawLine(0, 0, 1, LCD_HEIGHT, COLOR); \
		LCD_DrawLine(1, 40, LCD_WIDTH, 40, COLOR);

void shiftArray(Complex arr[], uint8_t n);
void visualize_fft(Complex *x, int16_t N);

int main(void){
  uint8_t len = 1;								 
  int idle =0;//int dac=0;
  int16_t adcr, tmpr = 40;

	Complex samples[FFT_WIDTH] = { {0, 0} }; 				// initialize all samples to 0 first

  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  LCD_Clear(BLACK);
  ADC3powerUpInit(1);                     // Initialize ADC0, Ch3 & Ch16

	// draw x-y axis
	DRAW_X_Y(WHITE);
	
  while (1) {                             
		idle++;
	  LCD_WR_Queue();                   // Manage LCD com queue!

		if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
			adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4095
			tmpr = (adcr * LCD_HEIGHT) >> 12; 					// scale adc value in range of 0-LCD_HEIGHT with divsion 4096
			LCD_DrawPoint_big(4, tmpr, YELLOW);         // display value on LCD
			samples[1].real = tmpr;                     // save the value
			adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
		} 

		delay_1ms(12);							// one iteration takes ~12 ms
		
		// shifting lcd dots
		shiftArray(samples, len);
		if (len <= FFT_WIDTH) len++;

		LCD_DrawPoint_big(4, tmpr, BLACK);      // remove trail before reading next value
		adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																	ADC_REGULAR_CHANNEL);
	
		while(!delay_finished()); 		// wait until iteration done
  
		if (idle==500){										// wait until 10 seconds have passed
			// testing fft visuals
			LCD_Clear(BLACK);
			fft(samples, len-1); 
			visualize_fft(samples, len-1);
			delay_1ms(2000);
			while(!delay_finished()); 		// display for 2 seconds
			
			// testing inverse visuals
			LCD_Clear(BLACK);
			inverse_fft(samples, len-1);
			visualize_fft(samples, len-1);
			delay_1ms(2000);
			while(!delay_finished()); 		// display for 2 seconds
			LCD_Clear(BLACK);

			DRAW_X_Y(WHITE);							// draw the original axis again
		}
	}
}

void shiftArray(Complex arr[], uint8_t n){
	for (uint8_t i=n-1; i>0; i--){														// crossing x axes?..
		if (arr[i].real == 40) LCD_DrawPoint(i, arr[i].real, WHITE); 	// ..YES! draw this dot white..
		else 									 LCD_DrawPoint(i, arr[i].real, BLACK);  // ..else in black
    
		arr[i+1] = arr[i];		                       // shift 1 step
    if (i < FFT_WIDTH)
			LCD_DrawPoint(i+1, arr[i+1].real, BLUE);      // display moved value
  }
}

void visualize_fft(Complex *x, int16_t N){				//where N has to be 2^k
	int32_t magnitudes[N]; 
	int32_t max_magnitude = 0;
	 
	for (int16_t i = 0; i<N; i++){
		magnitudes[i] = cordic_hypotenuse(x[i].imag, x[i].real);
		if (magnitudes[i] > max_magnitude){
			max_magnitude = magnitudes[i];
		}
	}
 
	// scale the magnitudes according to the max for the LCD
	for (int16_t i = 0; i<N; i++){
		// fixed division with max magnitude
		magnitudes[i] = (magnitudes[i] << CORDIC_MATH_FRACTION_BITS) / max_magnitude;
		// fixed multiplication with LCD height
		magnitudes[i] = (magnitudes[i] >> CORDIC_MATH_FRACTION_BITS) * (LCD_HEIGHT/2);
	}
	
	// draw x-y axis 
	DRAW_X_Y(YELLOW);

	for (int16_t i=0; i<N; i++){
		LCD_DrawLine(i, 40, i, magnitudes[i], RED);		// freq spectrum
		LCD_DrawPoint(i, x[i].real, WHITE);						// original signal (only white dots when not invers)
	}
}