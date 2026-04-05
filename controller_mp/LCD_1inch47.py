import time

import framebuf
from machine import SPI, Pin, PWM

class Display2(framebuf.FrameBuffer):
    BLACK = 0xFFFF
    WHITE = 0x0000
    width = 172
    height = 320

    def __init__(self, spi:int, clk:int, dc:int, rst:int, mosi:int, bl:int) -> None:
        self.buffer = bytearray(self.height * self.width * 2)
        super().__init__(self.buffer, self.width, self.height, framebuf.RGB565)

        self.backlight = PWM(bl, freq=1000) 
        self.clk       = Pin(clk, Pin.OUT)
        self.dc        = Pin(dc, Pin.OUT)
        self.mosi      = Pin(mosi, Pin.OUT)
        self.rst       = Pin(rst, Pin.OUT)

        self.backlight.freq(1000)
        self.backlight.duty_u16(32768)

        self.spi = SPI(spi, 40_000_000, phase=0, sck=self.clk, mosi=self.mosi)
        print("DISPLAY")

    def spi_write(self, data):
        if self.spi is None:
            return
        self.spi.write(bytearray(data))

    def delay_ms(self, delaytime:int):
        time.sleep(delaytime / 1000.0)

    def command(self, cmd, data=None) -> None:
        print(f"CMD : {cmd:#02x}")
        self.dc.off()
        self.spi_write([cmd])
        if data is not None:
            self.data(data)

    def data(self, data) -> None:
        if type(data) is int:
            data = bytearray([data])
        data_str = ",".join([f"{v:#02x}" for v in data])
        print(f"DATA: {data_str}")
        self.dc.on()
        self.spi_write(data)
        print()

    def reset(self):
        self.rst.off()
        time.sleep(0.05)
        self.rst.on()
        time.sleep(0.120)

    def init(self):
        print("INIT")
        """Initialize dispaly"""  
        self.reset()

        _porctrl  = [0x0C,0x0C,0x00,0x033,0x033]
        _pvgactrl = [0xF0,0xF0,0x00,0x04,0x04,0x04,0x05,0x29,0x33,0x3E,0x38,0x12,0x12,0x28,0x30]
        _nvgactrl = [0xF0,0x07,0x0A,0x0D,0x0B,0x07,0x28,0x33,0x3E,0x36,0x14,0x14,0x29,0x32]

        self.command(0x01)              # Software Reset
        self.command(0x36, 0x55)        # MADCTL (Memory Control) - Default
        self.command(0x3A, 0x05)        # COLMOD (Interface Pixel Format) - 16bit/pixel
        time.sleep(1)
        self.command(0xB2, _porctrl)    # PORCTRL (Porch Control) - Default
        self.command(0xB7,0x35)         # GCTRL (Gate Control) - Default
        time.sleep(0.1)
        self.command(0xBB,0x35)         # VCOMS 
        self.command(0xC0,0x2C)         # LCM Control - Default
        self.command(0xC2,0x01)         # VDVVRHEN (VDV and VRH enable) - Default
        self.command(0xC3,0x13)         # VRHS -4.5+
        self.command(0xC4,0x20)         # VDVS - Default
        self.command(0xC6,0x0F)         # FRCTRL2 (Frame Rate Control in Normal Mode) - Default 60hz
        self.command(0xD0,[0xA4,0xA1])  # PWCTRL (Power Control) - Default -4.8v
        self.command(0xE0, _pvgactrl)   # PVGACTRL (Positive Voltage Gamma Control)
        self.command(0xE1, _nvgactrl)   # NVGACTRL (Negative Voltage Gamma Control)
        self.command(0x21)              # INVON (Display Inversion On)
        self.command(0x11)              # SLPOUT (Sleep Out - Turn off sleep mode)
        self.command(0x13)              # Normal Display On
        self.command(0x29)              # DISPN (Display On)

    def show(self):
        self.command(0x2A, [0, 0, (self.width-1) >>8, (self.width-1)  & 0xFF])      # CASET (Column Address Set)
        self.command(0x2B, [0, 0, (self.height-1)>>8, (self.height-1) & 0xFF])      # RASET (Row Address Set)
        self.command(0x2C, self.buffer )

    def lines(self):
        self.fill(self.WHITE)
        self.show()

    def test(self):
        self.fill(0xFFFF)

        box_x = (self.width-100) //2
        box_y = (self.height -100) //2
        self.fill_rect(box_x,box_y-2, 104, 104, 0x0000 )



