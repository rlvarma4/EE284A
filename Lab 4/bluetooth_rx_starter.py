import asyncio
import csv
import re
import time
from bleak import BleakClient

DEVICE_ADDRESS = "7079ADB6-63F3-AC33-C5CA-9E39C9A629D0"
CHAR_UUID = "12345678-1234-1234-1234-1234567890ac"

OUTPUT_FILE = "ina219_bluetooth_data_fan25.csv"

rows = []
start_time = None

def handle_notify(_, data):
    global start_time

    message = data.decode("utf-8").strip()
    print(message)

    # Expected format:
    # Bus: 4.060 V  Shunt: -19.900 mV  Load: 4.040 V  I: -198.900 mA
    match = re.search(
        r"Bus:\s*([-0-9.]+)\s*V\s*"
        r"Shunt:\s*([-0-9.]+)\s*mV\s*"
        r"Load.*?:\s*([-0-9.]+)\s*V\s*"
        r"I:\s*([-0-9.]+)\s*mA",
        message
    )

    if match:
        bus_V = float(match.group(1))
        shunt_mV = float(match.group(2))
        battery_V = float(match.group(3))
        current_mA = float(match.group(4))

        elapsed_ms = int((time.time() - start_time) * 1000)
        rows.append([elapsed_ms, bus_V, shunt_mV, battery_V, current_mA])
    else:
        print("Could not parse line:", message)

async def main():
    global start_time

    client = BleakClient(DEVICE_ADDRESS)

    try:
        await client.connect()
        print("Connected:", client.is_connected)

        start_time = time.time()

        await client.start_notify(CHAR_UUID, handle_notify)
        print("Receiving for 60 seconds...")

        while time.time() - start_time < 60:
            await asyncio.sleep(0.02)

        await client.stop_notify(CHAR_UUID)

    finally:
        if client.is_connected:
            await client.disconnect()
            print("Disconnected cleanly")

    with open(OUTPUT_FILE, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow([
            "time_ms",
            "bus_voltage_V",
            "shunt_voltage_mV",
            "battery_voltage_V",
            "current_mA"
        ])
        writer.writerows(rows)

    if len(rows) > 0:
        avg_bus = sum(row[1] for row in rows) / len(rows)
        avg_shunt = sum(row[2] for row in rows) / len(rows)
        avg_batt = sum(row[3] for row in rows) / len(rows)
        avg_current = sum(row[4] for row in rows) / len(rows)
        avg_power_mW = sum(row[3] * row[4] for row in rows) / len(rows)

        print("\nAverages:")
        print(f"Average bus voltage: {avg_bus:.3f} V")
        print(f"Average shunt voltage: {avg_shunt:.3f} mV")
        print(f"Average battery voltage: {avg_batt:.3f} V")
        print(f"Average current: {avg_current:.3f} mA")
        print(f"Average instantaneous power: {avg_power_mW:.3f} mW")

    print(f"\nSaved {len(rows)} samples to {OUTPUT_FILE}")

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Stopped by user")