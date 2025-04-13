import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# === CONFIG ===
SERIAL_PORT = "COM5"
BAUD_RATE = 115200
INTENSITY_LENGTH = 500
ANGLE_MIN = -30
ANGLE_MAX = 30

# === INIT SERIAL ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# === INIT DATA ARRAY ===
angle_range = ANGLE_MAX - ANGLE_MIN + 1
heatmap = np.zeros((INTENSITY_LENGTH, angle_range))  # (samples, angles)

# === PLOTTING SETUP ===
fig, ax = plt.subplots()
im = ax.imshow(
    heatmap,
    aspect='auto',
    origin='lower',
    interpolation='nearest',
    extent=[ANGLE_MIN - 0.5, ANGLE_MAX + 0.5, 0, INTENSITY_LENGTH],
    cmap='viridis', # Color map
    vmin=0,         # Min voltage
    vmax=40         # Max voltage
)
ax.set_xlabel("Angle (°)")
ax.set_ylabel("Sample Index")

# === UPDATE FUNCTION ===
def update(frame):
    global heatmap
    try:
        # Read one full frame
        raw_line = ser.readline().strip()
        if raw_line:
            if len(raw_line) % 4 == 2:

                angle_index = int(raw_line[:2], 16)
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
ani = FuncAnimation(fig, update, interval=50)
plt.tight_layout()
plt.show()
