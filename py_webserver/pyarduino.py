import serial

ser = serial.Serial(timeout=1)
ser1 = serial.Serial(timeout=1)
dataPort = []
try:
    dataPort = [l.rstrip() for l in open('configFile', 'r').readlines()]
    ser.port = dataPort[0]
    ser.baudrate = 9600
    ser1.port = dataPort[2]
    ser1.baudrate = 9600
except:
    pass


def configure_serial(port_o, config):
    if port_o.is_open:
        port_o.close()
    port_o.port = config[0]
    port_o.baudrate = int(config[1])
    port_o.timeout = 0


def cmd_ard(port_o, instruct):
    if not port_o.is_open:
        port_o.open()
    instruct_byte = instruct.encode('utf-8')
    port_o.write(instruct_byte)

def get_state(port_o, length):
    if not port_o.is_open:
        port_o.open()
    c = ''
    while not c:
        port_o.write(b'9')
        c = port_o.read(length)
    return c