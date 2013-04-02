import serial.tools.list_ports
import urllib
from md5 import md5 
from time import time
import socket 


IPADDR = '209.20.80.141'
PORTNUM = 11311

def usage():
    print 'Usage: cicada.py <serial device> "me@example.com" "My Name" "160 Varick, New York, NY 10031"'
    print
    print "Run cicada.py to upload your sensor's temperature to WNYC and learn how to check on the status of your upload"
    print 
    print "Your system has the following serial ports; your thermometer will be one of them:"
    serial.tools.list_ports.main()
    import sys
    sys.exit(1)

def send(temp, email, name, addr):
    email, name, addr = map(urllib.quote_plus, (email, name, addr))
    packet = "cicada 0 0 2 %s %s %s %s" % (temp, email, name, addr)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
    s.connect((IPADDR, PORTNUM))
    s.send(packet)
    s.close()

def main(argv):

    if len(argv) != 5:
        usage()

    _, device, email, name, addr = argv
    if '@' not in email:
        usage()

    print "You can track your data uploads at http://project.wnyc.net/cicada/raw/upload/" + md5(email).hexdigest()
    print
    print "Trying to connect to sensor" 
    ser = serial.Serial(device, 19200, timeout=5)  
    t = time()
    first_time = True
    while True:
        line = ser.readline()
        if line.startswith('Temp in F'):
            if first_time:
                print "Temp sensor detected!"
                first_time = False
            temp = int(float(line.split()[-1]))
            send(temp, email, name, addr)
            print temp, "F"
            t = time()
        if time() - t > 60:
            if first_time:
                print "No temp sensor detected"
            else:
                print "Temp sensor died"
            break


if __name__ == "__main__":
    import sys
    main(sys.argv)
