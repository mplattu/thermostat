import os
import sys
import argparse

from Logger import Logger

class Settings:
    def __init__(self):
        self.logger = Logger()

        self.cmdline_args = None
        self.read_cmdline()

    def read_cmdline(self):
        parser = argparse.ArgumentParser(
            description='Thermostat script',
            epilog='See also the THERMOSTAT_* environment strings')

        parser.add_argument('--owfs', dest='owfs_root', default='/tmp/owfs', help='OWFS-FUSE root path, defaults to /tmp/owfs')
        args = parser.parse_args()
        self.cmdline_args = vars(args)

    def get_cmdline(self, name, is_required=False):
        value = None
        try:
            value = self.cmdline_args[name]
        except:
            if is_required:
                self.logger.print("Required command line parameter %s is missing - exiting" % name)
                self.cmdline_args.print_help()
                sys.exit(1)
            else:
                self.logger.print("Command line parameter %s is missing" % name)

        return value

    def get_environ(self, name, is_required=False):
        env_value = ""
        try:
            env_value = os.environ[name]
        except:
            if is_required:
                self.logger.print("Required environment variable %s is missing - exiting" % name)
                sys.exit(1)
            else:
                self.logger.print("Environment variable %s is missing" % name)

        return env_value
