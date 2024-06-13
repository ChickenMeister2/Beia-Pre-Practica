from influxdb_client import InfluxDBClient
import pandas as pd
from flask import Flask, jsonify
import requests

app = Flask(__name__)

client = InfluxDBClient(
    url="https://eu-central-1-1.aws.cloud2.influxdata.com",
    token="hidden",
    org="974d440648ed6233"
)

query_api = client.query_api()

flux_query = '''
from(bucket: "sensor_data") 
  |> range(start: -10m) 
  |> filter(fn: (r) => r._measurement == "data" and (r._field == "temperature" or r._field == "humidity"))
  |> last()
'''

@app.route('/data', methods=['GET'])
def get_data():
    result = query_api.query_data_frame(flux_query)
    if not result.empty:
        temperature = result[result['_field'] == 'temperature']['_value'].iloc[0]
        humidity = result[result['_field'] == 'humidity']['_value'].iloc[0]
        return jsonify({"Temperature": f"{temperature}°C", "Humidity": f"{humidity}%"})
    else:
        return jsonify({"Error": "No data found."})

def send_to_ifttt(temperature, humidity):
    url = "https://maker.ifttt.com/trigger/temperature_humidity_update/with/key/hidden"
    data = {"value1": temperature, "value2": humidity}
    print(f"Sending data to IFTTT: {data}")
    response = requests.post(url, json=data)
    if response.status_code == 200:
        print('Notification sent successfully.')
    else:
        print(f'Failed to send notification. Status code: {response.status_code}, Response: {response.text}')

@app.route('/trigger_ifttt', methods=['GET'])
def trigger_ifttt():
    result = query_api.query_data_frame(flux_query)
    if not result.empty:
        temperature = result[result['_field'] == 'temperature']['_value'].iloc[0]
        humidity = result[result['_field'] == 'humidity']['_value'].iloc[0]
        print(f"Temperature: {temperature}, Humidity: {humidity}")
        send_to_ifttt(f"{temperature}°C", f"{humidity}%")
        return jsonify({"Message": "IFTTT notification sent."})
    else:
        print("No data found.") 
        return jsonify({"Error": "No data found."})

if __name__ == "__main__":
    app.run(debug=True)
