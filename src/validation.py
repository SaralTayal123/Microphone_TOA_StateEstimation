import numpy as np

speed = 0.343 # mm / us
radius_mm = 220/2
mic_locations = [np.array([radius_mm*np.cos(-i*np.pi/3), radius_mm*np.sin(-i*np.pi/3)]) for i in range(5)]

bounce_location = np.array([50, 50])

times = []

offset = np.random.randint(0, 60000)
for mic in mic_locations:
    dist = np.linalg.norm(mic - bounce_location)
    noise = np.random.randint(-100, 100)
    time = dist/speed + offset + noise
    times.append(round(time))
    print("Time", time, "us")
print(times)