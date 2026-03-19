import sys
import time

from motors import MotorController, Motor
from displays import Display

def main():
    controller = MotorController([
        {"pwm":14, "in1":10, "in2":11, "freq":1000},
        {"pwm":15, "in1":12, "in2":13, "freq":1000},
    ])

    (motor_left, motor_right) = controller.motors

    display = Display(
        display_type = "waveshare_oled_2.23",
        params = {"spi":0,"cs":17,"rst":12,"dc":16,"clk":18,"mosi":19}
    )

    display.text = [
        "Motor Dir Speed",
        f"Left  {Motor.STATUS[motor_left.status]} {motor_left.speed}",
        f"Right {Motor.STATUS[motor_right.status]} {motor_right.speed}"
    ]

    while line := sys.stdin.readline():
        if line[0] == '\x00':
            tokens = line[1:].strip().split(' ')
        else:
            tokens = line.strip().split(' ')

        command = tokens[0]
        try:
            args = [] if len(tokens) == 1 else tokens[1].split(',')
            args = [int(x) for x in args]
            argc = len(args)
        except ValueError:
            print("error in paramters")
            continue

        if command == "ma" and argc == 1:
            motor_left.value = args[0]
        elif command == "mb" and argc == 1:
            motor_right.value = args[0]
        elif command == "m" and argc == 2:
            motor_left.value  = args[0]
            motor_right.value = args[1]
        elif command == "f" and argc == 1:
            motor_left.value  = args[0]
            motor_right.value = args[0]
        elif command == "b" and argc == 1:
            motor_left.value  = -args[0]
            motor_right.value = -args[0]
        elif command == "l" and argc == 1:
            motor_left.value  = -args[0]
            motor_right.value =  args[0]
        elif command == "r" and argc == 1:
            motor_left.value  =  args[0]
            motor_right.value = -args[0]
        elif command == "p":
            controller.brake()
        elif command == "n":
            controller.neutral()
        elif command == "s":
            controller.brake()
            time.sleep(1)
            controller.neutral()
        elif command == 'g' and argc == 0:
            print(f"Left : {Motor.STATUS[motor_left.status]} {motor_left.speed}")
            print(f"Right: {Motor.STATUS[motor_right.status]} {motor_right.speed}")
        else:
            print(f"Error: {command} : {args}")

        display.text = [
            "Motor Dir Speed",
            f"Left  {Motor.STATUS[motor_left.status]} {motor_left.speed}",
            f"Right {Motor.STATUS[motor_right.status]} {motor_right.speed}"
        ]
       
if __name__ == "__main__":
    main()
