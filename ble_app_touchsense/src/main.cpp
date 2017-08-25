/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "touchService.h"
#include "motorService.h"

#define MIN_CONN_INTERVAL 3 // milliseconds
#define MAX_CONN_INTERVAL 5 // milliseconds
#define MIN_TOUCH_DURATION 20 // microseconds
DigitalOut alivenessLED(p7, 0);
DigitalOut motor(p25, 0);

InterruptIn touch(p15);

const static char     DEVICE_NAME[] = "SmartPedal";
static const uint16_t uuid16_list[] = {TouchService::TOUCH_SERVICE_UUID, MotorService::MOTOR_SERVICE_UUID};

static EventQueue eventQueue(/* event count */ 64 * EVENTS_EVENT_SIZE);

MotorService *motorServicePtr;
TouchService *touchServicePtr;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void) params;
    BLE::Instance().gap().startAdvertising();
}

void blinkCallback(void)
{
    alivenessLED = !alivenessLED; /* Do blinky on LED1 to indicate system aliveness. */
}

void touchFallCallback(void)
{
    if (!(bool)touch.read()) eventQueue.call(Callback<void(bool)>(touchServicePtr, &TouchService::updateTouchState), false);
}

void touchRiseCallback(void)
{
    touch.disable_irq();
    wait_us(MIN_TOUCH_DURATION);
    touch.enable_irq();
    if ((bool)touch.read()) eventQueue.call(Callback<void(bool)>(touchServicePtr, &TouchService::updateTouchState), true);
}
/**
 * This callback allows the motorService to receive updates to the motorState Characteristic.
 *
 * @param[in] params
 *     Information about the characterisitc being updated.
 */
void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == motorServicePtr->getValueHandle()) && (params->len == 1)) {
        motor = *(params->data);
    }
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);

    touch.rise(touchRiseCallback);
    touch.fall(touchFallCallback);

    bool initialValueForMotorCharacteristic = false;
    motorServicePtr = new MotorService(ble, initialValueForMotorCharacteristic);
    touchServicePtr = new TouchService(ble, true /* initial value for button pressed */);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    touch.mode(PullDown);
    //eventQueue.call_every(500, blinkCallback);

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}
