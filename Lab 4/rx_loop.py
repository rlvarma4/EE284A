import asyncio
import csv
import re
import time
from bleak import BleakClient

DEVICE_ADDRESS = "7079ADB6-63F3-AC33-C5CA-9E39C9A629D0"
CHAR_UUID = "12345678-1234-1234-1234-1234567890ac"

RAW_OUTPUT_FILE = "part7_raw_20ms_sun_then_dark.csv"
AVG_OUTPUT_FILE = "part7_10s_averages_sun_then_dark.csv"

RECORD_TIME_S = 600   # 10 minutes total
SWITCH_TIME_S = 300   # 5 minutes sun, then 5 minutes dark

rows = []
start_time = None

def handle_notify(_, data):
    global start_time

    message = data.decode("utf-8").strip()
    print(message)

    # Expected Arduino format:
    # Bus: 4.060 V  Shunt: -19.900 mV  Load: 4.040 V  I: -198.900 mA  P: -803.556 mW  Fan: 25%  State: LOW_BATT_LOW_FAN

    match = re.search(
        r"Bus:\s*([-0-9.]+)\s*V\s*"
        r"Shunt:\s*([-0-9.]+)\s*mV\s*"
        r"Load:\s*([-0-9.]+)\s*V\s*"
        r"I:\s*([-0-9.]+)\s*mA\s*"
        r"P:\s*([-0-9.]+)\s*mW\s*"
        r"Fan:\s*([0-9]+)%\s*"
        r"State:\s*([A-Za-z0-9_]+)",
        message
    )

    if match:
        elapsed_s = time.time() - start_time
        elapsed_ms = int(elapsed_s * 1000)

        bus_V = float(match.group(1))
        shunt_mV = float(match.group(2))
        battery_V = float(match.group(3))
        current_mA = float(match.group(4))
        power_mW = float(match.group(5))
        fan_percent = int(match.group(6))
        state = match.group(7)

        if elapsed_s < SWITCH_TIME_S:
            condition = "sun"
        else:
            condition = "dark"

        rows.append([
            elapsed_ms,
            elapsed_s,
            condition,
            bus_V,
            shunt_mV,
            battery_V,
            current_mA,
            power_mW,
            fan_percent,
            state
        ])

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

        print("Recording for 10 minutes.")
        print("0–5 min: sun")
        print("5–10 min: dark")

        switched = False

        while time.time() - start_time < RECORD_TIME_S:
            elapsed = time.time() - start_time

            if elapsed >= SWITCH_TIME_S and not switched:
                print("\n==============================")
                print("SWITCH TO DARK NOW")
                print("==============================\n")
                switched = True

            await asyncio.sleep(0.02)

        await client.stop_notify(CHAR_UUID)

    finally:
        if client.is_connected:
            await client.disconnect()
            print("Disconnected cleanly")

    # Save raw 20 ms data
    with open(RAW_OUTPUT_FILE, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow([
            "time_ms",
            "time_s",
            "condition",
            "bus_voltage_V",
            "shunt_voltage_mV",
            "battery_voltage_V",
            "current_mA",
            "power_mW",
            "fan_percent",
            "state"
        ])
        writer.writerows(rows)

    print(f"\nSaved {len(rows)} raw samples to {RAW_OUTPUT_FILE}")

    if len(rows) == 0:
        print("No data received.")
        return

    # Overall averages
    avg_voltage = sum(row[5] for row in rows) / len(rows)
    avg_current = sum(row[6] for row in rows) / len(rows)
    avg_power = sum(row[7] for row in rows) / len(rows)
    fan_on_rows = [row for row in rows if row[8] > 0]
    fan_on_percent = 100 * len(fan_on_rows) / len(rows)

    start_voltage = rows[0][5]
    end_voltage = rows[-1][5]

    print("\nOverall results:")
    print(f"Starting battery voltage: {start_voltage:.3f} V")
    print(f"Ending battery voltage:   {end_voltage:.3f} V")
    print(f"Average battery voltage:  {avg_voltage:.3f} V")
    print(f"Average current:          {avg_current:.3f} mA")
    print(f"Average power:            {avg_power:.3f} mW")
    print(f"Fan on percentage:        {fan_on_percent:.1f}%")

    if end_voltage >= start_voltage:
        print("Result: battery voltage ended equal or higher.")
    else:
        print("Result: battery voltage decreased.")

    # 10-second averages
    avg_rows = []

    print("\n10-second averages:")

    for start in range(0, RECORD_TIME_S, 10):
        end = start + 10
        bin_rows = [row for row in rows if start <= row[1] < end]

        if bin_rows:
            avg_v = sum(row[5] for row in bin_rows) / len(bin_rows)
            avg_i = sum(row[6] for row in bin_rows) / len(bin_rows)
            avg_p = sum(row[7] for row in bin_rows) / len(bin_rows)
            avg_fan = sum(row[8] for row in bin_rows) / len(bin_rows)

            condition = "sun" if start < SWITCH_TIME_S else "dark"

            avg_rows.append([
                start,
                end,
                condition,
                avg_v,
                avg_i,
                avg_p,
                avg_fan
            ])

            print(
                f"{start:3d}-{end:3d} s, {condition}: "
                f"V={avg_v:.3f} V, "
                f"I={avg_i:.3f} mA, "
                f"P={avg_p:.3f} mW, "
                f"Fan={avg_fan:.1f}%"
            )

    with open(AVG_OUTPUT_FILE, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow([
            "start_time_s",
            "end_time_s",
            "condition",
            "avg_battery_voltage_V",
            "avg_current_mA",
            "avg_power_mW",
            "avg_fan_percent"
        ])
        writer.writerows(avg_rows)

    print(f"\nSaved 10-second averages to {AVG_OUTPUT_FILE}")

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Stopped by user")