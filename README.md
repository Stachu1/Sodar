# Sodar: FPGA-Based Ultrasonic Phased Array

Sodar is a semester project exploring directional ultrasonic object detection using a custom 19-element phased array. The system combines a custom PCB, a Digilent Zedboard with Xilinx Zynq-7000 SoC, VHDL-based signal generation, C firmware running on the Zynq processing system, and a Python visualization tool.

The project was developed as a concept and experimental prototype rather than a finished sensing system. The main goals were to learn the Zynq-7000 architecture and its peripherals, investigate phased-array beam steering, and build a complete hardware-to-software signal path.

<img width="600" height="467" alt="image" src="https://github.com/user-attachments/assets/33487c6b-bd8f-47b7-9fee-60a76ac81a98" />

## System Overview

The system consists of four main parts:

- **19-element transmitter array** using 40 kHz piezoelectric ultrasonic transducers
- **Custom PCB** with 19 full H-bridge driver channels based on TC78H621FNG motor-driver ICs for driving the transducers
- **Digilent Zedboard (Zynq-7000 SoC)**, with VHDL logic in the programmable logic (PL) generating the phase-shifted transmit signals and C firmware running on the processing system (PS) handling acquisition and signal processing
- **Python host software** for receiving processed data over UART and visualizing the measured echo strength as a function of angle and distance

The system was able to detect a small object at distances of up to approximately 4 m when transmitting straight ahead. When steering the beam towards ±20°, the received signal became substantially weaker and more affected by noise, limiting reliable detection of the test object to approximately 1 m.

<img width="700" height="350" alt="image" src="https://github.com/user-attachments/assets/d7832ddc-2d67-4c5d-9f8f-0a5cf52f5f8d" />

## Transducer Array

The array contains 19 identical 40 kHz ultrasonic transducers arranged in two staggered rows: one row contains 10 transducers and the other contains 9.

<img width="612" height="494" alt="image" src="https://github.com/user-attachments/assets/b8ac0e1a-416e-424d-a617-8817a9afe851" />


The physical spacing between adjacent transducers along each row is 10 mm. The second row is shifted by 5 mm, creating a staggered arrangement where the projected spacing between elements along the left-right steering axis is approximately 5 mm.

At 40 kHz, the acoustic wavelength in air is approximately 8.6 mm at room temperature, making the projected element spacing close to λ/2. The arrangement was chosen for horizontal beam steering while keeping the projected element spacing small enough to reduce unwanted grating lobes.

The array was designed for horizontal steering only; there is no intended vertical beam steering.

A 3D radiation pattern simulation was used to visualize the array's predicted transmission characteristics and beam steering performance.

<img width="600" height="467" alt="image" src="https://github.com/user-attachments/assets/0d26e7d8-5d50-4039-a6fd-30f3ce6238db" />


## Hardware

The custom PCB contains the 19 ultrasonic transducers together with the electronics required to drive them.

Each transducer has an independent H-bridge output provided by a motor-driver IC (TC78H621FNG). The Zynq PL provides the control signals required for each channel, including an enable signal and drive-direction signals.

The board was designed, assembled, and soldered as part of the project.

Main hardware:

- Digilent ZedBoard with Xilinx Zynq-7000 SoC
- 19 - 40 kHz ultrasonic transducers
- 19 - full H-bridge driver channels
- Custom PCB with transducers and driver ICs (TC78H621FNG)
- Zynq-7000 internal XADC used for receiving the ultrasonic signal

## FPGA and Firmware

The Zynq-7000 is divided into two parts in the project:

- **Programmable Logic (PL):** VHDL implementation of the phase-controlled transmit signal generation
- **Processing System (PS):** C firmware running on the Zynq CPU for ADC acquisition, signal processing, beam control, and UART communication

### Phase-Controlled Transmit Generation

The PL contains a reusable PWM generator with programmable:

- period
- duty cycle
- phase shift

