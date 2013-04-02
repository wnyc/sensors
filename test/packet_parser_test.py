import urllib
from wnyc_sensors.server.packet_parser import Packet, CicadiaPacket, FlasherPacket, CicadaIPPacket
from unittest import main, TestCase


class PacketTests():
    hash = int("deafbeef", 16)
    device = 3
    temp = 273.15
    hit = 22
    miss = 7
    version = 1 

    @property
    def header(self):
        return "%(name)s %(hash)x %(device)d %(version)s "

    def test_type(self):
        self.assertTrue(isinstance(self.pkt, self.type))

    def test_device(self):
        self.assertEquals(self.pkt.device, self.device)

    def setUp(self):
        values = {}
        values.update(vars(PacketTests))
        values.update(vars(self.__class__))
        values.update(vars(self))

        self.packet_string = (self.header + self.packet_format).rstrip() % values

    @property
    def pkt(self):
        return Packet(self.packet_string)
    
    def test_the_test_no_null_entries(self):
        self.assertTrue(all(self.packet_string.split(' ')))
        

class HitMissTests():
    packet_format = "%(hit)d %(miss)d "
    def test_hit(self):
        self.assertEquals(self.pkt.hit, self.hit)

    def test_miss(self):
        self.assertEquals(self.pkt.miss, self.miss)


class FlasherPacketTest(PacketTests, TestCase, HitMissTests):
    name = "flashers"
    type = FlasherPacket
    packet_format = HitMissTests.packet_format

class CicadaaOverIPPacketTest(PacketTests, TestCase):
    name = "cicada"
    type = CicadaIPPacket
    version = 2 
    packet_format = "%(temp)f %(email)s %(user)s %(address)s"
    
    email = urllib.quote_plus("adeprince@nypublicradio.org")
    user = urllib.quote_plus("Adam DePrince")
    address = urllib.quote_plus("160 Varick; New York, NY 10013")

    def test_temp(self):
        self.assertAlmostEquals(self.pkt.temp, self.temp)

    def test_email(self):
        self.assertEquals(self.pkt.email, urllib.unquote_plus(self.email))

    def test_user(self):
        self.assertEquals(self.pkt.user, urllib.unquote_plus(self.user))
        
    def test_address(self):
        self.assertEquals(self.pkt.address, urllib.unquote_plus(self.address))
        
                          
class CicadiaPacketTest(PacketTests, TestCase, HitMissTests):
    name = "cicada"
    type = CicadiaPacket
    packet_format = "%(temp)f " + HitMissTests.packet_format
    def test_temp(self):
        self.assertAlmostEquals(self.pkt.temp, self.temp)

                      
if __name__ == "__main__":
    unittest.main()
