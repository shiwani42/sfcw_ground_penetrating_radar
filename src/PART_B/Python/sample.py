import serial
from datetime import datetime

PORT = 'COM12'
BAUD_RATE = 115200
BUFFER_SIZE = 993
OUT_FILE = 'correctie_f.txt'

ser = serial.Serial(PORT, BAUD_RATE, timeout=2)
print(f"[{datetime.now()}] Listening on {PORT}...")

try:
    with open(OUT_FILE, 'w') as f:  # <-- changed 'a' to 'w'
        buffer = []

        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if line.isdigit():
                buffer.append(int(line))

                if len(buffer) == BUFFER_SIZE:
                    f.write('\t'.join(map(str, buffer)) + '\n')
                    f.flush()
                    print(f"[{datetime.now()}] Sweep saved with {BUFFER_SIZE} samples")
                    buffer = []

except KeyboardInterrupt:
    print("\nExiting...")

finally:
    ser.close()
