import select
import sys
import time

from machine import Pin

from displays import Display
from motors import Motor, MotorController

class Walter:
    def __init__(self) -> None:
        self.update_rate = 0.1
        self.controller = MotorController(
            [
                {"pwm": 14, "in1": 10, "in2": 11, "en1": 22, "en2": 26},
                {"pwm": 15, "in1": 12, "in2": 13, "en1": 21, "en2": 20},
            ]
        )
        self.display = Display(
            display_type="waveshare_oled_2.23",
            params={"spi": 0, "cs": 17, "rst": 12, "dc": 16, "clk": 18, "mosi": 19},
        )

    def run(self) -> None:
        text = self.display_text
        self.display.text = text
        while True:
            self.controller.update()

            new_text:list[str] = self.display_text
            if new_text != text:
                text = new_text
                self.display.text = text

            self.process_command()
            time.sleep(self.update_rate)

    @property
    def display_text(self) -> list[str]:
        ml,mr = self.controller.motors
        return [
            f"M: D PCT RPM",
            f"L: {ml.pwm:<3} {ml.rpm:>3.2f}",
            f"R: {mr.pwm:<3} {mr.rpm:>3.2f}",
        ]

    def process_command(self):
        motor_left, motor_right = self.controller.motors
        tokens = []
        ready, _, _ = select.select([sys.stdin], [], [], 1)

        if ready:
            line = sys.stdin.readline()
            if line[0] == "\x00":
                tokens = line[1:].strip().split(" ")
            else:
                tokens = line.strip().split(" ")

        if tokens:
            command = tokens[0]
            try:
                args = [] if len(tokens)==1 else tokens[1].split(',')
                args = [int(x) for x in args]
                argc = len(args)
            except ValueError:
                print("error in parameters")
                return

            if command == "ma" and argc == 1:
                motor_left.rpm = args[0]
            elif command == "mb" and argc == 1:
                motor_right.rpm = args[0]
            elif command == "m" and argc == 2:
                motor_left.rpm = args[0]
                motor_right.rpm = args[1]
            elif command == "f" and argc == 1:
                motor_left.rpm = args[0]
                motor_right.rpm = args[0]
            elif command == "b" and argc == 1:
                motor_left.rpm = -args[0]
                motor_right.rpm = -args[0]
            elif command == "l" and argc == 1:
                motor_left.rpm = -args[0]
                motor_right.rpm = args[0]
            elif command == "r" and argc == 1:
                motor_left.rpm = args[0]
                motor_right.rpm = -args[0]
            elif command == "p":
                self.controller.brake()
            elif command == "n":
                self.controller.neutral()
            elif command == "s":
                self.controller.stop()
            elif command == "g" and argc == 0:
                print(
                    f"L: {motor_left.pwm:<3} {motor_left.rpm:>4}",
                    f"R: {motor_right.pwm:<3} {motor_right.rpm:>4}"
                )
                print(f"L: {motor_right.rpm:>4}")
                print(f"R: {motor_right.rpm:>4}")
            else:
                print(f"Error: {command} : {args}")
