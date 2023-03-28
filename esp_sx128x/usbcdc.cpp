/*
 * USB-CDC support for Arduino ESP32
 * File: "usbcdc.cpp"
 */

//-----------------------------------------------------------------------------
#include "usbcdc.h"
#ifdef ARDUINO_USBCDC // look "config.h"
#include <Arduino.h>
//-----------------------------------------------------------------------------
#if ARDUINO_USB_MODE
void usbcdc_begin()
{
  // do nothing
}
//-----------------------------------------------------------------------------
#else // !ARDUINO_USB_MODE
//-----------------------------------------------------------------------------
#if !ARDUINO_USB_CDC_ON_BOOT
USBCDC USBSerial;
#endif
//-----------------------------------------------------------------------------
static void usbcdc_evt_callback(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t * data = (arduino_usb_event_data_t*)event_data;
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT:
        HWSerial.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        HWSerial.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        break;
      case ARDUINO_USB_RESUME_EVENT:
        HWSerial.println("USB RESUMED");
        break;
      
      default:
        break;
    }
  } else if (event_base == ARDUINO_USB_CDC_EVENTS) {
    arduino_usb_cdc_event_data_t * data = (arduino_usb_cdc_event_data_t*)event_data;
    switch (event_id) {
      case ARDUINO_USB_CDC_CONNECTED_EVENT:
        HWSerial.println("CDC CONNECTED");
        break;
      case ARDUINO_USB_CDC_DISCONNECTED_EVENT:
        HWSerial.println("CDC DISCONNECTED");
        break;
      case ARDUINO_USB_CDC_LINE_STATE_EVENT:
        HWSerial.printf("CDC LINE STATE: dtr: %u, rts: %u\n",
                         data->line_state.dtr, data->line_state.rts);
        break;
      case ARDUINO_USB_CDC_LINE_CODING_EVENT:
        HWSerial.printf("CDC LINE CODING: bit_rate: %u, data_bits: %u, stop_bits: %u, parity: %u\n",
                        data->line_coding.bit_rate, data->line_coding.data_bits,
                        data->line_coding.stop_bits, data->line_coding.parity);
        break;
      case ARDUINO_USB_CDC_RX_EVENT:
        // FIXME: copy to RX FIFO/buffer
        HWSerial.printf("CDC RX [%u]:", data->rx.len);
        if (0) // FIXME: dead code, do nothing
        {
          //uint8_t buf[data->rx.len];
          //size_t len = USBSerial.read(buf, data->rx.len);
          //HWSerial.write(buf, len);
        }
        break;
       case ARDUINO_USB_CDC_RX_OVERFLOW_EVENT:
        HWSerial.printf("CDC RX Overflow of %d bytes",
                        data->rx_overflow.dropped_bytes);
        break;
     
      default:
        break;
    }
  }
}
//-----------------------------------------------------------------------------
void usbcdc_begin()
{
  USB.onEvent(usbcdc_evt_callback);
  USBSerial.onEvent(usbcdc_evt_callback);
  USBSerial.begin();
  USBSerial.setDebugOutput(true);
  USB.begin();
}
//-----------------------------------------------------------------------------
#endif // !ARDUINO_USB_MODE
#endif // ARDUINO_USBCDC
//-----------------------------------------------------------------------------

/*** end of "usbcdc.cpp" file ***/

