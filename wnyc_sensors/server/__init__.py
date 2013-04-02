import SocketServer
from wnyc_sensors.server.udp_handler import UDPHandler
from wnyc_sensors.server.uploader import GoogleForm


class Handler(UDPHandler, GoogleForm): pass

def main(port=11311):
    server = SocketServer.UDPServer(('0.0.0.0', port), Handler)
    server.serve_forever()


