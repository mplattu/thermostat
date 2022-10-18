from influxdb_client import InfluxDBClient, Point
from influxdb_client .client.write_api import SYNCHRONOUS
import certifi

class InfluxDbWriter:
    def __init__(self, name, influxdb_url, influxdb_organisation, influxdb_bucket, influxdb_token):
        self.client = None
        self.write_api = None
        self.last_error_message = ""

        if (influxdb_url != '' and influxdb_organisation != '' and influxdb_bucket != '' and influxdb_token != ''):
            self.client = InfluxDBClient(url=influxdb_url, org=influxdb_organisation, token=influxdb_token, verify_ssl=False)
            self.write_api = self.client.write_api(write_options=SYNCHRONOUS)

            self.name = name
            self.influxdb_bucket = influxdb_bucket

    def close(self):
        if self.write_api is None:
            self.last_error_message = "close() was called but there is no InfluxDB client defined - check your settings"
            return False
    
    def write_value(self, legend, value):
        if self.write_api is None:
            self.last_error_message = "write_value() was called but there is no InfluxDB client defined - check your settings"
            return False
        
        point = Point(self.name).field(legend, value)

        self.write_api.write(bucket=self.influxdb_bucket, record=[point])

        return True
    

