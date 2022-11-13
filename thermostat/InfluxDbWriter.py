from influxdb_client import InfluxDBClient, Point
from influxdb_client .client.write_api import SYNCHRONOUS
import certifi

from Logger import Logger

class InfluxDbWriter:
    def __init__(self, name, influxdb_url, influxdb_organisation, influxdb_bucket, influxdb_token):
        self.logger = Logger()

        self.client = None
        self.write_api = None
        self.write_api_test = None
        self.last_error_message = ""

        if (influxdb_url == '_test_'):
            self.write_api_test = True
            self.name = name

        if (influxdb_url != '' and influxdb_organisation != '' and influxdb_bucket != '' and influxdb_token != ''):
            self.client = InfluxDBClient(url=influxdb_url, org=influxdb_organisation, token=influxdb_token, verify_ssl=False)
            self.write_api = self.client.write_api(write_options=SYNCHRONOUS)

            self.name = name
            self.influxdb_bucket = influxdb_bucket

    def close(self):
        if self.write_api_test is not None:
            return

        if self.write_api is None:
            self.last_error_message = "close() was called but there is no InfluxDB client defined - check your settings"
            return False
    
    def write_value(self, legend, value):
        if self.write_api_test is not None:
            self.logger.debug(f'Writing InfluxDB name={self.name}, legend={legend}, value={value}')
            return True

        if self.write_api is None:
            self.last_error_message = "write_value() was called but there is no InfluxDB client defined - check your settings"
            return False
        
        point = Point(self.name).field(legend, value)

        self.write_api.write(bucket=self.influxdb_bucket, record=[point])

        return True
    

