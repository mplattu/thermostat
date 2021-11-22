import os
import sys
import argparse

from Logger import Logger

class Settings:
    def __init__(self):
        self.logger = Logger()

    def get_environ(self, name, is_required=False):
        env_value = ""
        try:
            env_value = os.environ[name]
        except:
            if is_required:
                self.logger.print("Required environment variable %s is missing - exiting" % name)
                sys.exit(1)
            else:
                self.logger.debug("Environment variable %s is missing" % name)

        return env_value

    def environ_is_true(self, name, is_required=False):
        env_value = self.get_environ(name, is_required).lower()

        if env_value in ["", "0", "no", "false", "-"]:
            return False

        return True
