# Example of low level interaction with a BLE UART device that has an RX and TX
# characteristic for receiving and sending data.  This doesn't use any service
# implementation and instead just manipulates the services and characteristics
# on a device.  See the uart_service.py example for a simpler UART service
# example that uses a high level service implementation.
# Author: Tony DiCola
import logging
import time
import uuid

import Adafruit_BluefruitLE


# Enable debug output.
#logging.basicConfig(level=logging.DEBUG)

# Define service and characteristic UUIDs used by the UART service.
Bluetooth_Base_UUID_suffix = '-0000-1000-8000-00805F9B34FB'
BluetoothUUID = lambda x: '0000' + x + Bluetooth_Base_UUID_suffix

TOUCH_SERVICE_UUID              = uuid.UUID(BluetoothUUID('A002'))
TOUCH_STATE_CHARACTERISTIC_UUID = uuid.UUID(BluetoothUUID('A003'))
MOTOR_SERVICE_UUID              = uuid.UUID(BluetoothUUID('A000'))
MOTOR_STATE_CHARACTERISTIC_UUID = uuid.UUID(BluetoothUUID('A001'))

# Get the BLE provider for the current platform.
ble = Adafruit_BluefruitLE.get_provider()

# Main function implements the program logic so it can run in a background
# thread.  Most platforms require the main thread to handle GUI events and other
# asyncronous events like BLE actions.  All of the threading logic is taken care
# of automatically though and you just need to provide a main function that uses
# the BLE provider.
def main():
    # Clear any cached data because both bluez and CoreBluetooth have issues with
    # caching data and it going stale.
    ble.clear_cached_data()

    # Get the first available BLE network adapter and make sure it's powered on.
    adapter = ble.get_default_adapter()
    adapter.power_on()
    print('Using adapter: {0}'.format(adapter.name))

    # Disconnect any currently connected UART devices.  Good for cleaning up and
    # starting from a fresh state.
    print('Disconnecting any connected Smart Pedal devices...')
    ble.disconnect_devices([TOUCH_SERVICE_UUID, MOTOR_SERVICE_UUID])

    # Scan for UART devices.
    print('Searching for Smart Pedal device...')
    try:
        adapter.start_scan()
        # Search for the first UART device found (will time out after 60 seconds
        # but you can specify an optional timeout_sec parameter to change it).
        device = ble.find_device(service_uuids=[TOUCH_SERVICE_UUID, MOTOR_SERVICE_UUID])
        if device is None:
            raise RuntimeError('Failed to find UART device!')
    finally:
        # Make sure scanning is stopped before exiting.
        adapter.stop_scan()

    print('Connecting to device...')
    device.connect()  # Will time out after 60 seconds, specify timeout_sec parameter
                      # to change the timeout.

    # Once connected do everything else in a try/finally to make sure the device
    # is disconnected when done.
    try:
        # Wait for service discovery to complete for at least the specified
        # service and characteristic UUID lists.  Will time out after 60 seconds
        # (specify timeout_sec parameter to override).
        print('Discovering services...')
        device.discover([TOUCH_SERVICE_UUID, MOTOR_SERVICE_UUID], [TOUCH_STATE_CHARACTERISTIC_UUID, MOTOR_STATE_CHARACTERISTIC_UUID])

        # Find the Touch sensor service and its characteristics.
        touch = device.find_service(TOUCH_SERVICE_UUID)
        touchState = touch.find_characteristic(TOUCH_STATE_CHARACTERISTIC_UUID)
        motor = device.find_service(MOTOR_SERVICE_UUID)
        motorState = motor.find_characteristic(MOTOR_STATE_CHARACTERISTIC_UUID)

        # Write a string to the TX characteristic.
        print('Actuating vibromotor...')
        motor.write_value(0x01)
        time.sleep(2)
        motor.write_value(0x00)

        # Function to receive RX characteristic changes.  Note that this will
        # be called on a different thread so be careful to make sure state that
        # the function changes is thread safe.  Use queue or other thread-safe
        # primitives to send data to other threads.
        def received(data):
            print('Received: {0}'.format(data))

        # Turn on notification of RX characteristics using the callback above.
        print('Subscribing to RX characteristic changes...')
        touch.start_notify(received)

        # Now just wait for 30 seconds to receive data.
        print('Waiting 60 seconds to receive data from the device...')
        time.sleep(60)
    finally:
        # Make sure device is disconnected on exit.
        device.disconnect()


# Initialize the BLE system.  MUST be called before other BLE calls!
ble.initialize()

# Start the mainloop to process BLE events, and run the provided function in
# a background thread.  When the provided main function stops running, returns
# an integer status code, or throws an error the program will exit.
ble.run_mainloop_with(main)
