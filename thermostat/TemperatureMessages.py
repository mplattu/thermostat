import select
import time
from socket import *

from Logger import Logger

class TemperatureMessages:
    def __init__(self, udp_port, bind_ip=" "):
        self.logger = Logger()

        self.socket = socket(AF_INET, SOCK_DGRAM)
        self.logger.debug("Binding to IP '%s', port %d" % (bind_ip, udp_port))
        self.socket.bind((bind_ip, udp_port))
        self.socket.setblocking(0)

    def gather_messages(self, wait_time, accepted_sensor_names):
        start_time = int(time.time())

        messages = {}

        while (int(time.time()) < (start_time + wait_time)):
            data_ready = select.select([self.socket], [], [], 2)
            if data_ready[0]:
                data_packet = self.socket.recv(1024)
                data_arr = str(data_packet, encoding='utf-8').split(':', 2)
                sensor = data_arr[0]
                temperature = float(data_arr[1])
                if sensor in accepted_sensor_names:
                    messages[sensor] = float(temperature)
                    self.logger.debug('Got temperature from sensor "%s": %f' % (sensor, temperature))

        return messages

    def get_temperature(self, sensor_values):
        self.logger.debug('Raw sensor values: %s' % sensor_values)

        if len(sensor_values) == 0:
            return None

        temp_sum = 0.0
        for this_sensor in sensor_values:
            temp_sum = temp_sum + sensor_values[this_sensor]

        return temp_sum / len(sensor_values)
