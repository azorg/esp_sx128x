Errata
======
Any known issues with the operation of the radio and their solutions are
presented in this Section.
See topic 16 on page 149 of Semtech SX1280 datasheet.

1. All Modems: Continuous Receive Mode in Congested Traffic
-----------------------------------------------------------
If the radio may be subject to high levels of BLE traffic,
to allow the radio to remain in operation RX single mode must be used:
> `sx128x_rx(radio, SX128X_RX_TIMEOUT_SINGLE, timeout_base);`
Do not use:
> `sx128x_rx(radio, SX128X_RX_TIMEOUT_CONTINUOUS, timeout_base);`

2. LoRa Modem: Additional Header Checks Required
------------------------------------------------
If the LoRa modem is used with the header enabled, in the presence
of a header error no RxDone or RxTimeout will be generated.

You mast the HeaderError interrupt must be mapped to the DIO lines
and routed to the host microcontroller. Upon reception of the HeaderError
interrupt the host could, for example, put the radio back in receiver mode.

Example (map IRQ to DIO1):
```
sx128x_irq_dio_mask(radio,
                    SX128X_IRQ_ALL,                      // enable all IRQ
                    dio1_mask | SX128X_IRQ_HEADER_ERROR, // DIO1 mask
                    SX128X_IRQ_NONE,                     // DIO2 mask
                    SX128X_IRQ_NONE);                    // DIO3 mask
```

3. All Modems: Interrupt with Bad CRC
-------------------------------------
You must allways check bit 4 in `pkt_errors` returned by 
`sx128x_packet_status()` in FLRC/GFSK/BLE
(see Table 11-68: Error Packet Status Byte)
and allways check bit 6 (mask `SX128X_IRQ_CRC_ERROR`)
of IRQ register by RX done (see `sx128x_get_irq()`).

4. FLRC Modem: Increased PER in FLRC Packets with Synch Word
------------------------------------------------------------
Solution:
* Disable Sync Word in FLRC: `sx128x_packet_flrc(radio, preamble, sw=0, sw_mode=0, crc, ...);`

* If CR=1/2 or CR=3/4 set in FLRC don't use next Sync Word via call `sx128x_set_sw_flrc()`
```
0x8C38xxxx
0x630Exxxx
```

* Do not use Sync Word and CR=1 in FLRC modes

