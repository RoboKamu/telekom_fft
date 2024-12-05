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
		LCD_DrawLine(0, 0, 0, LCD_HEIGHT, COLOR); \
		LCD_DrawLine(0, 40, LCD_WIDTH, 40, COLOR);

void shiftArray(Complex arr[], uint8_t n, int dc);
void visualize_fft(Complex *x, int16_t N);
void visualize_ifft(Complex *x, int16_t N);

int main(void){
  uint8_t len = 1;								 
  int idle =0;//int dac=0;
  int16_t adcr, tmpr = 40;
	int DC;
	int max_val;
	int min_val;
	static int g = 0b00000101;							// 80/4096 = 0,01953125 << 8

	Complex samples[FFT_WIDTH] = { {0, 0} }; 				// initialize all samples to 0 first

  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  LCD_Clear(BLACK);
  ADC3powerUpInit(1);                     // Initialize ADC0, Ch3 & Ch16

	// draw x-y axes
	DRAW_X_Y(WHITE);
	
  while (1) {                             

		delay_until_1us(100);							// can sample around 5 Hz

		idle++;
	  LCD_WR_Queue();                   // Manage LCD com queue!

		if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
			adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4095
			tmpr = (int16_t)(adcr * g) >> 8; 					// scale adc value in range of 0-LCD_HEIGHT with divsion 4096
			LCD_DrawPoint_big(4, tmpr, YELLOW);         // display value on LCD
			samples[1].real = tmpr;                     // save the value
			adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
		} 

		// shifting lcd dots
		shiftArray(samples, len, DC);
		if (len <= FFT_WIDTH) len++;

		LCD_DrawPoint_big(4, tmpr, BLACK);      // remove trail before reading next value
		adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																	ADC_REGULAR_CHANNEL);
  
		if (!(idle%500)){										// every 6 seconds..
			// test fft visuals
			LCD_Clear(BLACK);
			fft(samples, len-1); 
			visualize_fft(samples, len-1);
			delay_1ms(3000);
			while(!delay_finished()); 		// display for 2 seconds
			
			// test inverse visuals
			LCD_Clear(BLACK);
			inverse_fft(samples, len-1);
			visualize_ifft(samples, len-1);
			delay_1ms(2000);
			while(!delay_finished()); 		// display for 2 seconds
			
			LCD_Clear(BLACK);
			DRAW_X_Y(WHITE);							// draw the original axes again
		}

			
		while(!delay_finished()); 		// wait until iteration done


	}
}

void shiftArray(Complex arr[], uint8_t n, int dc){
	DRAW_X_Y(WHITE);
	for (uint8_t i=n-1; i>0; i--){																	
		LCD_DrawPoint(i, 40-arr[i].real, BLACK);   											 // remove trail
		arr[i+1] = arr[i];		                       									 // shift 1 step
    if (i < FFT_WIDTH) LCD_DrawPoint(i+1, 40-arr[i+1].real, BLUE);    // display moved value
  }
}

void visualize_fft(Complex *x, int16_t N){				//where N has to be 2^k
	int32_t magnitudes[N]; 
	int32_t max_magnitude = 0;
	 
	for (int16_t i = 0; i<N; i++){
		magnitudes[i] = cordic_hypotenuse(x[i].real, x[i].imag);										// calc magnitude
		magnitudes[i] = magnitudes[i] > LCD_HEIGHT ? LCD_HEIGHT : magnitudes[i];		// limit magnitude
		if (magnitudes[i] > max_magnitude){
			max_magnitude = magnitudes[i];
		}
	}

  // prevent scaling issues
	max_magnitude = max_magnitude == 0 ? 1 : max_magnitude; 		 

	// scale the magnitudes according to the max for the LCD
	for (int16_t i = 0; i<N; i++){
		// fixed division with max magnitude
		magnitudes[i] = (magnitudes[i] << CORDIC_MATH_FRACTION_BITS) / max_magnitude;
		// fixed multiplication with half of LCD height
		magnitudes[i] = (magnitudes[i] >> CORDIC_MATH_FRACTION_BITS) * (LCD_HEIGHT/2);
		//magnitudes[i] = floor_log2_32(magnitudes[i]);
	}
	
	// draw x-y axes 
	DRAW_X_Y(WHITE);

	for (int16_t i=0; i<N; i++){
		LCD_DrawLine(i+1, 40, i+1, 40-magnitudes[i], RED);		// freq spectrum
	}
}

void visualize_ifft(Complex *x, int16_t N){
	DRAW_X_Y(WHITE);
	for (int16_t i=0; i<N; i++){
		LCD_DrawPoint(i, x[i].real, YELLOW);								  // inverse FFT of input signal
	}
}