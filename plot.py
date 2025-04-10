import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Configuration parameters
SERIAL_PORT = 'COM10'
BAUD_RATE = 115200
TIMEOUT = 1
MAX_POINTS = 500

# Open the serial port
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
except Exception as e:
    print("Error opening serial port:", e)
    exit(1)

data_buffer = []


fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2, linestyle='-')
ax.set_ylim(0.0, 0.1)
ax.set_xlim(0, MAX_POINTS)
ax.set_title("Real-Time ADC Plot")
ax.set_xlabel("Sample Index")
ax.set_ylabel("Voltage [V]")
ax.grid(True)

def init():
    """Initialize the plot line."""
    line.set_data([], [])
    return line,

def update(frame):
    global data_buffer

    raw_line = ser.readline().strip()
    if raw_line:
        if len(raw_line) % 4 != 0: raw_line[:-(len(raw_line) % 4)]

        samples = [raw_line[i:i+4] for i in range(0, len(raw_line), 4)]
        for val in samples:
            data_buffer.append(int(val, 16) / 65535)

    if len(data_buffer) > MAX_POINTS:
        data_buffer = data_buffer[-MAX_POINTS:]

    xdata = list(range(len(data_buffer)))
    ydata = data_buffer
    line.set_data(xdata, ydata)

    ax.set_xlim(0, max(MAX_POINTS, len(data_buffer)))
    return line,

# Interval - delay between reads in ms
ani = animation.FuncAnimation(fig, update, init_func=init, interval=10, blit=True, save_count=10)

try:
    plt.show()
except KeyboardInterrupt:
    print("Plotting interrupted.")

ser.close()

# fig2, ax2 = plt.subplots()
# ax2.plot(range(len(data_buffer)), data_buffer, lw=2, linestyle='-')
# ax2.set_ylim(0, 1)
# ax2.set_xlim(0, MAX_POINTS)
# ax2.set_title("Static ADC Plot")
# ax2.set_xlabel("Sample Index")
# ax2.set_ylabel("Voltage [V]")
# ax2.grid(True)
# plt.show()