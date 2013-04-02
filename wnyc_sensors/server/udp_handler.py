import SocketServer
from wnyc_sensors.server.packet_parser import Packet

class UDPHandler(SocketServer.BaseRequestHandler):
    """
    This class works similar to the TCP handler class, except that
    self.request consists of a pair of data and client socket, and since
    there is no connection the client address must be given explicitly
    when sending data back via sendto().
    """

    def handle(self):
        packet = Packet(self.request[0]).as_dict()
        print packet
        self.write(packet)

