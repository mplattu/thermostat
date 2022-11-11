import aioesphomeapi
import asyncio
from Logger import Logger

class ESPCommander:
    def __init__(self, hostnames, passwords, keys):
        self.logger = Logger()

        self.devices = self.parse_devices(hostnames, passwords, keys)

        self.test_mode = False

    def parse_devices(self, hostnames, passwords, keys):
        devices = []

        for idx,this_hostname in enumerate(hostnames):
            try:
                this_password = passwords[idx]
            except IndexError:
                this_password = None

            try:
                this_key = keys[idx]
            except IndexError:
                this_key = None

            if this_password is not None and this_key is not None:
                devices.append({"hostname": this_hostname, "password": this_password, "key": this_key})
            else:
                if this_password is None:
                    self.logger.print("Relay %s is missing an API password" % this_hostname)
                if this_key is None:
                    self.logger.print("Relay %s is missing a key" % this_hostname)

        return devices

    def set_test_mode(self, test_mode):
        self.test_mode = test_mode

    async def relay_send_command(self, hostname, password, key, switch_command):
        api = aioesphomeapi.APIClient(hostname, 6053, password)
        await api.connect(login=True)

        sensors = await api.list_entities_services()

        # Get the integer key for the given key (which is actually an ID)
        key_int = None
        for this_sensor in sensors[0]:
            if this_sensor.object_id == key:
                self.logger.debug("Device %s has matching object: %s" % (hostname, this_sensor.object_id))
                key_int = this_sensor.key
            else:
                self.logger.debug("Device %s has object: %s" % (hostname, this_sensor.object_id))

        if not key_int:
            self.logger.print("Could not get integer key for device %s object %s" % (hostname, key))
            return
        
        await api.switch_command(key_int, switch_command)
        await api.disconnect()

    def relay_set_real(self, relay_index, switch_command):
        hostname = self.devices[relay_index]['hostname']
        password = self.devices[relay_index]['password']
        key = self.devices[relay_index]['key']

        relay_command_event_loop = asyncio.get_event_loop()
        relay_command_event_loop.run_until_complete(self.relay_send_command(hostname, password, key, switch_command))

        self.logger.debug("Device %s key %s was set to: %s" % (hostname, key, switch_command))

    def relay_set_all_real(self, switch_command):
        for idx, val in enumerate(self.devices):
            self.relay_set_real(idx, switch_command)

    def relay_set_all_test(self, switch_command):
        for idx, val in enumerate(self.devices):
            self.logger.debug("Device %s key %s was set to: %s (test-mode)" % (self.devices[idx]['hostname'], self.devices[idx]['key'], switch_command))

    def relay_set(self, switch_command):
        if self.test_mode:
            self.relay_set_all_test(switch_command)
        else:
            self.relay_set_all_real(switch_command)

    def relay_on(self):
        self.relay_set(True)

    def relay_off(self):
        self.relay_set(False)
