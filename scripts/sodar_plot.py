import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from threading import Thread, Lock

# === CONFIG ===
SERIAL_PORT = "COM5"
BAUD_RATE = 230400
SAMPLES = 60
FOV = 40
BLIND_ZONE = 10
VMIN_VMAX_SCAle = 0.5

# === INIT SERIAL ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# === INIT DATA ARRAY ===
angle_range = FOV + 1
heatmap_zeros = np.zeros((SAMPLES, angle_range))  # (samples, angles)
heatmap = heatmap_zeros.copy()

# === PLOTTING SETUP ===
fig, ax = plt.subplots()
im = ax.imshow(
    heatmap,
    aspect='auto',
    origin='lower',
    interpolation='nearest',
    # Dist scal = num * sample_div * sound_speed * 100(m -> cm) / sample_rate / 2
    extent=[-FOV/2 - 0.5, FOV/2 + 0.5, 0, SAMPLES * 50 * 343 * 100 / 450e3 / 2],
    cmap='inferno', # Color map
    vmin=0,         # Min voltage
    vmax=20         # Max voltage
)

heatmap_lock = Lock()

ax.set_title("Mode: SRC")
ax.set_xlabel("Angle [°]")
ax.set_ylabel("Distance [cm]")


def read_data():
    global heatmap
    while ser.is_open:
        raw_line = ser.readline().strip()
        if raw_line:
            try:
                mode = 0x80 & int(raw_line[:2], 16)
                if mode:
                    angle_index = 0x3F & int(raw_line[:2], 16)
                    distance = int(raw_line[2:6], 16)

                    data = [0] * SAMPLES
                    data[distance] = 1000

                    if 0 <= angle_index < angle_range:
                        with heatmap_lock:
                            heatmap = heatmap_zeros.copy()
                            heatmap[:, angle_index] = data

                else:
                    angle_index = 0x3F & int(raw_line[:2], 16)
                    hex_data = raw_line[2:]

                    if len(hex_data) != SAMPLES * 4:
                        continue

                    decoded_data = []
                    for val in [hex_data[i:i + 4] for i in range(0, len(hex_data), 4)]:
                        decoded_data.append(int(val, 16) / 65.535)

                    for i in range(BLIND_ZONE):
                        decoded_data[i] = 0

                    if 0 <= angle_index < angle_range:
                        with heatmap_lock:
                            heatmap[:, angle_index] = decoded_data

            except Exception as e:
                print(f"Error: {e}")


# === UPDATE FUNCTION ===
def update(frame):
    global heatmap
    ax.set_title(f"Mode: SRC")

    with heatmap_lock:
        display_data = heatmap.copy()

    current_vmax = np.max(display_data[BLIND_ZONE:, :])
    im.set_clim(vmin=current_vmax * VMIN_VMAX_SCAle, vmax=current_vmax)
    im.set_data(display_data)

# === ANIMATION LOOP ===
try:
    thread = Thread(target=read_data)
    thread.start()

    ani = FuncAnimation(fig, update, interval=100, cache_frame_data=False)
    plt.tight_layout()
    plt.show()
except KeyboardInterrupt:
    print("Closing...")
    ser.close()
    thread.join()