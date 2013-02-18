import socket
import sys


def main():
    HOST, PORT = sys.argv[1:3]
    data = "\t".join(sys.argv[3:])
    PORT = int(PORT)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(data, (HOST, PORT))
