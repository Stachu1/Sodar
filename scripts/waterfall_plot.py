
import serial
import matplotlib.pyplot as plt
import numpy as np
import time

# Serial port configuration
serial_port = "COM5"  # Updated to the specified serial port
baud_rate = 115200

# Initialize serial connection
ser = serial.Serial(serial_port, baud_rate)

# Parameters for the waterfall chart
num_samples = 500 # Number of samples per measurement sequence
max_cols = 150  # Number of columns to display in the waterfall chart

# Data storage
data = np.zeros((num_samples, max_cols))
times = np.zeros(max_cols)

speed_of_sound = 343  # meters per second in air
sample_time = 11e-6  # 13.2 microseconds in seconds
sample_resolution = (speed_of_sound * sample_time * 100) / 2  # cm

# Set up the plot
plt.ion()
fig, ax = plt.subplots()
waterfall = ax.imshow(data, aspect='auto', cmap='viridis', interpolation='nearest', vmin=0, vmax=0.04)
plt.colorbar(waterfall, ax=ax)

# Initialize y-axis ticks and labels
ax.invert_yaxis()
ax.set_ylabel('Distance (cm)')

tick_step = 50  # Set the step for the y-axis ticks
ax.set_yticks(np.arange(0, num_samples, tick_step)[::-1])  # Create more ticks
ax.set_yticklabels([f'{dist:.2f}' for dist in np.arange(0, num_samples, tick_step)[::-1] * sample_resolution ])

ax.grid(axis='y', linestyle='--', color='gray', linewidth=0.5)


# Initialize x-axis labels
ax.set_xticks(np.linspace(0, max_cols - 1, num=10))
ax.set_xticklabels(['' for _ in range(10)])
ax.set_xlabel('Elapsed Time (s)')

# Start time for elapsed time calculation
start_time = time.time()

# Function to parse data from serial input
def parse_data(line):
    try:
        raw_line = ser.readline().strip()
        if raw_line:
            if len(raw_line) % 4 != 0: raw_line[:-(len(raw_line) % 4)]

            data_buffer = []
            samples = [raw_line[i:i+4] for i in range(0, len(raw_line), 4)]
            for val in samples:
                data_buffer.append(int(val, 16) / 65535)
            return data_buffer
    except Exception as e:
        print(f"Error parsing line: {line} - {e}")
    return None


# Main loop for receiving and plotting data
while True:
    try:
        # Read line from serial port
        line = ser.readline()

        # Parse the line to extract data
        new_data = parse_data(line)
        

        if new_data and len(new_data) == num_samples:
            elapsed_time = time.time() - start_time

            # Update data and plot
            data = np.roll(data, -1, axis=1)  # Shift data left
            data[:, -1] = new_data  # Add new data to the right column

            # Update plot data
            waterfall.set_data(data)
            ax.set_title(f'Waterfall Chart of Analog Measurements\nSample Resolution: {sample_resolution * 4:.2f} cm')

            # Draw plot
            fig.canvas.flush_events()
            plt.draw()
            # plt.pause(0.05)

    except KeyboardInterrupt:
        print("Exiting...")
        break

# Close serial connection
ser.close()



