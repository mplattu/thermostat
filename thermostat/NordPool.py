from datetime import datetime, timedelta
import pytz

from nordpool import elspot
from nordpool_db import nordpool_db

class NordPool:
    def __init__(self, sqlite_path, area, timezone):
        self.UPDATE_TIMEFRAME = 60*60*8
        self.AREA = area
        self.TIMEZONE = pytz.timezone(timezone)
    
        self.npdb = nordpool_db.NordpoolDb(sqlite_path)
    
    def update_nordpool_data(self):
        seconds_from_last_update = self.npdb.get_seconds_from_last_update(self.AREA)
        if seconds_from_last_update is None or seconds_from_last_update > self.UPDATE_TIMEFRAME:
            prices_spot = elspot.Prices()

            self.npdb.update_data(prices_spot.hourly(areas=[self.AREA], end_date=self.TIMEZONE.localize(datetime.today()-timedelta(days=1))))
            self.npdb.update_data(prices_spot.hourly(areas=[self.AREA], end_date=self.TIMEZONE.localize(datetime.today())))
            self.npdb.update_data(prices_spot.hourly(areas=[self.AREA], end_date=self.TIMEZONE.localize(datetime.today()+timedelta(days=1))))

    def is_cheap(self, cheap_hours):
        dt_today_start = self.TIMEZONE.localize(datetime.strptime(datetime.today().strftime('%Y-%m-%d 00:00:00'), '%Y-%m-%d %H:%M:%S'))
        dt_today_end =  self.TIMEZONE.localize(datetime.strptime(datetime.today().strftime('%Y-%m-%d 23:59:59'), '%Y-%m-%d %H:%M:%S'))

        price_rank, _ = self.npdb.get_price_rank(self.AREA, dt_today_start, dt_today_end, self.TIMEZONE.localize(datetime.today()))

        if price_rank is None:
            return False

        return price_rank <= cheap_hours
