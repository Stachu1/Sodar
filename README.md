# Sodar: FPGA-Based Ultrasonic Phased Array

**Sodar** is a directional ultrasonic detection and ranging system developed as a semester project at my university. It uses a phased array of 10mm ultrasonic transducers to emit and steer ultrasonic bursts, and captures echoes using a receiving transducer. The system is built around a Xilinx ZedBoard FPGA and enables directional scanning and object detection similar to radar/lidar but using sound.

## 🔧 System Overview

- **Transmitter**: Phased array of 19 ultrasonic transducers controlled by the FPGA using square-wave PWM signals.
- **Beam Steering**: Achieved via adjustable phase shifts between the transducers.
- **Receiver**: Single transducer feeding into ZedBoard's onboard XADC.
- **Signal Processing**: Echo data is low-pass filtered using a custom FIR filter and then transmitted via UART.
- **Visualization**: A Python script receives UART data and generates a real-time heatmap of distance vs angle.

---

## 📦 Components

- **Hardware**:
  - Xilinx ZedBoard FPGA
  - 10mm ultrasonic transducers
  - Dual full-bridge drivers for signal amplification
  - Custom PCB for transducer array
- **Firmware (C)**:
  - Configures PWM for 40kHz generation
  - Sets beam angle using sinusoidal phase offset
  - Captures and filters ADC echo data
  - UART transmission of polar data
- **Host Software (Python)**:
  - Real-time UART parser
  - Dynamic heatmap of angle vs distance using Matplotlib

---

## ⚙️ How It Works

1. **Phase-Controlled Transmission**:
   - FPGA drives transducers with square wave bursts.
   - By applying phase shifts across elements, the beam is steered from -20° to +20° (FOV = 40°).

2. **Echo Sampling**:
   - The echo from each direction is captured via the receiving transducer.
   - Data is averaged over multiple pulses to reduce noise.
   - A 30-tap FIR low-pass filter is applied to the ADC buffer.

3. **Data Encoding & UART Transmission**:
   - Echo strength is normalized and downsampled.
   - Each directional slice is sent as a single UART line as a string of hex characters

4. **Visualization**:
   - Python script reads serial data and updates a real-time polar heatmap.

---

## 🚀 Running the System

1. **Build and flash the FPGA + C firmware** onto the ZedBoard.
2. **Connect UART** from ZedBoard to your laptop (e.g. `COM5` on Windows).
3. **Run the plotting script**:
   ```bash
   python plot.py
   ```
4. Watch the ultrasonic beam scan left-to-right and back, revealing object positions in real time.

---

## 📈 Data Format

- **Scan mode (SRC)**:
  - Data line: `[angle_index][sample0][sample1]...[sampleN]` (hex)
  - 12-bit samples, 2 bytes each

- **Track mode (TRK)**:
  - Data line: `[0x80 | angle_index][echo_peak_index]` (hex)
  - Used for object locking (not always active)

---

## 📷 Example Output
<img width="1200" height="700" alt="image" src="https://github.com/user-attachments/assets/a53b5ded-5262-489b-be20-f020d4a3c184" />
<img width="1300" height="700" alt="image" src="https://github.com/user-attachments/assets/a7525abe-64a4-461b-9082-f24fc1468b64" />

- Realtime distance-vs-angle heatmap:
  - X-axis: Angle (°)
  - Y-axis: Distance (cm)
  - Color intensity: Echo strength (V)

---

## 📷 Example Setup
<img width="450" height="350" alt="image" src="https://github.com/user-attachments/assets/33487c6b-bd8f-47b7-9fee-60a76ac81a98" />
<img width="1200" height="700" alt="image" src="https://github.com/user-attachments/assets/1d4348c7-6754-4df5-9622-e70634fdc0cc" />


---

## 🛠️ Configuration

Set these parameters in the firmware and script:

| Parameter       | Value     | Description                              |
|----------------|-----------|------------------------------------------|
| PWM Frequency  | 40 kHz    | Ultrasonic burst frequency               |
| FIR Taps       | 30        | Low-pass filter taps                     |
| ADC Samples    | 3000      | Echo samples per beam angle              |
| Sample Divider | 50        | Downsampling factor before UART          |
| Field of View  | 40°       | Total sweep range (-20° to +20°)         |
| UART Baud Rate | 230400    | Communication speed with host PC         |

---

## 🔍 Applications

- Short-range object detection & ranging
- Obstacle detection
- Robotic navigation
- Beamforming and DSP experimentation

