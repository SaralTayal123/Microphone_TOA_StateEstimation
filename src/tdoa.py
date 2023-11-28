import numpy as np
import scipy.optimize as opt

speed = 0.343 # mm / us
radius_mm = 230/2

def functions(x0, y0, x1, y1, x2, y2, d01, d02, d12):
    """ Given observers at (x0, y0), (x1, y1), (x2, y2) and TDOA between observers d01, d02, d12, this closure
        returns a function that evaluates the system of three hyperbolae for given event x, y.
    """
    def fn(args):
        x, y = args
        a = np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d01
        b = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d02
        c = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - d12
        return [a, b, c]
    return fn
def jacobian(x0, y0, x1, y1, x2, y2, d01, d02, d12):
    def fn(args):
        x, y = args
        adx = (x - x1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - (x - x0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        bdx = (x - x2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (x - x0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        cdx = (x - x2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (x - x1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.))
        ady = (y - y1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - (y - y0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        bdy = (y - y2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (y - y0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        cdy = (y - y2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (y - y1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.))

        return [
            [adx, ady],
            [bdx, bdy],
            [cdx, cdy]
        ]
    return fn

def solve_3_points(measurement0, measurement1, measurement2):
    x0, y0, t0 = measurement0
    x1, y1, t1 = measurement1
    x2, y2, t2 = measurement2
    xp = np.mean([x0, x1, x2])
    yp = np.mean([y0, y1, y2])
    F = functions(x0, y0, x1, y1, x2, y2, (t1 - t0) * speed, (t2 - t0) * speed, (t2 - t1) * speed)
    J = jacobian(x0, y0, x1, y1, x2, y2, (t1 - t0) * speed, (t2 - t0) * speed, (t2 - t1) * speed)
    [x, y], _ = opt.leastsq(F, x0=[xp, yp], Dfun=J)
    return x, y

def check_sol_validity(measurement0, measurement1, measurement2):
    solx, soly = solve_3_points(measurement0, measurement1, measurement2)
    valid = False
    if (solx ** 2 + soly ** 2 < radius_mm ** 2):
        valid = True
    return [solx, soly, valid]

def solve_best_fit(measurements):
    sols = []
    sols.append(check_sol_validity(measurements[0], measurements[1], measurements[2]))
    sols.append(check_sol_validity(measurements[0], measurements[1], measurements[3]))
    sols.append(check_sol_validity(measurements[0], measurements[1], measurements[4]))
    sols.append(check_sol_validity(measurements[0], measurements[2], measurements[3]))
    sols.append(check_sol_validity(measurements[0], measurements[2], measurements[4]))
    sols.append(check_sol_validity(measurements[0], measurements[3], measurements[4]))
    sols.append(check_sol_validity(measurements[1], measurements[2], measurements[3]))
    sols.append(check_sol_validity(measurements[1], measurements[2], measurements[4]))
    sols.append(check_sol_validity(measurements[1], measurements[3], measurements[4]))
    sols.append(check_sol_validity(measurements[2], measurements[3], measurements[4]))

    validx = [sol[0] for sol in sols if sol[2]]
    validy = [sol[1] for sol in sols if sol[2]]

    meanx = np.mean(validx)
    meany = np.mean(validy)
    print(meanx, meany)


if __name__ == "__main__":
    with open("data.txt", "r") as f:
        times = f.readlines()

    for time in times:
        time = [float(x) for x in time.split(", ")]
        measurements = [[radius_mm*np.cos(-i*np.pi/3), radius_mm*np.sin(-i*np.pi/6), time[i]] for i in range(len(time))]
        solve_best_fit(measurements)
