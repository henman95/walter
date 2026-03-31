import _thread
import time

from machine import PWM, Pin

from encoder import Encoder


class Motor:
    NEUTRAL = 0
    FORWARD = 1
    REVERSE = 2
    BRAKE = 3

    states = [
        [False, False],
        [False, True],
        [True, False],
        [True, True],
    ]

    def __init__(self, pwm: int, in1: int, in2: int, en1: int, en2: int) -> None:
        self.pwm_pin: PWM = PWM(Pin(pwm))
        self.in1: Pin = Pin(in1, Pin.OUT)
        self.in2: Pin = Pin(in2, Pin.OUT)
        self.en1: Pin = Pin(en1, Pin.IN)
        self.en2: Pin = Pin(en2, Pin.IN)

        self.status: int = 0
        self.brake_status = False

        self.pwm_pin.freq(1000)

        self.encoder = Encoder(en1)
        self.encoder_pwm: int = 0
        self.encoder_rpm: float = 0.0

        self.state = 0
        self.current_state = None

        self.rpm = 0
        self.rpm_setpoint = 0

    def set(self, state, velocity=None):
        self.state = state
        if velocity:
            self.rpm = velocity

    @property
    def rpm(self) -> float:
        return self.encoder_rpm

    @rpm.setter
    def rpm(self, value: float) -> None:
        self.rpm_setpoint = value
        # self.update()

    @property
    def pwm(self) -> int:
        return self.encoder_pwm

    @pwm.setter
    def pwm(self, value: int) -> None:
        if value > 100:
            value = 100
        elif value < 0:
            value = 0
        self.encoder_pwm = value
        self.pwm_pin.duty_u16(int(65535 * abs(value / 100)))

    def update(self):
        self.encoder_rpm = self.encoder.pps / 495 * 60
        if self.state != self.current_state:
            self.current_state = self.state
            self.in1.value(Motor.states[self.state][0])
            self.in2.value(Motor.states[self.state][1])

            if self.state == self.NEUTRAL:
                self.rpm_setpoint = 0
                self.pwm = 0
            elif self.state == self.FORWARD:
                pass
            elif self.state == self.REVERSE:
                pass
            elif self.state == self.BRAKE:
                self.rpm_setpoint = 0
                self.pwm = 0

        # Adjust PWM if needed
        if self.state in [self.FORWARD, self.REVERSE]:
            self.pwm = self.pid()

    def pid(self):
        pwm_in = self.encoder_pwm
        pwm_out = int(self.rpm_setpoint / 1.9)
        return pwm_out


class MotorController:
    LEFT = 0
    RIGHT = 1

    def __init__(self, configs: list[dict]) -> None:
        self.lock = _thread.allocate_lock()
        self.running = False
        self.motors = [Motor(**config) for config in configs]

    def update_task(self):
        while self.running:
            self.do_update()
        print("Core1 Exiting")

    def start_updates(self) -> None:
        print("Controller: Starting updates")
        with self.lock:
            self.running = True
            _thread.start_new_thread(self.update_task, ())

    def stop_updates(self) -> None:
        print("Controller: Stopping updates")
        with self.lock:
            self.running = False

    def do_update(self) -> None:
        with self.lock:
            for motor in self.motors:
                motor.update()

    def cmd(self, cmds: list) -> None:
        with self.lock:
            for cmd in cmds:
                self.motors[cmd[0]].set(cmd[1], cmd[2])

    def brake(self) -> None:
        self.cmd([[x, Motor.BRAKE, 0] for x in range(len(self.motors))])

    def stop(self) -> None:
        self.brake()
        time.sleep(0.5)
        self.neutral()

    def neutral(self) -> None:
        self.cmd([[x, Motor.NEUTRAL, 0] for x in range(len(self.motors))])
