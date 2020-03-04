import aioesphomeapi
import asyncio
from Logger import Logger

class ESPCommander:
    def __init__(self, hostname, password, key):
        self.logger = Logger()
        self.hostname = hostname
        self.password = password
        self.key = key
        self.asyncio_loop = None

    async def relay_set(self, switch_command):
        cli = aioesphomeapi.APIClient(
            eventloop = self.asyncio_loop,
            address = self.hostname,
            port = 6053,
            password = self.password,
            keepalive = 0
        )
        await asyncio.wait_for(cli.connect(login=True), timeout=10)

        sensors, services = await asyncio.wait_for(cli.list_entities_services(), timeout=10)

        # Get the integer key for the given key (which is actually an ID)
        key_int = None
        for this_sensor in sensors:
            if this_sensor.object_id == self.key:
                self.logger.print("Device %s has matching object: %s" % (self.hostname, this_sensor.object_id))
                key_int = this_sensor.key
            else:
                self.logger.print("Device %s has object: %s" % (self.hostname, this_sensor.object_id))


        if not key_int:
            self.logger.print("Could not get integer key for device %s object %s" % (self.hostname, self.key))
            return

        await asyncio.wait_for(cli.switch_command(key_int, switch_command), timeout=10)
        await asyncio.wait_for(cli.disconnect(), timeout=10)

        self.logger.print("Device %s key %s (%d) was set to: %s" % (self.hostname, self.key, key_int, switch_command))
        self.asyncio_loop.stop()

    def relay_on(self):
        self.asyncio_loop = asyncio.new_event_loop()
        self.asyncio_loop.run_until_complete(self.relay_set(True))
        self.asyncio_loop.run_forever()

    def relay_off(self):
        self.asyncio_loop = asyncio.new_event_loop()
        self.asyncio_loop.run_until_complete(self.relay_set(False))
        self.asyncio_loop.run_forever()
