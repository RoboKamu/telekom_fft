#include "gd32vf103.h"
#include "lcd.h"
#include "adc.h"
#include "delay.h"
#include "fft.h"
#include "cordic-math.h"

#define LCD_HEIGHT 80
#define LCD_HEIGHT_HALF 40
#define LCD_WIDTH 160
#define ARRAY_SIZE 128			// FFT needs 2^k samples, displaying IFFT limited by LCD_WIDTH 

#define DRAW_X_Y(COLOR) \
		LCD_DrawLine(0, 0, 0, LCD_HEIGHT, COLOR); \
		LCD_DrawLine(0, 40, LCD_WIDTH, 40, COLOR);

void shiftArray(Complex arr[], uint8_t n, int dc);
void visualize_fft(Complex *x, int16_t N);
void visualize_ifft(Complex *x, int16_t N);

int main(void){
	int16_t adcr = 0, tmpr = 0;
	static int8_t sf_adc_lcd = 0b00000101;		// scaling factor Q8: 80/4096 => 0,01953125 << 8

	Complex samples[ARRAY_SIZE] = { {0, 0} };		 	// initialize all samples to 0 first
	Complex* pSamples = samples;									// pointer to samples array
	int16_t count = ARRAY_SIZE;						 			// start with last index for backfilling array


  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  ADC3powerUpInit(1);                       // Initialize ADC0, Ch3 & Ch16
  LCD_Clear(BLACK);

  while (1) {                             
		do{
			//delay_until_1us(500); // using 500 us when working without function generator
			delay_until_1us(10);

			if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
				adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4096
				tmpr = (int16_t)(adcr * sf_adc_lcd) >> 8;	  // scale adc value in range of 0-LCD_HEIGHT with divsion 4096
				pSamples[(--count)].real = tmpr;            // buffer error after 2 iterations?
				adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
			}

			adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																		ADC_REGULAR_CHANNEL);

			while(!delay_finished()); 		// wait until iteration done
		}while(count>0);					// do this until 128 samples
	
		// test fft visuals
		fft(samples, ARRAY_SIZE); 
		visualize_fft(samples, ARRAY_SIZE);
		delay_1ms(1000);
		
		// test inverse visuals
		inverse_fft(samples, ARRAY_SIZE);
		visualize_ifft(samples, ARRAY_SIZE);
		delay_1ms(1000);
		
		// clear LCD, counter and array values
		LCD_Clear(BLACK);
		for (int16_t i=0; i<ARRAY_SIZE; i++) {
			pSamples[i].real = 0;
			pSamples[i].imag = 0;
		}
		count = ARRAY_SIZE;
		// while (count < ARRAY_SIZE) pSamples[(count++)].real = 0;
	}
}

void visualize_fft(Complex *x, int16_t N){				
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
		magnitudes[i] = (magnitudes[i] << CORDIC_MATH_FRACTION_BITS) / max_magnitude;
		magnitudes[i] = (magnitudes[i] * LCD_HEIGHT_HALF) >> CORDIC_MATH_FRACTION_BITS;
		//magnitudes[i] = floor_log2_32(1 + magnitudes[i]);
	}
	
	// draw x-y axes 
	LCD_Clear(BLACK);
	DRAW_X_Y(WHITE);

	for (int16_t i=0; i<N; i++){
		LCD_DrawLine(i+1, 40, i+1, 40-magnitudes[i], RED);				// freq spectrum centered around x axes  
	}
}

void visualize_ifft(Complex *x, int16_t N){
	// calculate the dc offset
	int16_t dc = 0;
	for (int16_t i = 0; i<N; i++){
		dc += x[i].real;						
	}
	dc /= N;			
	
	LCD_Clear(BLACK);
	DRAW_X_Y(WHITE);
	for (int16_t i=0; i<N; i++){
		LCD_DrawPoint(i, (40+dc) - x[i].real, YELLOW);
	}
	LCD_Wait_On_Queue(); 		// wait until LCD finished drawing
}