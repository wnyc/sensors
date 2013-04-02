import urllib
PACKET_PARSE_CLASSES = {}

class ParseHitMiss():
    def pop_hit_miss(self):
        return self.pop_int(), self.pop_int()

class ParseFloat():
    def pop_float(self):
        return float(self.tokens.pop())

class Packet(str):
    def __init__(self, value):
        str.__init__(self, value)
        self.tokens = list(reversed(self.split(" ")))
        packet_type = self.pop()
        packet_type = PACKET_PARSE_CLASSES.get(packet_type)
        if not packet_type:
            raise ValueError("Invalid UDP packet.  Don't recognize packet type: " + repr(packet_type))
        self.hash = self.pop_hex()
        self.device = self.pop_int()
        self.version = self.pop_int()
        packet_type = packet_type.get(self.version)
        if not packet_type:
            raise ValueError("Invalid UDP Packet.  Don't recognize packet version: %d" % self.version)
        self.__class__ = packet_type
        self.parse()

    def as_dict(self):
        return {'hash': self.hash,
                'device': self.device,
                'version': self.version,
                'packet_type': self.__class__.__name__}

    def pop_hex(self):
        return self.pop_int(16)

    def pop_int(self, base=10):
        return int(self.pop(), base)

    def pop(self):
        return self.tokens.pop()

    def pops(self, count=1):
        for _ in range(count):
            yield self.pop()

class CicadaIPPacket(Packet, ParseFloat):
    def pop_str(self):
        return urllib.unquote_plus(self.pop())
    
    def as_dict(self):
        d = Packet.as_dict(self)
        d.update({'temp': self.temp,
                  'email': self.email,
                  'address': self.address,
                  'user': self.user})
        return d

    def parse(self):
        self.temp = self.pop_float()
        self.email = self.pop_str()
        self.user = self.pop_str()
        self.address = self.pop_str()
        

class CicadiaPacket(Packet, ParseHitMiss, ParseFloat):
    def as_dict(self):
        d = Packet.as_dict(self)
        d.update({'temp': self.temp,
                  'hit': self.hit,
                  'miss': self.miss})
        return d
                
    def parse(self):
        self.temp = self.pop_float()
        self.hit, self.miss = self.pop_hit_miss()
        

class FlasherPacket(Packet, ParseHitMiss):
    def as_dict(self):
        d = Packet.as_dict(self)
        d.update({'hit': self.hit,
                  'miss': self.miss})
        return d
    def parse(self):
        self.hit, self.miss = self.pop_hit_miss()

PACKET_PARSE_CLASSES['cicada'] = {1:CicadiaPacket,
                                  2:CicadaIPPacket}

PACKET_PARSE_CLASSES['flashers'] = {1:FlasherPacket}
