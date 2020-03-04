import urllib.request
from xml.dom.minidom import parseString
import xml.dom.minidom
import re

class TemperatureFMI:
    def __init__(self, fmi_url):
        self.fmi_url = fmi_url

    def get_temperature(self):
        contents = urllib.request.urlopen(self.fmi_url).read()
        dom = parseString(contents.decode("utf-8"))
        collection = dom.documentElement
        temperatures = collection.getElementsByTagName("gml:doubleOrNilReasonTupleList")[0]

        temp_array = temperatures.childNodes[0].data.split("\n")
        for this_temp in temp_array:
            temp_string = re.sub(r'[^\d,\.\-]', '', this_temp)
            if this_temp != '':
                return float(temp_string)

        return None
