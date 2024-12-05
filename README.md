Works, but not higher than around 5 Hz.
</br>
TODO:
- [x] Sample
- [x] Transform (FFT)
- [x] Visualize on LCD
- [ ] Center graph (get max and min)
- [ ] make more effective (optional)
</br>
For effectiveness: use delay_util and sample as fast as you can. Then when delay finished, fft and visualize. Can use get_time_value to see how effective parts of code is.
</br>
Note: Using Gd32v linux toolchain. Change the makefile for the one in windows for it to work.
