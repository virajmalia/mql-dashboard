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
    positions = None
    pos_syms = []
    pos_vol = []
    pos_type = []
    pos_profit = []

    open_syms = None
    open_vol = None
    open_type = None
    open_profit = None

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

    def positions_total(self):
        """Number of Open Positions"""
        return mt5.positions_total()

    def positions_get(self):
        """Get all Open Positions"""
        # Cleanup previous values
        self.pos_syms.clear()
        self.pos_vol.clear()
        self.pos_type.clear()
        self.pos_profit.clear()

        self.positions = mt5.positions_get()
        for position in self.positions:
            self.pos_syms.append(position[16])
            self.pos_profit.append(str(position[15]))
            self.pos_vol.append(str(position[9]))
            self.pos_type.append(str(position[5]))
        self.open_syms = ','.join(self.pos_syms)
        self.open_vol = ','.join(self.pos_vol)
        self.open_type = ','.join(self.pos_type)
        self.open_profit = ','.join(self.pos_profit)

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
    num_pos_topic = "mt5/num_pos"
    syms_topic = "mt5/open_syms"
    vol_topic = "mt5/open_vol"
    type_topic = "mt5/open_type"
    pos_profit_topic = "mt5/open_profit"
    equity = 0

    while True:
        day = time.localtime().tm_wday
        # From Mon-Fri and Sun
        if (0 <= day <= 4) or (day == 6):
            # Account summary
            equity = mt5server.get_account_info().equity
            balance = mt5server.get_account_info().balance
            margin = mt5server.get_account_info().margin
            profit = mt5server.get_account_info().profit
            mt5server.publish_to_topic(bal_topic, balance)
            mt5server.publish_to_topic(eq_topic, equity)
            mt5server.publish_to_topic(margin_topic, margin)
            mt5server.publish_to_topic(profit_topic, profit)
            # Positions
            num_positions = mt5server.positions_total()
            mt5server.positions_get()
            mt5server.publish_to_topic(num_pos_topic, num_positions)
            mt5server.publish_to_topic(syms_topic, mt5server.open_syms)
            mt5server.publish_to_topic(vol_topic, mt5server.open_vol)
            mt5server.publish_to_topic(type_topic, mt5server.open_type)
            mt5server.publish_to_topic(pos_profit_topic, mt5server.open_profit)
            time.sleep(60)
        else:
            time.sleep(23*60*60)

main()
