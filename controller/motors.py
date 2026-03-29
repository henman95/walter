import time
from machine import Pin, PWM

class Motor:
    STATUS = ["NEU", "FWD", "REV", "BRK"]
    def __init__(self, pwm:int, in1:int, in2:int, freq:int) -> None:
        self.pwm:PWM = PWM(Pin(pwm))
        self.in1:Pin = Pin(in1, Pin.OUT)
        self.in2:Pin = Pin(in2, Pin.OUT)
        self.freq:Pin = freq

        self.status:int = 0
        self._value:int = 0

        self.pwm.freq(self.freq)
        self.pwm.duty_u16(0)
        self.in1.off()
        self.in2.off()

    @property
    def value(self) -> int:
        return self._value

    @value.setter
    def value(self, value) -> None:
        self._value = value
        pwm_value = int(65535*abs(value)/100)
        if self._value <= -999:
            self.in1.on()
            self.in2.on()
            self.pwm.duty_u16(pwm_value)
            self._value = 0
            self.status = 3
        elif self._value > 0:
            self.in1.off()
            self.in2.on()
            self.pwm.duty_u16(pwm_value)
            self.status = 1
        elif self._value < 0:
            self.in1.on()
            self.in2.off()
            self.pwm.duty_u16(pwm_value)
            self.status = 2
        else:
            self.in1.off()
            self.in2.off()
            self.pwm.duty_u16(pwm_value)
            self.status = 0

    @property
    def speed(self):
        return abs(self.value)

    def neutral(self) -> None:
        self.value = 0

    def brake(self) -> None:
        self.value = -999

    def __str__(self) -> str:
        return f"Motor: {self.status} {self.value}"


class MotorController:
    def __init__(self, configs:list[dict]) -> None:
        self.motors = [Motor(**config) for config in configs]

    def brake(self) -> None:
        for motor in self.motors:
            print(str(motor))
            motor.brake()

    def neutral(self) -> None:
        for motor in self.motors:
            motor.neutral()
