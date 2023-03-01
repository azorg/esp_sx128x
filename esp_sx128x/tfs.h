/*
 * TFS - Arduino Trivial EEPROM "File" System
 * File: "tfs.h"
 */

#pragma once
#ifndef TFS_H
#define TFS_H
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "config.h"
//-----------------------------------------------------------------------------
//#define TFS_ARDUINO // use Arduino "EEPROM.h"
//#define TFS_DEBUG // TFS debug output
//-----------------------------------------------------------------------------
// return values (flags)
#define TFS_SUCCESS      0x000 // no error
#define TFS_ERR_NOTFOUND 0x001 // record not found
#define TFS_ERR_DELETED  0x002 // find only deleted record
#define TFS_ERR_CS       0x004 // check sum error
#define TFS_ERR_FORMAT   0x008 // format error (need EEPROM erase)
#define TFS_ERR_TOOBIG   0x010 // too big record size
#define TFS_ERR_ERASE    0x020 // erase error
#define TFS_ERR_WRITE    0x040 // write error
#define TFS_ERR_VERIFY   0x080 // verify error
#define TFS_ERR_NOSPACE  0x100 // no free space (need EEPROM page erase)
//-----------------------------------------------------------------------------
// TFS structure of one EEPROM region for save one record
typedef struct tfs_ {
  unsigned address;   // base EEPROM address
  unsigned page_size; // size of page (bytes)
  uint8_t  max_page;  // maximal page index (0 or 1)
} tfs_t;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// init TFS structure (initial set EEPROM space)
void tfs_init(tfs_t    *self,      // EEPROM space
              uint8_t  num_pages,  // number of pages (1 or 2)
              unsigned address,    // base EEPROM address
              unsigned page_size); // size of page (bytes)
//-----------------------------------------------------------------------------
// erase all EEPROM space
// return code may be:
//   TFS_SUCCESS   - success erase EEPROM
//   TFS_ERR_ERASE - error erase EEPROM
uint16_t tfs_erase(const tfs_t *self);
//-----------------------------------------------------------------------------
// get address (offset) to data record in EEPROM space
// return error code, address (offset) to data record in EEPROM space and record size
// return code:
//   TFS_SUCCESS                  - find record success
//   TFS_SUCCESS     | TFS_ERR_CS - find record with bad check sum (CS)
//   TFS_ERR_DELETED              - find only old deleted rrecord
//   TFS_ERR_DELETED | TFS_ERR_CS - find only old deleted record with bad CS
//   TFS_ERR_NOTFOUND             - record not found
//   TFS_ERR_FORMAT               - corrupt TFS format (page(s) need erase) 
uint16_t tfs_get(const tfs_t *self,              // EEPROM space
                 unsigned *addr, uint16_t *size, // record region (offset, size)
                 uint16_t *cnt,                  // (re)write counter
                 uint16_t *cs);                  // check sum
//-----------------------------------------------------------------------------
// read record from EEPROM space
// find and copy record from EEPROM to buffer in RAM,
// return error code and record size
// return code:
//   TFS_SUCCESS                  - find record success
//   TFS_SUCCESS     | TFS_ERR_CS - find record with bad check sum (CS)
//   TFS_ERR_DELETED              - find only old deleted rrecord
//   TFS_ERR_DELETED | TFS_ERR_CS - find only old deleted record with bad CS
//   TFS_ERR_NOTFOUND             - record not found
//   TFS_ERR_FORMAT               - corrupt TFS format (page(s) need erase) 
uint16_t tfs_read(const tfs_t *self,                  // EEPROM region
                  void *buffer, uint16_t buffer_size, // destination buffer
                  uint16_t *size,                     // size of record
                  uint16_t *cnt);                     // (re)write counter
//-----------------------------------------------------------------------------
// write record to EEPROM space (mega funcion)
// return code:
//   TFS_SUCCESS    - success write record to EEPROM region
//   TFS_ERR_TOOBIG - too big record size
//   TFS_ERR_ERASE  - erase error
//   TFS_ERR_WRITE  - write error
//   TFS_ERR_VERIFY - verify error
uint16_t tfs_write(const tfs_t *self,                // EEPROM space
                   const void *data, uint16_t size); // record source
//-----------------------------------------------------------------------------
// delete record in EEPROM space (mark record as deleted)
// return code:
//   TFS_SUCCESS      - success write record to EEPROM space
//   TFS_ERR_NOTFOUND - record not found
//   TFS_ERR_WRITE    - write EEPROM error
uint16_t tfs_delete(const tfs_t *self);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // TFS_H

/*** end of "tfs.h" file ***/

