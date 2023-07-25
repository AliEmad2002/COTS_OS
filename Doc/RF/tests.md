**Test #1: Observe RF-receiver's output when the RF-transmitter is idle (not transmitting).**
  * Low amplitude nose is outputted.
  * Image 1: RF-receiver's output on the spectrum when RF-transmitter is idle:
    ![1](https://github.com/AliEmad2002/COTS_OS/assets/99054912/8911cb13-ad09-4251-ac4d-7ab22f279f02)


**Test #2: Input various bit-rates to the RF-transmitter and observe RF-receiver's output.**
  * Best usable output is when input bit-rate is in the range: 100Hz - 4kHz
  * Image 2: RF-receiver's output on the spectrum when RF-transmitter gets an input signal of 2kHz, 50% duty cycle:
    ![2](https://github.com/AliEmad2002/COTS_OS/assets/99054912/5ae76c60-6994-4433-94de-6c02f42a7071)

  * Image 3: RF-receiver's output on the oscilloscope (Blue channel) when RF-transmitter gets an input signal of 2kHz, 50% duty cycle (Yelloww channel):
    ![3](https://github.com/AliEmad2002/COTS_OS/assets/99054912/829296bf-f143-450e-a392-702c3a531843)

**Test #3: Input 2kbps signal to the RF-transmitter, take RF-receiver's output on LPF of Fc = 3kHz and observe.**
  * Image 4: RF-receiver's output (low pass filtered @Fc = 3kHz) on the spectrum when RF-transmitter gets an input signal of 2kHz, 50% duty cycle:
    ![4](https://github.com/AliEmad2002/COTS_OS/assets/99054912/ebf444ac-9582-431d-bea4-3025b5b9dc04)

  * Image 5: RF-receiver's output (low pass filtered @Fc = 3kHz) on the oscilloscope (Blue channel) when RF-transmitter gets an input signal of 2kHz, 50% duty cycle (Yelloww channel):
    ![5](https://github.com/AliEmad2002/COTS_OS/assets/99054912/3b4731c0-e52e-4811-a8aa-381689abe562)
