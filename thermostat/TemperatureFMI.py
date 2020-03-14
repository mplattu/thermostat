import urllib.request
from xml.dom.minidom import parseString
import xml.dom.minidom
import re

from Logger import Logger

class TemperatureFMI:
    def __init__(self, fmi_url):
        self.fmi_url = fmi_url
        self.logger = Logger()

    def get_temperature(self):
        try:
            contents = urllib.request.urlopen(self.fmi_url).read()
        except:
            self.logger.print("Could not retrieve FMI page")
            return None

        dom = parseString(contents.decode("utf-8"))
        collection = dom.documentElement
        temperatures = collection.getElementsByTagName("gml:doubleOrNilReasonTupleList")[0]

        temp_array = temperatures.childNodes[0].data.split("\n")
        for this_temp in temp_array:
            temp_string = re.sub(r'[^\d,\.\-]', '', this_temp)
            if this_temp != '':
                return float(temp_string)

        return None
