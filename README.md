# waveform_gen_uc

The goal is to create an arbitrary waveform generator, 
with ability to generate audio-frequency signals
and decent 1 Hz - 100 kHz waveforms. The project consists
of two parts: desktop application and embedded application.

This is embedded app repo.

Embedded app (under development):
Developed for STM32L152RE (with Nucleo board), uses internal DAC
to generate waveform. Includes LCD display for controlling the device.
Able to generate basic signals without desktop app connected.
Transmission via UART, controlled with encoder. 


Desktop app:
Built in Qt framework. Creates waveforms and transmits them
to uC.
