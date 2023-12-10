import numpy as np
import matplotlib.pyplot as plt

def kalman_filter(x_measure, y_measure, initialize, R_x = 20, R_y = 20, dt = 0.1):
    # Initialize State
    if initialize == True:
        kalman_filter.x = np.array([[x_measure],
                            [y_measure], 
                            [0],
                            [0]])
        kalman_filter.H = np.array([[1, 0, 0, 0],
                            [0, 1, 0, 0]])
        kalman_filter.HT = np.array([[1, 0],
                            [0, 1],
                            [0, 0],
                            [0, 0]])
        kalman_filter.Q = 0.01 * np.array([[1, 0, 0, 0],
                            [0, 1, 0, 0],
                            [0, 0, 1, 0],
                            [0, 0, 0, 1]])
            
        kalman_filter.R = 2 * np.array([[1, 0.0],
                            [0.0, 1]])
        
        kalman_filter.P = np.array([[1, 0, 0, 0],
                        [0, 1, 0, 0],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])

    kalman_filter.A = np.array([[1, 0, dt, 0],
                        [0, 1, 0, dt],
                        [0, 0, 1, 0],
                        [0, 0, 0, 1]])  

    # Predict State Forward
    x_p = kalman_filter.A @ kalman_filter.x
    # Predict Covariance Forward
    P_p = kalman_filter.A @ kalman_filter.P @ np.transpose(kalman_filter.A) + kalman_filter.Q
    # Compute Kalman Gain
    S = kalman_filter.H @ P_p @ kalman_filter.HT + kalman_filter.R
    K = P_p @ kalman_filter.HT @ np.linalg.inv(S)

    # Estimate State
    residual = np.array([[x_measure], [y_measure]]) - (kalman_filter.H @ x_p)
    kalman_filter.x = x_p + K@residual

    # Estimate Covariance
    kalman_filter.P = P_p - K.dot(kalman_filter.H).dot(P_p)
    # I = np.eye(4)
    # kalman_filter.P = np.dot(np.dot(I - np.dot(K, kalman_filter.H), kalman_filter.P), (I - np.dot(K, kalman_filter.H)).T) + np.dot(np.dot(K, kalman_filter.R), K.T)

    return kalman_filter.x, kalman_filter.P

def weighted_avg_filter(ball_pos:tuple, initialize: bool, time, weight = 0.5):
    if initialize: 
        weighted_avg_filter.ball_pred_y = 0
        weighted_avg_filter.ball_pred_x = 0
        weighted_avg_filter.ball_vel_x = 0
        weighted_avg_filter.ball_vel_y = 0
        weighted_avg_filter.ball_pos_history = []

    weighted_avg_filter.ball_pos_history.append((ball_pos[0], ball_pos[1], time))
    state_count = len(weighted_avg_filter.ball_pos_history)

    if state_count == 1:
        ball_pred_x = ball_pos[0]
        ball_pred_y = ball_pos[1]
        return ball_pos[0], ball_pos[1], 0, 0
    
    # find all the velocities between the last state and the current state
    vel_x = [(weighted_avg_filter.ball_pos_history[i][0] - weighted_avg_filter.ball_pos_history[i-1][0])/(weighted_avg_filter.ball_pos_history[i][2] - weighted_avg_filter.ball_pos_history[i-1][2]) for i in range(1, state_count)]
    vel_y = [(weighted_avg_filter.ball_pos_history[i][1] - weighted_avg_filter.ball_pos_history[i-1][1])/(weighted_avg_filter.ball_pos_history[i][2] - weighted_avg_filter.ball_pos_history[i-1][2]) for i in range(1, state_count)]

    # find the average velocity
    ball_vel_x = sum(vel_x)/len(vel_x)
    ball_vel_y = sum(vel_y)/len(vel_y)

    # predict the current state based on past information 
    last_time_step =weighted_avg_filter. ball_pos_history[-2][2]

    ball_pred_x_ff = weighted_avg_filter.ball_pos_history[-2][0] + ball_vel_x * (time - last_time_step)
    ball_pred_y_ff = weighted_avg_filter.ball_pos_history[-2][1] + ball_vel_y * (time - last_time_step)

    ball_pred_x = (ball_pos[0] * weight) + (ball_pred_x_ff * (1 - weight))
    ball_pred_y = (ball_pos[1] * weight) + (ball_pred_y_ff * (1 - weight))

    weighted_avg_filter.ball_pos_history[-1] = (ball_pred_x, ball_pred_y, time)

    return ball_pred_x, ball_pred_y, ball_vel_x, ball_vel_y