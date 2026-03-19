import time

import framebuf
from machine import SPI, Pin

class OLED_2inch23(framebuf.FrameBuffer):
    WHITE = 0xFFFF
    BLACK = 0x0000

    def __init__(self, spi:int, cs:int, rst:int, dc:int, clk:int, mosi:int) -> None:
        self.width = 128
        self.height = 32
        self.buffer = bytearray(self.height * self.width // 8)
        super().__init__(self.buffer, self.width, self.height, framebuf.MONO_VLSB)

        self.cs   = Pin(cs, Pin.OUT)
        self.rst  = Pin(rst, Pin.OUT)
        self.dc   = Pin(dc, Pin.OUT)
        self.clk  = Pin(clk, Pin.OUT)
        self.mosi = Pin(mosi, Pin.OUT)
        
        self.cs(1)
        self.spi = SPI(spi, 10000_000, polarity=0, phase=0, sck=self.clk, mosi=self.mosi, miso=None)
        self.dc(1)
        self.init_display()

    def write_cmd(self, cmd):
        self.cs(1)
        self.dc(0)
        self.cs(0)
        self.spi.write(bytearray([cmd]))
        self.cs(1)

    def write_data(self, buf):
        self.cs(1)
        self.dc(1)
        self.cs(0)
        self.spi.write(bytearray([buf]))
        self.cs(1)

    def init_display(self):
        """Initialize dispaly"""
        self.rst(1)
        time.sleep(0.001)
        self.rst(0)
        time.sleep(0.01)
        self.rst(1)

        self.write_cmd(0xAE)  # turn off OLED display*/

        self.write_cmd(0x04)  # turn off OLED display*/

        self.write_cmd(0x10)  # turn off OLED display*/

        self.write_cmd(0x40)  # set display start linen(40h~7Fh)
        self.write_cmd(0x81)  # Set SEG Output Current Brightness
        self.write_cmd(0x80)  # set contrast control register
        self.write_cmd(0xA1)  # Set Segment Re-map
        self.write_cmd(0xA6)  # Set Normal/Inverse Display(A7 Inverse)
        self.write_cmd(0xA8)  # Set Multiplex Ratio
        self.write_cmd(
            0x1F
        )  # Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
        self.write_cmd(0xC8)  # Set COM Output Scan Direction (C0h/C8h)
        self.write_cmd(0xD3)  # Set Display Offset
        self.write_cmd(
            0x00
        )  # Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
        self.write_cmd(0xD5)  # Set Display Clock Divide Ratio/ Oscillator Frequency
        self.write_cmd(0xF0)  # -not offset
        self.write_cmd(0xD8)  # Set Area Color Mode ON/OFF & Low Power Display Mode
        self.write_cmd(0x05)
        self.write_cmd(0xD9)  # --set pre-charge period
        self.write_cmd(0xC2)
        self.write_cmd(0xDA)  # --set com pins hardware configuration
        self.write_cmd(0x12)
        self.write_cmd(0xDB)  # Set VCOMH Deselect Level
        self.write_cmd(0x08)
        self.write_cmd(0xAF)  # Set Display ON/OFF (ACh/AEh/AFh)

    def show(self):
        for page in range(0, 4):
            self.write_cmd(0xB0 + page)
            self.write_cmd(0x04)
            self.write_cmd(0x10)
            self.dc(1)
            for num in range(0, 128):
                self.write_data(self.buffer[page * 128 + num])

    def lines(self, lines:list[str]):
        self.fill(self.BLACK)
        for index,line in enumerate(lines):
            self.text(line, 0, 12*index, self.WHITE)
        self.show()

class Display:
    def __init__(self, display_type:str, params:dict) -> None:
        self.display = None
        if display_type == "waveshare_oled_2.23":
            self.display = OLED_2inch23(**params)
        self._text:list[str] = []

    @property
    def text(self) -> list[str]:
        return self._text

    @text.setter
    def text(self, lines:list[str]) -> None:
        self._text = lines
        if self.display:
            self.display.lines(lines)
