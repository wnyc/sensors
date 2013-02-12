from sensors.server.packet_parser import Packet, CicadiaPacket, FlasherPacket

from unittest import main, TestCase

class PacketTests():
    hash = int("deafbeef", 16)
    device = 3
    temp = 273.15
    hit = 22
    miss = 7

    @property
    def header(self):
        return "%(name)s\t%(hash)x\t%(device)d\t"

    def test_type(self):
        self.assertTrue(isinstance(self.pkt, self.type))
    def test_device(self):
        self.assertEquals(self.pkt.device, self.device)

    def setUp(self):
        self.pkt = Packet((self.header + self.packet_format).rstrip()% vars(self))

class HitMissTests():
    packet_format = "%(hit)d\t%(miss)d\t"
    def test_hit(self):
        self.assertEquals(self.pkt.hit, self.hit)
    def test_miss(self):
        self.assertEquals(self.pkt.miss, self.miss)

class FlasherPacketTest(TestCase, PacketTests, HitMissTests):
    self.name = "flasher"
    
    
class CicadiaPacketTest(TestCase, PacketTests, HitMissTests):
    self.name = "cicadia"
    self.packet_format = "%(temp)f\t" + HitMissTests.packet_format
    def test_temp(self):
        self.assertAlmostEquals(self.pkg.temp, self.temp)
    
                      
if __name__ == "__main__":
    unittest.main()
