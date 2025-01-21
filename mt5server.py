""" MetaTrader 5 MQTT Broadcaster """
import time
import configparser
import MetaTrader5 as mt5
import paho.mqtt.client as mqtt

config = configparser.ConfigParser()
config.read('mt5account.ini')  # load configuration file

MT5_EXE = config.get('MetaTrader', 'exe_path')
MT5_SERVER = config.get('MetaTrader', 'server')

class MT5Server():
    """ Metatrader 5 Server """
    # Parameters
    ac_info = None

    def __init__(self) -> None:
        self.init_mt5()
        self.init_mqtt_client()

    def __del__(self):
        # Disconnect from MetaTrader 5 and MQTT broker
        mt5.shutdown()
        self.mqtt_client.disconnect()

    def init_mt5(self):
        """Init"""
        while not mt5.initialize():
            print(f"Reattempting login: {mt5.last_error()}")

        self.get_account_info()

    def get_account_info(self):
        """Account info"""
        self.ac_info = mt5.account_info()
        return self.ac_info

    def init_mqtt_client(self):
        """Setup this device"""
        # Set up MQTT client
        self.mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        # Replace with your MQTT broker address and port
        self.mqtt_client.connect("localhost", 1883)

    # Function to publish time to MQTT topic
    def publish_to_topic(self, topic, data):
        """Publish to MQTT topic"""
        self.mqtt_client.publish(topic, data)

def main():
    """Publish all topics"""
    mt5server = MT5Server()
    bal_topic = "mt5/balance"
    eq_topic = "mt5/equity"
    margin_topic = "mt5/margin"
    profit_topic = "mt5/profit"
    equity = 0

    while True:
        day = time.localtime().tm_wday
        # From Mon-Fri and Sun
        if (0 <= day <= 4) or (day == 6):
            equity = mt5server.get_account_info().equity
            balance = mt5server.get_account_info().balance
            margin = mt5server.get_account_info().margin
            profit = mt5server.get_account_info().profit
            mt5server.publish_to_topic(bal_topic, balance)
            time.sleep(60)
            mt5server.publish_to_topic(eq_topic, equity)
            time.sleep(60)
            mt5server.publish_to_topic(margin_topic, margin)
            time.sleep(60)
            mt5server.publish_to_topic(profit_topic, profit)
            time.sleep(60)
        else:
            time.sleep(23*60*60)

main()
