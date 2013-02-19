import BaseHTTPServer

import json
from wnyc_sensors.server.packet_parser import Packet


class SensorHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        print self.path

def main(server_class=BaseHTTPServer.HTTPServer,
         handler_class=SensorHTTPRequestHandler,
         port=11311):
    server_address = ('0.0.0.0', port)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()
