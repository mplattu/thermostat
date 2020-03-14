import aioesphomeapi
import asyncio
from Logger import Logger

class ESPCommander:
    def __init__(self, hostnames, passwords, keys):
        self.logger = Logger()
        self.asyncio_loop = None

        self.devices = self.parse_devices(hostnames, passwords, keys)

        self.test_mode = False

    def parse_devices(self, hostnames, passwords, keys):
        devices = []

        hostname_arr = hostnames.split(',')
        password_arr = passwords.split(',')
        key_arr = keys.split(',')

        for idx,this_hostname in enumerate(hostname_arr):
            try:
                this_password = password_arr[idx]
            except IndexError:
                this_password = None

            try:
                this_key = key_arr[idx]
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

    async def relay_set_real(self, relay_index, switch_command):
        hostname = self.devices[relay_index]['hostname']
        password = self.devices[relay_index]['password']
        key = self.devices[relay_index]['key']

        cli = aioesphomeapi.APIClient(
            eventloop = self.asyncio_loop,
            address = hostname,
            port = 6053,
            password = password,
            keepalive = 0
        )
        await asyncio.wait_for(cli.connect(login=True), timeout=10)

        sensors, services = await asyncio.wait_for(cli.list_entities_services(), timeout=10)

        # Get the integer key for the given key (which is actually an ID)
        key_int = None
        for this_sensor in sensors:
            if this_sensor.object_id == key:
                self.logger.debug("Device %s has matching object: %s" % (hostname, this_sensor.object_id))
                key_int = this_sensor.key
            else:
                self.logger.debug("Device %s has object: %s" % (hostname, this_sensor.object_id))


        if not key_int:
            self.logger.print("Could not get integer key for device %s object %s" % (hostname, key))
            return

        await asyncio.wait_for(cli.switch_command(key_int, switch_command), timeout=10)
        await asyncio.wait_for(cli.disconnect(), timeout=10)

        self.logger.debug("Device %s key %s (%d) was set to: %s" % (hostname, key, key_int, switch_command))

    async def relay_set_all_real(self, switch_command):
        for idx, val in enumerate(self.devices):
            await self.relay_set_real(idx, switch_command)

        self.asyncio_loop.stop()

    def relay_set_all_test(self, switch_command):
        for idx, val in enumerate(self.devices):
            self.logger.debug("Device %s key %s was set to: %s (test-mode)" % (self.devices[idx]['hostname'], self.devices[idx]['key'], switch_command))

    def relay_set(self, switch_command):
        if self.test_mode:
            self.relay_set_all_test(switch_command)
        else:
            self.asyncio_loop = asyncio.new_event_loop()
            self.asyncio_loop.run_until_complete(self.relay_set_all_real(switch_command))
            self.asyncio_loop.run_forever()

    def relay_on(self):
        self.relay_set(True)

    def relay_off(self):
        self.relay_set(False)
