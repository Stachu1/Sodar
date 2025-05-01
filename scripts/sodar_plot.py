import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# === CONFIG ===
SERIAL_PORT = "COM5"
BAUD_RATE = 230400
SAMPLES = 100
ANGLE_MIN = -30
ANGLE_MAX = 30

# === INIT SERIAL ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# === INIT DATA ARRAY ===
angle_range = ANGLE_MAX - ANGLE_MIN + 1
heatmap_zeros = np.zeros((SAMPLES, angle_range))  # (samples, angles)
heatmap = heatmap_zeros.copy()

# === PLOTTING SETUP ===
fig, ax = plt.subplots()
im = ax.imshow(
    heatmap,
    aspect='auto',
    origin='lower',
    interpolation='nearest',
    extent=[ANGLE_MIN - 0.5, ANGLE_MAX + 0.5, 0, SAMPLES],
    cmap='viridis', # Color map
    vmin=5,         # Min voltage
    vmax=40         # Max voltage
)

ax.set_title("Mode: SRC")
ax.set_xlabel("Angle (°)")
ax.set_ylabel("Sample Index")

# === UPDATE FUNCTION ===
def update(frame):
    global heatmap
    try:
        # Read one full frame
        raw_line = ser.readline().strip()
        if raw_line:
            # if len(raw_line) % 4 == 2:
                
            mode = 0x80 & int(raw_line[:2], 16)
            if mode:
                ax.set_title("Mode: TRC")
                angle_index = 0x3F & int(raw_line[:2], 16)
                distance = int(raw_line[2:6], 16)
                data = [0] * SAMPLES
                data[distance] = 1000
                if 0 <= angle_index < angle_range:
                    heatmap = heatmap_zeros.copy()
                    heatmap[:, angle_index] = data

            else:
                ax.set_title("Mode: SRC")

                angle_index = 0x3F & int(raw_line[:2], 16)
                hex_data = raw_line[2:]

                decoded_data = []
                for val in [hex_data[i:i+4] for i in range(0, len(hex_data), 4)]:
                    decoded_data.append(int(val, 16) / 65.535)

                if 0 <= angle_index < angle_range:
                    heatmap[:, angle_index] = decoded_data

        # Update the image
        im.set_data(heatmap)
    except Exception as e:
        print(f"Error: {e}")

# === ANIMATION LOOP ===
ani = FuncAnimation(fig, update, interval=1, cache_frame_data=False)
plt.tight_layout()
plt.show()
