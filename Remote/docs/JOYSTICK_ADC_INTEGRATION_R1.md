# Joystick ADC interface — integration draft

The firmware now has a library-free ADS1115 single-shot reader for four joystick channels, assigned AIN0..AIN3 at provisional I2C address **0x4A**.

- Uses 860 SPS, ±4.096 V full-scale and signed 16-bit conversion.
- Converts positive ADC counts to the existing 0..4095 calibration scale.
- Each conversion polls OS-ready for at most 8 ms; failures reject the whole sample.
- Negative ADS readings are rejected.
- **Do not connect joystick pots until verify their supply voltage and wiper range.** ADS1115 input must stay within its supply rails; power the ADC from a safe 3.3V rail and use compatible analog voltage.
- Confirm address strap, I2C pullups, ADC module supply and board GPIO8/9 crosswalk before installation.
- This implementation is not yet hardware-tested.

## Provisional channel map
AIN0 left X; AIN1 left Y; AIN2 right X; AIN3 right Y.
