import sys

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
                self.logger.print("Required environment %s is missing - exiting")
                sys.exit(1)
            else:
                self.logger.print("Environment variable %s is missing" % name)

        return env_value
