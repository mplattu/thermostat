import datetime
import sys

class Logger:
    def message(self, message):
        return "%s: %s" % (self.get_timestamp(), message)

    def get_timestamp(self):
        return '{0:%Y-%m-%d %H:%M:%S}'.format(datetime.datetime.now())

    def print(self, message):
        print(message)
        self.debug(message)

    def debug(self, message):
        print(self.message(message), file=sys.stderr)
