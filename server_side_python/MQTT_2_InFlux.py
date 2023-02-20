
"""
conda deactivate
conda env remove -n Flux_3_11
conda create --name Flux_3_11 python=3.11 -y
conda activate Flux_3_11
pip install -U paho-mqtt
pip install -U influxdb

"""

import random
import json

from   datetime  import datetime
from   paho.mqtt import client as mqtt_client
from   influxdb  import InfluxDBClient

def ESP_2_flux(data_str:str):
    data                     = json.loads(data_str)
    flux_data                = {}
    flux_data['time']        = data.pop('time', datetime.now().timestamp())
    flux_data['measurement'] = conf['sensors_pos'].get(str(data['deviceid']), 'unknown')
    flux_data['tags']        = {'device': 'ESP_32', 'topic': conf['MQTT']['topic']}
    flux_data['fields']      = data.copy()
    return(flux_data)


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0: print("Connected to MQTT Broker!")
        else:       print("Failed to connect, return code %d\n", rc)

    client_id = f"{conf['MQTT']['client_id']}-{random.randint(0, 1000)}"
    client    = mqtt_client.Client(client_id)     # Set Connecting Client ID
    #client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(conf['MQTT']['host'], conf['MQTT']['port'])
    return client


def MQTT_subscribe(client: mqtt_client, influx_client):
    def on_message(client, userdata, msg):
        receved_data = msg.payload.decode()
        flux_data    = ESP_2_flux(receved_data)
        print(flux_data)
        influx_client.write_points(flux_data)
    client.influx_client = influx_client
    client.subscribe(conf['MQTT']['topic'])
    client.on_message = on_message


def connect_influx():
    print("toto")
    client  = InfluxDBClient(host=conf['InFlux']['host'], port=conf['InFlux']['port'], username=conf['InFlux']['user'], \
                         password=conf['InFlux']['password']) #, ssl=conf['InFlux']['ssl'], verify_ssl=conf['InFlux']['verify_ssl']
    print("titi")
    db_list = client.get_list_database()
    print("tutu")
    print(db_list)
    return client


if __name__ == '__main__':
    with open('server_side_python/config.json') as fp: conf = json.load(fp)
    influx_client = connect_influx()
    #influx_client =  "None"
    mqtt_client   = connect_mqtt()
    MQTT_subscribe(mqtt_client, influx_client)
    mqtt_client.loop_forever()


"""


client.create_database('example')
client.write_points(json_body)
result = client.query('select value from cpu_load_short;')
print("Result: {0}".format(result))
"""