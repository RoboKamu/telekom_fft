Works, but not higher than around 24 kHz.
</br>
TODO:
- [x] Sample
- [x] Transform (FFT)
- [x] Visualize on LCD
- [x] Center graph (get max and min)
- [x] make more effective (optional)
- [x] Interrupt for ADC instead?
</br>
Changes to the normal fast version:
- fixed calculations on visualize_fft function
- not scaling during sampling anymore, only when displaying FFT and IFFT
- freq spectrum in log scale (times 10 for visibility, can change)
- comments, data structures and variable names
</br>
Professor observation: Dont clear. Only remove the specifik LCD values that are on. The LCD is what takes a lot of time in this program. 
</br>
Note: Using Gd32v linux toolchain.
