import appdaemon.plugins.hass.hassapi as hass
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import datetime
import json

#
# Turn Off PBR App
#
# Args:
#

class upDateSheet(hass.Hass):

    def initialize(self):
        start_time = self.datetime() + datetime.timedelta(seconds=10)
        self.handle = self.run_every(self.run_every_Serial,start_time,10)

    def run_every_Serial(self, kwargs):

        # use creds to create a client to interact with the Google Drive API
        scope = ['https://spreadsheets.google.com/feeds',
                 'https://www.googleapis.com/auth/drive']
        creds = ServiceAccountCredentials.from_json_keyfile_name('/share/googleDriveCredentials.json', scope)
        client = gspread.authorize(creds)
        fmt = '%Y-%m-%d %H:%M:%S'
        d = datetime.datetime.now()
        d_string = d.strftime(fmt)
        
        val1 = self.get_state("sensor.ph")
        val2 = self.get_state("sensor.disolved_oxygen")
        val3 = self.get_state("sensor.culture_temperature")
        val4 = self.get_state("sensor.lux")
        val5 = self.get_state("sensor.culture_turbidity")
        val6 = self.get_state("sensor.co2_out")
        
        val7 = ' '
        
        val8 = self.get_state("sensor.coil_two_temp")

        sheet = client.open("vhausAlgaePBR").sheet1
        
        row = [d_string,val1,val2,val3,val4,val5,val6,val7,val8]
        index = 2
        sheet.insert_row(row, index, value_input_option='USER_ENTERED')
        