The top-level design instantiates 19 generators, with the phase of each channel calculated from the requested beam angle.

The phase of each channel is calculated from a common base phase and the channel index. This allows the relative phase between the 19 transmitters to be changed from the PS firmware without modifying the FPGA design.

The system generates 40 kHz square-wave bursts with phase offsets corresponding to the requested steering angle. The steering range used during testing was approximately -20° to +20°.

### Beam Steering

The array is steered horizontally by introducing a constant phase difference between adjacent transducers. Since the projected element spacing in the steering direction is approximately 5 mm, the required phase shift can be calculated from the wavelength of the 40 kHz ultrasonic signal.

Assuming a speed of sound of 343 m/s:

$$
\lambda = \frac{343\frac{m}{s}}{40\text{ kHz}} = 8.575\text{ mm}
$$

The phase difference between adjacent elements is then:

$$
\Delta\phi = 2\pi\frac{d}{\lambda}\sin(\theta)
$$

where $d = 5$ mm is the projected element spacing and $\theta$ is the desired steering angle.

This gives:

$$
\Delta\phi \approx 3.664\sin(\theta)\text{ rad}
$$

or, in degrees:

$$
\Delta\phi \approx 209.9\sin(\theta)^\circ
$$

The firmware therefore uses a constant of 209.9 to convert the requested steering angle into the phase difference between adjacent elements.

The calculated phase difference is converted into FPGA clock cycles and provided to the VHDL signal-generation logic. The phase of each element is then scaled according to its position in the array, producing the progressive phase shift required for beam steering.

The steering range used during testing was approximately -20° to +20°.

I'd make this section a little more detailed, because the burst-length/multiple-pulse experiments show useful engineering tradeoffs without overselling the DSP:

## Echo Acquisition and Signal Processing

The receiving transducer is sampled using the Zynq-7000's internal XADC at approximately 500 kHz.

The implemented processing pipeline:

1. XADC acquisition
2. DC offset removal
3. Absolute value
4. FIR low-pass filtering
5. Echo peak detection
6. Distance estimation

During testing, different transmit burst lengths were investigated to find a good balance between transmitted energy, echo detectability, and range resolution. In our experiments, a burst length of approximately 7 periods of the 40 kHz carrier gave the best results.

We also experimented with transmitting multiple bursts consecutively and summing the corresponding received waveforms. This provided some noise rejection by averaging out variations between individual measurements, but increased the acquisition time for each beam angle. As a result, using more bursts improved signal quality at the cost of a lower scanning refresh rate.

### DC Offset Removal

The average value of the acquired ADC buffer is calculated and subtracted from each sample. This centres the waveform approximately around zero before further processing.

### Rectification and FIR Filtering

The centred signal is passed through an absolute-value operation and then through a 30-tap FIR low-pass filter. This produces a smoothed representation of the received signal energy, making the location of the echo easier to identify.

The strongest point in the filtered signal is then used as the detected echo position.

### DC Offset Removal

The average value of the acquired ADC buffer is calculated and subtracted from each sample. This centres the waveform approximately around zero before further processing.

### Rectification and FIR Filtering

The centred signal is passed through an absolute-value operation and then through a 30-tap FIR low-pass filter. This produces a smoothed representation of the received signal energy, making the location of the echo easier to identify.

The strongest point in the filtered signal is then used as the detected echo position.

### Distance Estimation

Distance is calculated from the measured round-trip travel time using the standard ultrasonic time-of-flight relationship:

`distance = speed_of_sound × time / 2`

The speed of sound is configurable in the host-side Python script. No temperature compensation was implemented.

<img width="1200" height="700" alt="image" src="https://github.com/user-attachments/assets/a53b5ded-5262-489b-be20-f020d4a3c184" />

### Matched Filtering

Matched filtering was also investigated experimentally during development, including testing against the transmitted 40 kHz burst. It was not included in the final implemented signal-processing pipeline.

## Scanning and Object Detection

