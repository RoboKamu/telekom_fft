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

void visualize_fft(Complex *x, int16_t N);
void visualize_ifft(Complex *x, int16_t N, int8_t sf);

int main(void){
	int16_t adcr = 0, tmpr = 0;
	static int8_t sf_adc_lcd = 0b00000101;		// scaling factor Q8: 80/4096 => 0,01953125 << 8

	Complex samples[ARRAY_SIZE] = { {0, 0} };		 	// initialize all samples to 0 first
	Complex* pSamples = samples;									// pointer to samples array
	int16_t count = ARRAY_SIZE;						 				// start with last index for backfilling array


  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  ADC3powerUpInit(0);                       // Initialize ADC0, Ch3 
  LCD_Clear(BLACK);

  while (1) {                             
		do{
			if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
				adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4096
				pSamples[(--count)].real = adcr;            // ......backfill sample value to array
				adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
			}
			adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																		ADC_REGULAR_CHANNEL);
		}while(count>0);					// do this until 128 samples
	
		// test fft visuals
		fft(samples, ARRAY_SIZE); 
		visualize_fft(samples, ARRAY_SIZE);
		delay_1ms(500);
		
		// test inverse visuals
		inverse_fft(samples, ARRAY_SIZE);
		visualize_ifft(samples, ARRAY_SIZE, sf_adc_lcd);
		delay_1ms(500);
		
		// clear counter and array values
		for (uint16_t i=0; i<ARRAY_SIZE; i++) {
			pSamples[i].real = 0;
			pSamples[i].imag = 0;
		}
		count = ARRAY_SIZE;
	}
}

void visualize_fft(Complex *x, int16_t N){				
	int32_t magnitudes[N]; 
	int32_t max_magnitude = 0;
	int16_t tmpr = 0;
	
	for (uint8_t i = 0; i<N; i++){
		magnitudes[i] = cordic_hypotenuse(x[i].real, x[i].imag);										// calc magnitude
		if (magnitudes[i] > max_magnitude){
			max_magnitude = magnitudes[i];
		}
	}

  // prevent scaling issues
	max_magnitude = max_magnitude == 0 ? 1 : max_magnitude; 		 

	// scale the magnitudes according to the max for the LCD
	for (uint8_t i = 0; i<N; i++){
		magnitudes[i] = (magnitudes[i] << CORDIC_MATH_FRACTION_BITS) / max_magnitude;			//...Devide by maximum value
		magnitudes[i] = (magnitudes[i] * LCD_HEIGHT) >> CORDIC_MATH_FRACTION_BITS;		//....multiply with desired height (half LCD)
		magnitudes[i] = magnitudes[i] > LCD_HEIGHT ? LCD_HEIGHT : magnitudes[i];				  //....limit magnitude
		//magnitudes[i] = floor_log2_32(1 + magnitudes[i]) * 10; 														//....log scaling, modified for visibility
	}
	
	// draw x-y axes 		LCD_DrawPoint(i, (40+dc) - x[i].real, BLUE);		// draw IFFT

	LCD_Clear(BLACK);
	DRAW_X_Y(WHITE);

	for (int16_t i=0; i<N; i++){
		LCD_DrawLine(i+1, 40, i+1, 40-magnitudes[i], RED);				// freq spectrum centered around x axes  
	}
	LCD_Wait_On_Queue();					// wait until LCD finished drawing
}

void visualize_ifft(Complex *x, int16_t N, int8_t sf){
	// calculate the dc offset
	int16_t dc = 0, tmpr=0;
	for (uint8_t i = 0; i<N; i++){
		tmpr = (int16_t)(x[i].real * sf) >> 8;	  // scale adc value from 0-4096 to 0-LCD_HEIGHT
		x[i].real = tmpr;
		dc += x[i].real;						
	}
	dc /= N;						// dc offset (mean value of array)
	
	LCD_Clear(BLACK);
	DRAW_X_Y(WHITE);
	for (uint8_t i=0; i<N; i++){
		LCD_DrawPoint(i, (40+dc) - x[i].real, BLUE);		// draw IFFT
	}
	LCD_Wait_On_Queue();					// wait until LCD finished drawing
}