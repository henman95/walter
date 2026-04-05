import time

import framebuf
from machine import SPI, Pin

class LCD_1inch47:
    width = 172
    height = 320

    def __init__(self, spi:int, spi_clk:int, dc:int, rst:int, mosi:int, bl:int, bl_freq:int) -> None:
        self.buffer = bytearray(self.height * self.width // 8)
        super(self.buffer, self.width, self.height, framebuf.MONOVLSB)

        self.DC_PIN  = Pin(dc, Pin.OUT)
        self.RST_PIN = Pin(rst, Pin.OUT)
        self.SPI_CLK = spi_clk
        self.MOSI    = Pin(mosi, Pin.OUT)
        self.BL_PIN  = Pin(bl, Pin.PWM, freq=bl_freq) 

        self.SPI = SPI(spi, spi_clk, polarity=0, phase=0, sck=self.SPI_CLK, mosi=self.MOSI, miso=None)
        self.bl_DutyCycle(0)

    def digital_write(self, Pin, value):
        if value:
            Pin.on()
        else:
            Pin.off()

    def digital_read(self, Pin:Pin):
        return Pin.value

    def spi_write(self, data:list):
        if self.SPI is None:
            return
        self.SPI.write(data)

    def delay_ms(self, delaytime:int):
        time.sleep(delaytime / 1000.0)

    def bl_DutyCycle(self, duty):
        self.BL_PIN.duty_u16(int(65535/100*duty))

    def module_init(self):
        return 0

    def module_exit(self):
        if self.SPI !=None:
            self.SPI.close()    

    def command(self, cmd) -> None:
        self.digital_write(self.DC_PIN, False)
        self.spi_write([cmd])

    def data(self, value) -> None:
        self.digital_write(self.DC_PIN, True)
        if type(value) is list:
            self.spi_write(value)
        else:
            self.spi_write([value])

    def reset(self):
        self.digital_write(self.RST_PIN, True)
        time.sleep(0.01)
        self.digital_write(self.RST_PIN, False)
        time.sleep(0.01)
        self.digital_write(self.RST_PIN, True)
        time.sleep(0.01)

    def Init(self):
        """Initialize dispaly"""  
        self.module_init()
        self.reset()

        self.command(0x36)
        self.data(0x00)                 #self.data(0x00)

        self.command(0x3A)      # COLMOD
        self.data(0x05)         #   16bit/pixel

        self.command(0xB2)      # PORCTRL (Porch Control)
        self.data(0x0C)         #   Default
        self.data(0x0C)
        self.data(0x00)
        self.data(0x33)
        self.data(0x33)

        self.command(0xB7)      # GCTRL (Gate Control)
        self.data(0x35)         #   Default

        self.command(0xBB)      # VCOMS 
        self.data(0x35)         #   Not valid?

        self.command(0xC0)      # LCM Control
        self.data(0x2C)         #   Default

        self.command(0xC2)      # VDVVRHEN (VDV and VRH enable)
        self.data(0x01)         #   Default

        self.command(0xC3)      # VRHS
        self.data(0x13)         #   -4.5+

        self.command(0xC4)      # VDVS
        self.data(0x20)         #   Default

        self.command(0xC6)      # FRCTRL2 (Frame Rate Control in Normal Mode)
        self.data(0x0F)         #   Default - 60Hz

        self.command(0xD0)      # PWCTRL (Power Control)
        self.data(0xA4)         #  Default - 
        self.data(0xA1)         #  AVCL - -4.8v

        self.command(0xE0)      # PVGACTRL (Positive Voltage Gamma Control)
        self.data(0xF0)
        self.data(0xF0)
        self.data(0x00)
        self.data(0x04)
        self.data(0x04)
        self.data(0x04)
        self.data(0x05)
        self.data(0x29)
        self.data(0x33)
        self.data(0x3E)
        self.data(0x38)
        self.data(0x12)
        self.data(0x12)
        self.data(0x28)
        self.data(0x30)

        self.command(0xE1)      # NVGACTRL (Negative Voltage Gamma Control)
        self.data(0xF0)
        self.data(0x07)
        self.data(0x0A)
        self.data(0x0D)
        self.data(0x0B)
        self.data(0x07)
        self.data(0x28)
        self.data(0x33)
        self.data(0x3E)
        self.data(0x36)
        self.data(0x14)
        self.data(0x14)
        self.data(0x29)
        self.data(0x32)
        
        self.command(0x21)      # INVON (Display Inversion On)

        self.command(0x11)      # SLPOUT (Sleep Out - Turn off sleep mode)

        self.command(0x29)      # DISPN (Display On)

    def show(self):
        hs1,hs2 = 34 >> 8 & 0xff, 34 & 0xff
        he1,he2 = (self.width + 33) >>8 & 0xff, (self.width + 33) & 0xff
        vs1,vs2 = 0,0
        ve1,ve2 = self.height >>8 & 0xff, self.height & 0xff
        self.command(0x2A)      # CASET (Column Address Set)
        self.data([hs1,hs2])    # Set horizontal start
        self.data([he1,he2])    # Set horizontal width 
        self.command(0x2B)      # RASET (Row Address Set)
        self.data([vs1,vs2])    # Set vertical Start
        self.data([ve1,ve2])    # Set vertical Height
        self.command(0x2C)      # RAMWR (Memory Write)
        self.data(self.buffer)

    def lines(self):
        self.fill(self.BLACK)
        self.text(0,0,0,"Test")
        self.show()

        

        


        
     
       







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
        elif display_type == "waveshare_lcd_1inch47":
            self.display = LCD_1inch47(**params)
        self._text:list[str] = []

    @property
    def text(self) -> list[str]:
        return self._text

    @text.setter
    def text(self, lines:list[str]) -> None:
        self._text = lines
        if self.display:
            self.display.lines(lines)
