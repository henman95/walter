from machine import Pin
from rp2 import asm_pio, StateMachine

import time

class Encoder:
    def __init__(self, in_base:int, pio_sm:int=0, samples:int=50_000_000, timeout_ms:int=50) -> None:
        self.pio_sm:int = pio_sm 
        self.in_base:Pin =  Pin(in_base, Pin.IN)
        self.jump_pin:Pin = self.in_base
        self.samples:int = samples
        self.timeout_ns:int  = timeout_ms * 1_000_000
        self.sample_ns:float = 1/self.samples

        self.sideset_pin:Pin = Pin(20,Pin.OUT)

    @property
    def pps(self) -> int :
        pulses:int = 0
        pps:int = 0
        sm = StateMachine(
            self.pio_sm,
            self.pio_counter,
            freq=self.samples,
            in_base=self.in_base,
            jmp_pin=self.jump_pin
        )
        sm.active(1)
        sm.put(self.samples)
        end = time.time_ns() + self.timeout_ns
        while time.time_ns() < end:
            if sm.rx_fifo() > 0:
                value = sm.get()
                #print(f"V: {value}")
                pulses = self.samples - value
                pulses = 0 if pulses<0 else pulses
                break
            time.sleep(0.000_1)
        sm.active(0)
        
        if pulses > 0:
            pps = int(1/(self.sample_ns * pulses * 6))
        return pps 

    @staticmethod
    @asm_pio()
    def pio_counter():
        # Setup max number of samples
        pull(block)

        # Wait for starting leading edge
        label("begin")
        mov(x,osr)
        wait(0,pin,0)
        label("begin_loop")
        jmp(pin, "highstate")
        jmp(x_dec, "begin_loop")
        jmp("end")

        # Measure the high state
        label("highstate")
        mov(x,osr)
        label("highstate_loop")
        nop() [3]
        jmp(x_dec,"highstate_count")
        label("highstate_count")
        jmp(pin, "highstate_loop")
        
        # Count the low state
        label("lowstate_loop")
        jmp(pin,"end")
        nop() [2]
        jmp(x_dec, "lowstate_count")
        label("lowstate_count")
        jmp("lowstate_loop")
       
        # Send result to FIFO
        label("end")
        mov(isr,x)
        push()

def main2() -> None:
    pin = Pin(21,Pin.IN)
    while True:
        print(f"Pin: {pin} Value:{pin.value()}")
        time.sleep(1)

def main() -> None:
    c0:Counter = Counter(21)
    loops: int = 0
    while True:
        loops += 1
        pulses,hz = c0.pulses()
        print(f"{loops:>4} {pulses:>5} {hz:>5}")
        time.sleep(1)

if __name__ == "__main__":
    main()
