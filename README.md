Works, but not higher than around 26 kHz before aliasing
</br>
TODO:
- [x] Sample
- [x] Transform (FFT)
- [x] Visualize on LCD
- [x] Center graph (get max and min)
- [x] make more effective (optional)
- [x] Interrupt for ADC instead?
- [ ] DMA to trigger interrupt

Changes to the normal fast version:
- fixed calculations on visualize_fft function
- not scaling during sampling anymore, only when displaying FFT and IFFT
- freq spectrum in log scale (times 10 for visibility, can change)
- comments, data structures and variable names

Professor observation: Dont clear. Only remove the specifik LCD values that are on. The LCD is what takes a lot of time in this program. 
</br>
</br>
Note: Using Gd32v linux toolchain.
</br>
</br>
CREDITS MATH LIBRARY
[Cordic-Math library](https://github.com/Max-Gulda/Cordic-Math/tree/main)
