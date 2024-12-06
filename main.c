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
void visualize_ifft(Complex *x, int16_t N, int mean);

/************************************
Differences:
- not show continious reading 

Structure:
- Sample as fast as possbile 
- FFT and visualise for x seconds
- IFFT and visualise for X seconds
- Start samping again
***********************************/

int main(void){
  uint8_t len = 0;								 				// max value will be 128
  int16_t adcr = 0, tmpr = 40;
	uint16_t mean = 0;														// calc. DC component with average value
	static int g = 0b00000101;							// 80/4096 => 0,01953125 << 8

	Complex samples[FFT_WIDTH] = { {0, 0} }; 	// initialize all samples to 0 first

  Lcd_SetType(LCD_INVERTED);                // or use LCD_INVERTED!
  Lcd_Init();
  ADC3powerUpInit(1);                       // Initialize ADC0, Ch3 & Ch16
  LCD_Clear(BLACK);

	//int current_time =0, stop_time=0; 	// for get_timer_value() function
  while (1) {                             
		do{
			delay_until_1us(25);

			if (adc_flag_get(ADC0,ADC_FLAG_EOC)==SET) {   // ...ADC done?
				adcr = adc_regular_data_read(ADC0);         // ......get data   0<->4095
				tmpr = (int16_t)(adcr * g) >> 8; 					  // scale adc value in range of 0-LCD_HEIGHT with divsion 4096
				//samples[len-1].real = tmpr;                     // buffer error after 2 iterations?
				samples[0].real = tmpr;
				shiftArray(samples, len, mean);
				adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ......clear IF
				len++;
			}

			adc_software_trigger_enable(ADC0,  //Trigger another ADC conversion!
																		ADC_REGULAR_CHANNEL);

			while(!delay_finished()); 		// wait until iteration done
		} while(len<128);						// do this until 128 samples
	
		for (int i = 0; i < len; i++){
			mean += i; 
		}
		mean = mean / len;
		
		if (len==128){							// <-- unnecesary condition (?)
			// test fft visuals
			LCD_Clear(BLACK);
			fft(samples, len); 
			visualize_fft(samples, len);
			delay_1ms(1000);
			
			// test inverse visuals
			LCD_Clear(BLACK);
			inverse_fft(samples, len);
			visualize_ifft(samples, len, mean);
			delay_1ms(1000);
			
			LCD_Clear(BLACK);
			for (int16_t i = 0; i < len; i++){
				samples[i].real = 0; 
			}
			len=1;
		}
	}
}

void shiftArray(Complex arr[], uint8_t n, int dc){
	for (int8_t i=n-1; i>=0; i--){																	
		arr[i+1] = arr[i];		                       	// shift 1 step
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
		// fixed division with max magnitude
		magnitudes[i] = (magnitudes[i] << CORDIC_MATH_FRACTION_BITS) / max_magnitude;
		// fixed multiplication with half of LCD height
		magnitudes[i] = (magnitudes[i] >> CORDIC_MATH_FRACTION_BITS) * (LCD_HEIGHT/2);
		//magnitudes[i] = floor_log2_32(magnitudes[i]); 		// log scale magnitude
	}
	
	// draw x-y axes 
	DRAW_X_Y(WHITE);

	for (int16_t i=0; i<N; i++){
		LCD_DrawLine(i+1, 40, i+1, 40-magnitudes[i], RED);				// freq spectrum centered around x axes  
	}
}

void visualize_ifft(Complex *x, int16_t N, int DC){
	DRAW_X_Y(WHITE);
	for (int16_t i=0; i<N; i++){
		LCD_DrawPoint(i, DC-(x[i].real), YELLOW);								  // inverse FFT of input signal
	}
}