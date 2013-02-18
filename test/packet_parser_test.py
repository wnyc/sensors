from wnyc_sensors.server.packet_parser import Packet, CicadiaPacket, FlasherPacket

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
        return "%(name)s\t%(hash)x\t%(device)d\t%(version)s\t"

    def test_type(self):
        self.assertTrue(isinstance(self.pkt, self.type))

    def test_device(self):
        self.assertEquals(self.pkt.device, self.device)

    def setUp(self):
        values = {}
        values.update(vars(self))
        values.update(vars(self.__class__))
        values.update(vars(PacketTests))
        self.packet_string = (self.header + self.packet_format).rstrip() % values

    @property
    def pkt(self):
        return Packet(self.packet_string)
    
    def test_the_test_no_null_entries(self):
        self.assertTrue(all(self.packet_string.split('\t')))
        

class HitMissTests():
    packet_format = "%(hit)d\t%(miss)d\t"
    def test_hit(self):
        self.assertEquals(self.pkt.hit, self.hit)

    def test_miss(self):
        self.assertEquals(self.pkt.miss, self.miss)

class FlasherPacketTest(PacketTests, TestCase, HitMissTests):
    name = "flashers"
    type = FlasherPacket
    packet_format = HitMissTests.packet_format

    
class CicadiaPacketTest(PacketTests, TestCase, HitMissTests):
    name = "cicadia"
    type = CicadiaPacket
    packet_format = "%(temp)f\t" + HitMissTests.packet_format
    def test_temp(self):
        self.assertAlmostEquals(self.pkt.temp, self.temp)
    
                      
if __name__ == "__main__":
    unittest.main()
