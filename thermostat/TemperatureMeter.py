import os
import os.path
import re

class TemperatureMeter:
    def __init__(self, owfs_path, owfs_override):
        self.owfs_path = owfs_path
        self.temperature_types = ['DS18S20']

        self.owfs_overrides = owfs_override.split(',')
        self.temperature_files = self.scan_temp_files()

    def scan_temp_files(self):
        temp_files = []

        for this_file in os.listdir(self.owfs_path):
            fullpath = "%s/%s" % (self.owfs_path, this_file)
            if os.path.isdir(fullpath) and self.is_temperature_path(fullpath):
                temp_files.append('%s/temperature' % fullpath)

        return temp_files

    def is_temperature_path(self, path):
        if (not os.path.isfile('%s/temperature' % path)):
            return False
        if (not os.path.isfile('%s/type' % path)):
            return False

        for this_owfs_override in self.owfs_overrides:
            override_re = re.compile(this_owfs_override)
            if override_re.search(path):
                return False

        with open('%s/type' % path, 'r') as f:
            type = f.read()

        if type in self.temperature_types:
            return True

        return False

    def get_temperature(self):
        if len(self.temperature_files) == 0:
            return None

        temperatures = []

        for this_file in self.temperature_files:
            with open(this_file, 'r') as f:
                temperature = f.read()
                print("Measure %s: %s" % (this_file, temperature))
            temperatures.append(float(temperature))

        return sum(temperatures)/len(temperatures)
