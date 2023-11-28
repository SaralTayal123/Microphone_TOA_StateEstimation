import serial
with open("data.txt", "a") as f:
    with serial.Serial(port="COM7", baudrate=500000, timeout=1) as ser:
        while True:
            f.write(ser.readline().decode('ascii').replace('\r\n', '\n'))