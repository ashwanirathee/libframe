import math
import numpy as np

# ----------------------------
# Quaternion helpers (w,x,y,z)
# ----------------------------
def q_mul(a, b):
    aw, ax, ay, az = a
    bw, bx, by, bz = b
    return np.array([
        aw*bw - ax*bx - ay*by - az*bz,
        aw*bx + ax*bw + ay*bz - az*by,
        aw*by - ax*bz + ay*bw + az*bx,
        aw*bz + ax*by - ay*bx + az*bw
    ], dtype=np.float64)

def q_norm(q):
    return q / np.linalg.norm(q)

def q_from_axis_angle(axis, angle):
    axis = np.asarray(axis, dtype=np.float64)
    n = np.linalg.norm(axis)
    if n < 1e-12:
        return np.array([1,0,0,0], dtype=np.float64)
    axis = axis / n
    s = math.sin(angle/2)
    return np.array([math.cos(angle/2), axis[0]*s, axis[1]*s, axis[2]*s], dtype=np.float64)

def q_to_R(q):
    w,x,y,z = q
    # body->world
    return np.array([
        [1-2*(y*y+z*z), 2*(x*y - w*z), 2*(x*z + w*y)],
        [2*(x*y + w*z), 1-2*(x*x+z*z), 2*(y*z - w*x)],
        [2*(x*z - w*y), 2*(y*z + w*x), 1-2*(x*x+y*y)]
    ], dtype=np.float64)

def q_dot(q, w_body):
    # qdot = 0.5 * [0, wx, wy, wz] ⊗ q  (or q ⊗ omega depending convention)
    # Here we use omega_quat ⊗ q with omega in body frame.
    ow = np.array([0.0, w_body[0], w_body[1], w_body[2]], dtype=np.float64)
    return 0.5 * q_mul(ow, q)

# ----------------------------
# Quad parameters
# ----------------------------
m = 1.0
g = 9.81
I = np.diag([0.02, 0.02, 0.04])  # rough
I_inv = np.linalg.inv(I)

# state
t = 0.0
p = np.array([0.0, 0.5, 0.0], dtype=np.float64)
v = np.array([0.0, 0.0, 0.0], dtype=np.float64)
q = np.array([1.0, 0.0, 0.0, 0.0], dtype=np.float64)  # (w,x,y,z)
w = np.array([0.0, 0.0, 0.0], dtype=np.float64)       # body ang vel

controls = {
    "throttle": 0.55,  # 0..1
    "roll": 0.0,       # rad
    "pitch": 0.0,      # rad
    "yaw_rate": 0.0    # rad/s
}

# simple gains (tune later)
Kp_ang = np.array([6.0, 6.0, 2.5], dtype=np.float64)
Kd_ang = np.array([0.12, 0.12, 0.10], dtype=np.float64)

def set_controls(d):
    # d is a JsProxy coming from JS; convert to a real Python dict
    if hasattr(d, "to_py"):
        d = d.to_py()
    controls.update(d)

def reset_sim():
    global t, p, v, q, w
    t = 0.0
    p = np.array([0.0, 0.5, 0.0], dtype=np.float64)
    v = np.zeros(3, dtype=np.float64)
    q = np.array([1.0, 0.0, 0.0, 0.0], dtype=np.float64)
    w = np.zeros(3, dtype=np.float64)

def step_sim(dt: float):
    global t, p, v, q, w

    # Desired attitude from roll/pitch commands (yaw fixed-ish)
    roll = float(controls["roll"])
    pitch = float(controls["pitch"])
    yaw_rate = float(controls["yaw_rate"])

    # Build q_des from roll/pitch only (no absolute yaw here)
    qx = q_from_axis_angle([1,0,0], roll)
    qy = q_from_axis_angle([0,0,1], 0.0)  # placeholder yaw angle
    qz = q_from_axis_angle([0,0,1], 0.0)
    # Use roll then pitch composition (simple)
    q_des = q_mul(q_from_axis_angle([1,0,0], roll), q_from_axis_angle([0,0,1], 0.0))
    q_des = q_mul(q_des, q_from_axis_angle([0,0,1], 0.0))
    q_des = q_mul(q_des, q_from_axis_angle([0,0,1], 0.0))
    q_des = q_mul(q_from_axis_angle([1,0,0], roll), q_from_axis_angle([0,0,1], 0.0))  # keep simple
    q_des = q_mul(q_from_axis_angle([1,0,0], roll), q_from_axis_angle([0,0,1], 0.0))
    # Better: roll about body x and pitch about body z? For now use small-angle via desired rates below.

    # Instead of full q_des, do desired body rates:
    w_des = np.array([pitch * 2.0, -roll * 2.0, yaw_rate], dtype=np.float64)

    # Angular PD on body rates (simple)
    tau = Kp_ang * (w_des - w) - Kd_ang * w

    # Thrust along body +Z
    throttle = float(controls["throttle"])
    thrust = (0.2 + 1.6 * throttle) * m * g  # maps 0..1 to ~[0.2g .. 1.8g]

    R = q_to_R(q)
    f_world = R @ np.array([0.0, thrust, 0.0], dtype=np.float64)  # NOTE: choose axis consistent with your scene
    # If your "up" in Three.js is +Y (default), thrust should be +Y in world.

    # Linear dynamics
    a = (1.0 / m) * f_world + np.array([0.0, -g, 0.0], dtype=np.float64)

    # Semi-implicit Euler
    v = v + a * dt
    p = p + v * dt

    # Rotational dynamics
    wdot = I_inv @ (tau - np.cross(w, I @ w))
    w = w + wdot * dt
    q = q + q_dot(q, w) * dt
    q = q_norm(q)

    t += dt

def get_state():
    # return plain python types (fast to convert)
    return {
        "t": float(t),
        "p": [float(p[0]), float(p[1]), float(p[2])],
        "v": [float(v[0]), float(v[1]), float(v[2])],
        "q": [float(q[0]), float(q[1]), float(q[2]), float(q[3])],
        "w": [float(w[0]), float(w[1]), float(w[2])]
    }