The system can scan through a range of beam angles and transmit an ultrasonic burst at each angle.

For each angle, the received signal is sampled and processed before the resulting data is sent to the host computer over UART.

A small spray-paint can placed on a flat surface in a laboratory was used as the main test object.

In straight-ahead operation, the system was able to detect the object at approximately 4 m. At larger steering angles, the received signal became weaker and noisier, with reliable detection of the same object reaching approximately 1 m around the ±20° steering range.

These results were obtained in a normal laboratory environment rather than an acoustically controlled room, so reflections and background noise were present during testing.

<img width="1200" height="700" alt="image" src="https://github.com/user-attachments/assets/1d4348c7-6754-4df5-9622-e70634fdc0cc" />

## Tracking Mode

An experimental tracking mode was also implemented in the PS firmware.

Instead of performing a full angular scan, the system compares the echo strength at neighbouring beam angles and moves the beam towards the stronger response.

The implementation was experimental and not sufficiently reliable to be considered a finished tracking system. It is included primarily as an experiment demonstrating how the beam-steering mechanism could be used for closed-loop object tracking.

## Host Software

The repository contains a Python script for receiving the processed data over UART and displaying it as a real-time distance-versus-angle heatmap.

The visualization represents:

- **X-axis:** beam angle
- **Y-axis:** distance
- **Colour:** measured echo strength

The Python visualization can also be used to inspect received data and experiment with parameters such as the assumed speed of sound.

## Data Format

In scan mode, the firmware transmits the processed samples for each beam angle over UART.

The implementation uses a compact hexadecimal representation containing the angle index followed by the sampled data.

The Python script contains the corresponding serial parsing and visualization code.

## Project Structure

The repository is organized into the following main components:

- `firmware/` - C firmware running on the Zynq processing system
- `hardware/` - PCB and schematic files
- `scripts/` - Python visualization and host-side tools
- `vhdl/` - VHDL source for the programmable logic
- `simulations/` - Beam-pattern simulations
- `README.md` - Project documentation

The Vivado project itself is not intended to be directly reproducible from the repository. The VHDL source is provided separately so that the hardware logic can be inspected without including the generated Vivado project files.

## Results and Limitations

The prototype demonstrated the basic principle of electronically steering an ultrasonic phased array and detecting echoes from a physical object.

The main experimental results were:

| Configuration | Approximate result |
|---|---:|
| Straight-ahead detection | Up to ~4 m |
| Beam steering | -20° to +20° |
| Detection at large steering angles | Up to ~1 m |
| Transmit frequency | 40 kHz |
| Number of transmit elements | 19 |
| ADC sampling rate | ~500 kHz |
| FIR filter | 30 taps |

The system was a semester project and has several limitations. In particular, the received signal became significantly weaker as the beam was steered away from the centre, limiting the useful detection range. The tracking mode was also experimental rather than a robust tracking system.

The project was primarily intended as a proof-of-concept and learning exercise rather than as a finished ultrasonic sensing platform.

## What I Learned

The project provided practical experience with several parts of a mixed hardware/software system:

- Designing and assembling a relatively high-channel-count custom PCB
- Driving 19 ultrasonic transducers using independent H-bridge channels
- Developing VHDL logic for precisely phase-shifted signal generation
- Working with the Zynq-7000 PS/PL architecture
- Acquiring signals using the Zynq's internal XADC
- Implementing basic real-time signal processing in C
- Communicating processed measurements to a PC over UART
- Simulating and experimentally testing phased-array beam patterns
- Investigating the practical limitations of ultrasonic beam steering

The project was developed as a second-year engineering project, and the prototype left several areas open for further development, including improved analogue signal conditioning, more robust echo detection, better calibration of the array, and improved performance at larger steering angles. A natural next step for the signal processing would be to transmit a frequency-modulated chirp and use matched filtering on the received signal, which could improve range resolution and provide better noise immunity compared with the simple burst and peak-detection approach used in the prototype.
