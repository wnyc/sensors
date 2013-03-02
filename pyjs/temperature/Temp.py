import pyjd # this is dummy in pyjs.
from pyjamas.ui.RootPanel import RootPanel
from pyjamas.ui.ToggleButton import ToggleButton
from pyjamas.ui.HTML import HTML
from pyjamas.ui.Label import Label
from pyjamas.ui.HorizontalPanel import HorizontalPanel
from pyjamas import Window

import pygwt

def greet(*args):
    pass

class LED(ToggleButton):
    def __init__(self, pin, reporter, *args, **kwargs):
        self.bit = pin - 2
        self.reporter = reporter 
        self.reporter.append(self)
        ToggleButton.__init__(self, "-Pin %d dim-" % pin, "-Pin %d lit-" % pin, *args, **kwargs)
    
    def onClick(self, *args, **kwargs):
        ToggleButton.onClick(self, *args, **kwargs)
        self.reporter.compute()

    @property
    def value(self):
        if self.isDown():
            return 1
        return 0
        

class Gauge (Label):
    def __init__(self, *args, **kwargs):
        self.leds = []
        Label.__init__(self, *args, **kwargs)

    def append(self, element):
        self.leds.append(element)


    def compute(self, *args, **kwargs):
        self.setText("The temperature is %0.2f F"% ((self.value * 9 - 80.0) / 20.0))

    @property
    def value(self):
        if (self.left_nibble == self.right_nibble and self.center):
            return 255 + self.left_nibble
        if (self.left_nibble > self.right_nibble) != self.center:
            return self.left_nibble * 16 + self.right_nibble
        else:
            return self.right_nibble * 16 + self.left_nibble
        
    @property
    def center(self):
        return bool(self.leds[4].value)
        
    @property
    def left_nibble(self):
        return self.leds[0].value + self.leds[1].value * 2 + self.leds[2].value * 4 + self.leds[3].value * 8
    
    @property
    def right_nibble(self):
        return self.leds[8].value + self.leds[7].value * 2 + self.leds[6].value * 4 + self.leds[5].value * 8
        

if __name__ == '__main__':
    pyjd.setup("public/Temp.html")
    gauge = Gauge("")
    RootPanel().add(gauge)
    leds = HorizontalPanel()
    RootPanel().add(leds)
    for x in range(2,11):
        leds.add(LED(x, gauge))
    gauge.compute()

    pyjd.run()
