import numpy as np
import datetime
from scipy.optimize import least_squares

speed = 0.343 # mm / us
radius_mm = 220/2

def functions(x0, y0, x1, y1, x2, y2, d01, d02, d12):
    """ Given observers at (x0, y0), (x1, y1), (x2, y2) and TDOA between observers d01, d02, d12, this closure
        returns a function that evaluates the system of three hyperbolae for given event x, y.
    """
    def fn(args):
        x, y = args
        a = np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d01
        b = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d02
        c = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - d12
        return a**2 + b**2
    return fn
def derivative(fun, x, y, delta):
    dfdx = (fun([x+delta,y])-fun([x-delta,y]))/(2*delta)
    dfdy = (fun([x,y+delta])-fun([x,y-delta]))/(2*delta)
    return np.array([dfdx, dfdy])

def grad_desc(fun, x0, y0, num_iter, step_size):
    x = x0
    y = y0
    for _ in range(num_iter):
        grad = derivative(fun, x, y, 0.001)
        if np.linalg.norm(grad) < 0.0001:
            break
        x = x - step_size * grad[0]
        y = y - step_size * grad[1]
    return x, y

def solve_3_points(measurement0, measurement1, measurement2):
    x0, y0, t0 = measurement0
    x1, y1, t1 = measurement1
    x2, y2, t2 = measurement2
    xp = np.mean([x0, x1, x2])
    yp = np.mean([y0, y1, y2])
    F = functions(x0, y0, x1, y1, x2, y2, (t1 - t0) * speed, (t2 - t0) * speed, (t2 - t1) * speed)
    res = least_squares(F, [xp, yp])
    x, y = res.x
    # print("Cost: ", res.cost)
    return x, y, res.cost

def check_sol_validity(measurement0, measurement1, measurement2):
    solx, soly, cost = solve_3_points(measurement0, measurement1, measurement2)
    valid = False
    if (solx ** 2 + soly ** 2 < radius_mm ** 2 and cost < 1e-3):
        valid = True
    return [solx, soly, measurement0, measurement1, measurement2, valid]

def solve_best_fit(measurements):
    index_lut = [[0,1,2], [0,1,3], [0,1,4], [0,2,3], [0,2,4], [0,3,4], [1,2,3], [1,2,4], [1,3,4], [2,3,4]]
    sols = []

    for indexes in index_lut:
        # print("mics used: ", indexes)
        results = check_sol_validity(measurements[indexes[0]], measurements[indexes[1]], measurements[indexes[2]])
        # print("position: ", results[0], results[1], results[-1])
        if results[-1]:
            sols.append(results)
    if len(sols) > 0:
        print(f"Average: ({np.mean([x[0] for x in sols])}, {np.mean([x[1] for x in sols])})")


# measurements = [[110.0, 0.0, 42242],
#                 [55.000000000000014, -95.26279441628824, 42358],
#                 [-54.99999999999998, -95.26279441628826, 42569],
#                 [-110.0, -1.3471114790620885e-14, 42793],
#                 [-55.00000000000005, 95.26279441628823, 42319]]

# import serial
# ser = serial.Serial('COM7', 500000)
# while True:
#     time_data = ser.readline()
#     time_data = time_data.decode('utf-8')
#     time_data = time_data.split(',')
#     # remove trailing \n on last element
#     time_data[-1] = time_data[-1].strip()
#     time_data = [int(x) for x in time_data]

with open("data.txt", "r") as f:
    times = f.readlines()

for time in times:
    time_data = [float(x) for x in time.split(", ")]
    coupled_measurements = [[radius_mm*np.cos(-i*np.pi/3), radius_mm*np.sin(-i*np.pi/3), time_data[i]] for i in range(len(time_data))]
    print(coupled_measurements)
    start = datetime.datetime.now()
    solve_best_fit(coupled_measurements)
    end = datetime.datetime.now()
    diff = end - start
    print("Completion time: ", diff)
    print("")