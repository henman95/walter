import time
from machine import Pin, PWM
from encoder import Encoder

class Motor:
    NEUTRAL = 0
    FORWARD = 1
    REVERSE = 2
    BRAKE   = 3
    
    def __init__(self, pwm:int, in1:int, in2:int, en1:int, en2:int) -> None:
        self.pwm_pin:PWM = PWM(Pin(pwm))
        self.in1:Pin = Pin(in1, Pin.OUT)
        self.in2:Pin = Pin(in2, Pin.OUT)
        self.en1:Pin = Pin(en1, Pin.IN)
        self.en2:Pin = Pin(en2, Pin.IN)
        self.pwm_freq:int      = 1000 
        self.en_freq:int       = 50_000_000
        self.en_timeout_ms:int = 50

        self.status:int   = 0
        self.brake_status = False

        self.pwm_pin.freq(self.pwm_freq)

        self.encoder = Encoder(en1)
        self.encoder_pwm:int   = 0
        self.encoder_rpm:float = 0.0

        self.rpm = 0

    @property
    def rpm(self) -> float:
        return self.encoder_rpm

    @rpm.setter
    def rpm(self, value:float) -> None:
        self.pwm = int(value / 1.9)

    @property
    def pwm(self) -> int:
        return self.encoder_pwm

    @pwm.setter
    def pwm(self, value:int) -> None:
        if value > 100:
            value = 100
        elif value < -100:
            value = -100

        if value > 0:
            self.in1.off()
            self.in2.on()
            self.encoder_pwm = value
            self.pwm_pin.duty_u16(int(65535*abs(value/100)))
        elif value < 0:
            self.in1.on()
            self.in2.off()
            self.encoder_pwm = value
            self.pwm_pin.duty_u16(int(65535*abs(value/100)))
        else:
            self.in1.off()
            self.in2.off()
            self.encoder_pwm = 0
            self.pwm_pin.duty_u16(0)
        
    def update(self):
        rpm = self.encoder.pps / 495 * 60
        if self.status == self.REVERSE:
            rpm *= -1
        self.encoder_rpm = rpm

    def brake(self):
        self.set_brake(True)

    def set_brake(self, value:bool) -> None:
        if value:
            self.rpm = 0
            self.in1.on()
            self.in2.on()
            self.status = self.BRAKE
        else:
            self.rpm = 0
            self.status = self.NEUTRAL

class MotorController:
    def __init__(self, configs:list[dict]) -> None:
        self.motors = [Motor(**config) for config in configs]

    def update(self) -> None:
        for motor in self.motors:
            motor.update()

    def brake(self) -> None:
        for motor in self.motors:
            motor.brake()

    def stop(self) -> None:
        for motor in self.motors:
            motor.set_brake(True)
        time.sleep(0.5)
        for motor in self.motors:
            motor.set_brake(False)

    def neutral(self) -> None:
        for motor in self.motors:
            motor.rpm = 0
