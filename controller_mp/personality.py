import select
import sys
import time

from machine import Pin
from motors import Motor, MotorController
from LCD_1inch47 import Display2 as Display



class TwoWheel:
    def __init__(self) -> None:
        self.show_stats = False
        self.controller = MotorController(
            [
                {"pwm": 25, "in1": 9, "in2": 8, "en1": 7, "en2": 6},
                {"pwm":  1, "in1": 2, "in2": 3, "en1": 4, "en2": 5},
            ]
        )
        self.display = Display(spi=0, clk=18, dc=16, rst=20, mosi=19, bl=21)
        self.display.init()
        self.display.lines()

    def run(self) -> None:
        text = self.display_text
        #self.display.text = text
        self.controller.start_updates()

        try:
            while True:
                new_text: list[str] = self.display_text
                if new_text != text:
                    text = new_text
                    #self.display.text = text

                if self.show_stats:
                    m0 = self.controller.motors[0]
                    m1 = self.controller.motors[1]
                    print(f"{m0.rpm:<3.2f} {m0.pwm:<5} xxx {m1.rpm:<3.2f} {m1.pwm:<5}")

                self.process_command()
                time.sleep(.5)
        except KeyboardInterrupt:
            self.controller.stop_updates()

    @property
    def display_text(self) -> list[str]:
        m0 = self.controller.motors[0]
        m1 = self.controller.motors[1]
        return [
            f"Hello",
            f"0: {m0.rpm:<3} {m0.pwm:<5}",
            f"0: {m1.rpm:<3} {m1.pwm:<5}",
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
                args = [] if len(tokens) == 1 else tokens[1].split(",")
                args = [int(x) for x in args]
                argc = len(args)
            except ValueError:
                print("error in parameters")
                return

            velocity = args[0] if argc >= 1 else 0

            if command == "f" and argc == 1:
                print(f"Forward: {velocity}")
                self.controller.cmd(
                    [[0, Motor.FORWARD, velocity], [1, Motor.FORWARD, velocity]]
                )
            elif command == "b" and argc == 1:
                print(f"Reverse: {velocity}")
                self.controller.cmd(
                    [[0, Motor.REVERSE, velocity], [1, Motor.REVERSE, velocity]]
                )
            elif command == "l" and argc == 1:
                print(f"Left: {velocity}")
                self.controller.cmd(
                    [[0, Motor.REVERSE, velocity], [1, Motor.FORWARD, velocity]]
                )
            elif command == "r" and argc == 1:
                print(f"Right: {velocity}")
                self.controller.cmd(
                    [[0, Motor.FORWARD, velocity], [1, Motor.REVERSE, velocity]]
                )
            elif command == "0" and argc ==1:
                print(f'Motor0: {velocity}')
                self.controller.cmd( [[0, Motor.FORWARD, velocity]])
            elif command == "1" and argc ==1:
                print(f'Motor1: {velocity}')
                self.controller.cmd( [[1, Motor.FORWARD, velocity]])
            elif command == "p":
                print("Brake:")
                self.controller.cmd([[0, Motor.BRAKE, 0], [1, Motor.BRAKE, 0]])
            elif command == "n":
                print("Neutral:")
                self.controller.neutral()
            elif command == "s":
                print("Stop")
                self.controller.stop()
            elif command == "g" and argc == 0:
                self.show_stats = not self.show_stats
            else:
                print(f"Error: {command} : {args}")
