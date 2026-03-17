import pigpio
import paho.mqtt.client as mqtt
import struct
import time

BROKER = "YOUR_CLOUD_MQTT_IP"
PORT   = 1883
TOPIC  = "kd/rc/pwm6"

INPUT_PINS = [4, 17, 27, 22, 23, 24]

pi = pigpio.pi()
if not pi.connected:
    raise RuntimeError("pigpio not running")

pwm = [1500] * 6
rise = [0] * 6

def cbf(gpio, level, tick):
    idx = INPUT_PINS.index(gpio)
    if level == 1:
        rise[idx] = tick
    elif level == 0:
        pwm[idx] = pigpio.tickDiff(rise[idx], tick)

for pin in INPUT_PINS:
    pi.set_mode(pin, pigpio.INPUT)
    pi.callback(pin, pigpio.EITHER_EDGE, cbf)

client = mqtt.Client(client_id="PI_PWM_TX", clean_session=True)
client.connect(BROKER, PORT, keepalive=15)
client.loop_start()

print("6-CH PWM MQTT Sender started")

while True:
    payload = struct.pack(">6H", *pwm)
    client.publish(TOPIC, payload, qos=0)
    time.sleep(0.02)   # 50Hz RC frame rate
