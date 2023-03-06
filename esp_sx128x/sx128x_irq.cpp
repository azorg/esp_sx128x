/*
 * File: "sx128x_irq.cpp"
 */

//-----------------------------------------------------------------------------
#include "sx128x_irq.h"
#include "sx128x_hw_arduino.h"
#include "global.h"
#include "print.h"
//-----------------------------------------------------------------------------
// check interrupt from SX128x
void sx128x_irq()
{
  int8_t retv;
  uint16_t irq;
  uint8_t recv = 0;
  uint8_t ranging = 0;
  uint8_t buf[255];
  uint8_t verbose = Opt.verbose || !Fsm.run();

  if (!sx128x_hw_irq_flag) return;
  sx128x_hw_irq_flag = 0;

  mrl_clear(&Mrl);

  // get IRQ flags
  retv = sx128x_get_irq(&Radio, &irq);
  if (retv != SX128X_ERR_NONE) { mrl_refresh(&Mrl); return; } // error

  if (irq)
  { // clear IRQ flags
    retv = sx128x_clear_irq(&Radio, irq);
    if (retv != SX128X_ERR_NONE) { mrl_refresh(&Mrl); return; } // error
  }

  if (Opt.verbose >= 2)
  {
    print_str("DIO1 interrupt: cnt=");
    print_uint(sx128x_hw_irq_cnt);
    print_str(" time=");
    print_uint(sx128x_hw_irq_time);
    print_str(" irq=[");
    if (irq & SX128X_IRQ_TX_DONE              ) print_str(" TxDone");
    if (irq & SX128X_IRQ_RX_DONE              ) print_str(" RxDone");
    if (irq & SX128X_IRQ_SYNC_WORD_VALID      ) print_str(" SyncWordValid");
    if (irq & SX128X_IRQ_SYNC_WORD_ERROR      ) print_str(" SyncWordError!");
    if (irq & SX128X_IRQ_HEADER_VALID         ) print_str(" HeaderValid");
    if (irq & SX128X_IRQ_HEADER_ERROR         ) print_str(" HeaderError!");
    if (irq & SX128X_IRQ_CRC_ERROR            ) print_str(" CrcErr!");
    if (irq & SX128X_IRQ_SLAVE_RESPONSE_DONE  ) print_str(" SlaveResponseDone");
    if (irq & SX128X_IRQ_SLAVE_REQUEST_DISCARD) print_str(" SlaveRequestDiscard!");
    if (irq & SX128X_IRQ_MASTER_RESULT_VALID  ) print_str(" MasterResultValid");
    if (irq & SX128X_IRQ_MASTER_TIMEOUT       ) print_str(" MasterTimeout!");
    if (irq & SX128X_IRQ_SLAVE_REQUEST_VALID  ) print_str(" SlaveRequestValid");
    if (irq & SX128X_IRQ_CAD_DONE             ) print_str(" CadDone");
    if (irq & SX128X_IRQ_CAD_DETECTED         ) print_str(" CadDetected");
    if (irq & SX128X_IRQ_RX_TX_TIMEOUT        ) print_str(" RxTxTimeout!");
    if (sx128x_get_advanced_ranging(&Radio))
    {
      if (irq & SX128X_IRQ_ADVANCED_RANGING_DONE) print_str(" AdvancedRangingDone");
    }
    else
    {
      if (irq & SX128X_IRQ_PREAMBLE_DETECTED    ) print_str(" PreambleDetected");
    }
    print_str(" ]\r\n");
  } // if (Opt.verbose > 2)

  if (irq & SX128X_IRQ_TX_DONE)
  { // TX done
    unsigned long dt = Fsm.tx_done(sx128x_hw_irq_time);
    if (verbose) print_uval("TxDone: dt=", dt);
  }

  if (irq & SX128X_IRQ_RX_DONE)
  { // RX done
    unsigned long dt = Fsm.rx_done(sx128x_hw_irq_time);
    if (verbose) print_uval("RxDone: dT=", dt);
    recv = 1;
  }

  if (irq & SX128X_IRQ_RX_TX_TIMEOUT)
  { // RX/TX timeout
    Fsm.rxtx_timeout();
    if (verbose) print_str("RxTxTimeout!\r\n");
  }

  if ((irq & SX128X_IRQ_HEADER_ERROR) || (irq & SX128X_IRQ_CRC_ERROR))
  { // see Errata 16.2 LoRa Modem: Additional Header Checks Required (page 150)
    if (verbose) print_str("HeaderError!\r\n");
    sx128x_rx(&Radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_15_625US);
  }

  if (irq & SX128X_IRQ_MASTER_RESULT_VALID)
  {
    Fsm.rxtx_timeout(); //!!!
    if (verbose) print_str("MasterResultValid:\r\n");
    ranging = 1;
  }

  if (irq & SX128X_IRQ_MASTER_TIMEOUT)
  {
    Fsm.rxtx_timeout();
    if (verbose) print_str("MasterTimeout!\r\n");
    Led.off();
  }

  if (irq & SX128X_IRQ_SLAVE_REQUEST_VALID)
  {
    if (verbose) print_str("SlaveRequestValid:\r\n");
    Led.on();
  }

  if (irq & SX128X_IRQ_SLAVE_RESPONSE_DONE)
  {
    Fsm.rxtx_timeout(); //!!!
    if (verbose) print_str("SlaveResponseDone\r\n");
  }

  if (irq & SX128X_IRQ_SLAVE_REQUEST_DISCARD)
  {
    if (verbose) print_str("SlaveRequestDiscard!\r\n");
    sx128x_rx(&Radio, SX128X_RX_TIMEOUT_CONTINUOUS, SX128X_TIME_BASE_15_625US);
    Led.off();
  }

  if (irq & SX128X_IRQ_CAD_DONE)
  {
    if (verbose) print_str("CadDone\r\n");
    //...
  }

  if (irq & SX128X_IRQ_CAD_DETECTED)
  {
    if (verbose) print_str("CadDetected\r\n");
    //...
  }

  if (sx128x_get_advanced_ranging(&Radio) &&
      irq & SX128X_IRQ_ADVANCED_RANGING_DONE)
  {
    if (verbose) print_str("AdvancedRangingDone:\r\n");
    ranging = 1;
    Led.blink();
  }

  if (recv)
  { // receive packet
    sx128x_rx_t rx;
    uint8_t payload_size;

    // get RX data and RX status from chip (help mega function)
    retv = sx128x_get_recv(
             &Radio,          // pointer to `sx128x_t` object
             // input:
             irq,             // IRQ status from sx128x_get_irq()
             sizeof(buf),     // RX data buffer size
             // output:
             &rx,             // RX status
             buf,             // buffer for RX payload data
             &payload_size);  // real RX payload data size
    if (retv == SX128X_ERR_NONE && (rx.crc_ok || Opt.verbose > 1))
    {
      int i;
      if (rx.lora)
      { // LoRa/Ranging
        print_str("recv: LoRa/Ranging CRC_ok="); print_uint(rx.crc_ok);

        if (rx.hdr.fixed)
        { // Implicit header
          print_str(" HdrType=Implicit\r\n");
        }
        else
        { // Explicit header
          print_str(" HdrType=Explicit (CR="); print_int((int) rx.hdr.cr);
          print_str(" CRC="); print_str(rx.hdr.crc ? "on" : "off");
          print_str(")\r\n");
        }
        print_str("recv:");
        print_str(" RSSI=");      print_rssi(rx.rssi);      print_str("dB");
        print_str(" RSSI_inst="); print_rssi(rx.rssi_inst); print_str("dB");
        print_str(" SNR=");       print_snr(rx.snr);        print_str("dB");
        print_str(" FEI=");       print_fei(rx.fei);        print_str("kHz");
      }
      else
      { // GFSK/FLRC/BLE
        print_str("recv: GFSK/FLRC/BLE CRC_ok="); print_uint(rx.crc_ok);
        print_str(" RSSI="); print_rssi(rx.rssi);  print_str("dBm");
        print_str(" sync_addrs=0b"); print_bin((int) rx.sync_addrs, 3);

#if 1 // more compact
        if (rx.pkt_status & (1 << 0)) print_str(" PktSent=1");
        if (rx.pkt_status & (5 << 5)) print_str(" rxNoAck=1");

        if (rx.pkt_errors & (1 << 0)) print_str(" pktCtrlBusy=1");
        if (rx.pkt_errors & (1 << 1)) print_str(" pktRecv=1"    );
        if (rx.pkt_errors & (1 << 2)) print_str(" hdrRecv=1"    );
        if (rx.pkt_errors & (1 << 3)) print_str(" AbortErr=1"   );
        if (rx.pkt_errors & (1 << 4)) print_str(" crcErr=1"     );
        if (rx.pkt_errors & (1 << 5)) print_str(" LenErr=1"     );
        if (rx.pkt_errors & (1 << 5)) print_str(" SyncErr=1"    );

#else
        print_str(" PktSent="); print_int((int) (rx.pkt_status >> 0) & 0x1);
        print_str(" rxNoAck="); print_int((int) (rx.pkt_status >> 5) & 0x1);

        print_str(" pktCtrlBusy="); print_int((int) (rx.pkt_errors >> 0) & 0x1);
        print_str(" pktRecv="    ); print_int((int) (rx.pkt_errors >> 1) & 0x1);
        print_str(" hdrRecv="    ); print_int((int) (rx.pkt_errors >> 2) & 0x1);
        print_str(" AbortErr="   ); print_int((int) (rx.pkt_errors >> 3) & 0x1);
        print_str(" crcErr="     ); print_int((int) (rx.pkt_errors >> 4) & 0x1);
        print_str(" LenErr="     ); print_int((int) (rx.pkt_errors >> 5) & 0x1);
        print_str(" SyncErr="    ); print_int((int) (rx.pkt_errors >> 6) & 0x1);
#endif
      }
      print_uval("\r\nsize=", payload_size);

      print_str("data:");
      for (i = 0; i < payload_size; i++)
      {
        print_str(" 0x");
        print_hex(buf[i], 2);
      }
      print_eol();
    }
  }

  if (ranging)
  { // get Ranging results
    uint8_t  filter = 0; // FIXME: why? !!!
    uint32_t result;     // raw result
    int32_t  distance;   // distance [dm]
    uint8_t  rssi;       // RSSI of last exchange

    retv = sx128x_ranging_result(
             &Radio,
             &filter,   // result type (0-off, 1-on, 2-as-is)
             &result,   // raw result
             &distance, // distance [dm]
             &rssi);    // RSSI of last exchange
    if (retv == SX128X_ERR_NONE)
    {
      print_str("ranging result: filter=");
      print_uint(filter);
      print_str(" raw=0x");
      print_hex(result, 8);
      print_str(" distance=");
      print_distance(distance);
      print_str("m RSSI=");
      print_rssi(rssi);
      print_str("dBm\r\n");
    }
  }

  mrl_refresh(&Mrl);
}
//-----------------------------------------------------------------------------

/*** end of "sx128x_irq.cpp" file ***/

