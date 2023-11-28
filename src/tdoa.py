# https://lo.calho.st/posts/tdoa-multilateration/
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

measurements = [
    [0.0, 0.0, 0.0],
    [1918.6593908214895, 4770.574154674195, 0.6],
    [4111.911492926018, 3661.9046769939555, 2.5]
]   # x (m), y (m), t (s)
speed = 343. # m/s
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
xp = np.mean([x for x,y,t in measurements])
yp = np.mean([y for x,y,t in measurements])
x0, y0, t0 = measurements[0]
x1, y1, t1 = measurements[1]
x2, y2, t2 = measurements[2]

F = functions(x0, y0, x1, y1, x2, y2, (t1 - t0) * speed, (t2 - t0) * speed, (t2 - t1) * speed)
J = jacobian(x0, y0, x1, y1, x2, y2, (t1 - t0) * speed, (t2 - t0) * speed, (t2 - t1) * speed)

[x, y], _ = opt.leastsq(F, x0=[xp, yp], Dfun=J)
print(x, y)

# Create reasonable x, y bounds for visualization
max_x = max(x0, x1, x2, x)
min_x = min(x0, x1, x2, x)
range_x = max_x - min_x
min_x -= range_x * .2
max_x += range_x * .2

max_y = max(y0, y1, y2, y)
min_y = min(y0, y1, y2, y)
range_y = max_y - min_y
min_y -= range_y * .2
max_y += range_y * .2

# Create a grid of input coordinates
xs = np.linspace(min_x, max_x, 100)
ys = np.linspace(min_y, max_y, 100)
xs, ys = np.meshgrid(xs, ys)

# Evaluate the system across the grid
A, B, C = F((xs, ys))

# Plot the results
plt.scatter(x0, y0, color='r')
plt.scatter(x1, y1, color='g')
plt.scatter(x2, y2, color='b')
plt.scatter(x, y, color='k')
plt.contour(xs, ys, A, [0], colors='y')
plt.contour(xs, ys, B, [0], colors='m')
plt.contour(xs, ys, C, [0], colors='c')
plt.show()

d0 = np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
d1 = np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.))
d2 = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.))
t0 = d0 / speed
t1 = d1 / speed
t2 = d2 / speed

def circle(cx, cy, r):
    def fn(x, y):
        return np.sqrt(np.power(x - cx, 2.) + np.power(y - cy, 2.)) - r
    return fn

c0 = circle(x0, y0, d0)
c1 = circle(x1, y1, d1)
c2 = circle(x2, y2, d2)

plt.scatter(x0, y0, color='r')
plt.scatter(x1, y1, color='g')
plt.scatter(x2, y2, color='b')
plt.scatter(x, y, color='k')

plt.contour(xs, ys, c0(xs, ys), [0], colors='r')
plt.contour(xs, ys, c1(xs, ys), [0], colors='g')
plt.contour(xs, ys, c2(xs, ys), [0], colors='b')
plt.show()