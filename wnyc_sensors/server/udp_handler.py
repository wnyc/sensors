import SocketServer
import json
from wnyc_sensors.server.packet_parser import Packet

class UDPHandler(SocketServer.BaseRequestHandler):
    """
    This class works similar to the TCP handler class, except that
    self.request consists of a pair of data and client socket, and since
    there is no connection the client address must be given explicitly
    when sending data back via sendto().
    """

    def handle(self):
        print json.dumps(Packet(self.request[0]).as_dict())

def main(port=11311):
    server = SocketServer.UDPServer(('0.0.0.0', port), UDPHandler)
    server.serve_forever()
