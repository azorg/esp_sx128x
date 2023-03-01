/*
 * TFS - Trivial EEPROM "File" System
 * File: "tfs.c"
 */

//-----------------------------------------------------------------------------
#include "tfs.h"
//-----------------------------------------------------------------------------
#ifdef TFS_DEBUG
#  include <stdio.h>
#  define TFS_DBG(fmt, arg...) printf("TFS: " fmt "\r\n", ## arg)
#else
#  define TFS_DBG(fmt, ...) // debug output off
#endif // TFS_DEBUG
//-----------------------------------------------------------------------------
#ifdef TFS_ARDUINO // use Arduino EEPROM
#  include "eeprom.h"
#  define TFS_MEMTYPE "EEPROM"
#  define TFS_READ(  addr, cnt, data) eeprom_read(  (unsigned) (addr), (unsigned) (cnt), (uint8_t*) (data))
#  define TFS_WRITE( addr, cnt, data) eeprom_write( (unsigned) (addr), (unsigned) (cnt), (const uint8_t*) (data))
#  define TFS_VERIFY(addr, cnt, data) eeprom_verify((unsigned) (addr), (unsigned) (cnt), (const uint8_t*) (data))
#  define TFS_ERASE( addr, cnt)       eeprom_erase( (unsigned) (addr), (unsigned) (cnt), (uint8_t) 0x00)
#  define TFS_READ_WORD(addr) tfs_read_word((unsigned) (addr))
#  ifdef EEPROM_COMMIT
#    define TFS_COMMIT() EEPROM_COMMIT()
#  endif
#else
#  error Unknown EEPROM type and options!
#endif
//-----------------------------------------------------------------------------
#ifndef TFS_MAX_RECORD_SIZE
#  define TFS_MAX_RECORD_SIZE 4096
#endif
//-----------------------------------------------------------------------------
// record header format
// ^^^^^^^^^^^^^^^^^^^^
// offset | bytes | ident   | value
//--------+-------+---------+--------------------------------------------------
//  0     |  2    | sig     | signature:
//        |       | FREE    | 0x0000 - free space
//        |       | DATA    | 0x55AA - data record
//        |       | DELETED | 0xFFFF - delete record
//--------+-------+---------+--------------------------------------------------
//  2     |  2    | cnt     | (re)write counter (start value is 0)
//--------+-------+---------+--------------------------------------------------
//  4     |  2    | size    | Size of record data [0x0000..0xFFFF]
//--------+-------+---------+--------------------------------------------------
//  6     |  2    | cs      | Check Sum (ADD and XOR)
//--------+-------+---------+--------------------------------------------------
//  8     | size  | data    | record data (size bytes)
//--------+-------+---------+--------------------------------------------------
//
// page states:
// ^^^^^^^^^^^^
//  0. Empty  - initial state after erase EEPROM (all words are 0x0000)
//  1. Opened - first sig=0x55AA or sig=0xFFFF
//
//  If two page used then may be next combination:
//    Empty  and Empty  - initial state after tfs_erase()
//    Opened and Empty  - write to first page
//    Empty  and Opened - write to secnd page
//    Opened and Opened - error state
//
//-----------------------------------------------------------------------------
// size and offsets
#define TFS_HDR_SIZE 8 // header size (bytes)
#define TFS_OFF_SIG  0 // signature offset
#define TFS_OFF_CNT  2 // (re)write counter offset
#define TFS_OFF_SIZE 4 // size offset
#define TFS_OFF_CS   6 // check sum offset
#define TFS_ALIGN    1 // alignment: 1, 2, 4, 8 (bytes) 
//-----------------------------------------------------------------------------
// record signatures (16-bit words):
#define TFS_SIG_FREE    0x0000 // free space
#define TFS_SIG_DATA    0x55AA // data record
#define TFS_SIG_DELETED 0xFFFF // deleted record
//-----------------------------------------------------------------------------
// alignment record size (bytes)
#define TFS_ALIGN_OFF(size) (((size) + (TFS_ALIGN - 1)) & ~(TFS_ALIGN - 1))
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// read 16-bit word from EEPROM at once
static uint16_t tfs_read_word(unsigned addr)
{
  uint16_t word;
  TFS_READ(addr, sizeof(uint16_t), &word);
  return word;
}
//-----------------------------------------------------------------------------
// calculate trivial ADD/XOR check sum
static uint16_t tfs_cs(const void *data, uint16_t size)
{
  const uint8_t *ptr = (const uint8_t*) data;
  uint8_t add = 0xA5, xor = 0x3C;
  while (size--)
  {
    uint8_t byte = *ptr++;
    add += byte;
    xor ^= byte;
  }
  return (((uint16_t) add) << 8) | ((uint16_t) xor);
}
//-----------------------------------------------------------------------------
// find header address of record in EEPROM space and page index
// access mode:
//   0 - find any record
//   1 - find only non deleted record
//   2 - find only deleted record
// return: TFS_ERR_SUCCESS or TFS_ERR_NOTFOUND or TFS_ERR_FORMAT or TFS_ERR_DELETED
static uint16_t tfs_find_hdr(const tfs_t *self, uint8_t mode,
                             unsigned *addr, uint8_t *page)
{
  unsigned paddr = *addr = self->address;
  unsigned max_offset = self->page_size - TFS_HDR_SIZE;
  uint8_t pg, result = 0; // 0-not found, 1-found
  *page = 0;  

  for (pg = 0; pg <= self->max_page; pg++)
  {
    unsigned offset = 0;
    do { // while (offset < max_offset);
      unsigned ptr = paddr + offset;
      uint16_t hdr  = TFS_READ_WORD(ptr + TFS_OFF_SIG);
      uint16_t size = TFS_READ_WORD(ptr + TFS_OFF_SIZE);
      if (hdr == TFS_SIG_FREE)
      { // end of used EEPROM area in page (or empty page)
        break;
      }
      else if (hdr == TFS_SIG_DATA)
      { // find non deleted record
        if (mode != 2)
        {
          *addr = ptr;
          *page = pg;
          return TFS_SUCCESS;
        }
      }
      else if (hdr == TFS_SIG_DELETED)
      { // find deleted record
        if (size == 0 || size > TFS_MAX_RECORD_SIZE)
        { // bad record size
          return TFS_ERR_FORMAT;
        }
        if (mode != 1)
        {
          result = 1;
          *addr = ptr;
          *page = pg;
        }
      }
      else
      { // unknown (bad) hdr signature
        return TFS_ERR_FORMAT;
      }
      offset += TFS_ALIGN_OFF(size) + TFS_HDR_SIZE;
    } while (offset < max_offset);
    paddr += self->page_size;
  } // for (pg..

  if (result == 0) // record not found
    return TFS_ERR_NOTFOUND;

  return TFS_ERR_DELETED;
}
//-----------------------------------------------------------------------------
// find address to free space (>=size) in opened page(s)
// return: TFS_SUCCESS or TFS_ERR_NOSPACE or TFS_ERR_FORMAT
static uint16_t tfs_find_free_space(const tfs_t *self, uint16_t size,
                                    unsigned *addr, uint8_t *page)
{
  unsigned paddr = *addr = self->address;
  unsigned max_offset = self->page_size - TFS_HDR_SIZE;
  uint16_t max_size = 0;
  uint8_t pg, result = 0; // 0-not found, 1-found
  *page = 0;

  for (pg = 0; pg <= self->max_page; pg++)
  {
    uint16_t hdr = TFS_READ_WORD(paddr + TFS_OFF_SIG);
    if (hdr != TFS_SIG_FREE)
    { // page is not empty
      unsigned offset = 0;
      do { // while (offset < max_offset);
        unsigned ptr = paddr + offset; 
        uint16_t rsize;
        hdr   = TFS_READ_WORD(ptr + TFS_OFF_SIG);
        rsize = TFS_READ_WORD(ptr + TFS_OFF_SIZE);
        if (hdr == TFS_SIG_FREE)
        { // end of used EEPROM area in page
          unsigned free_space = self->page_size - (offset + TFS_HDR_SIZE);
          TFS_DBG("free_space=%u bytes on page %u", free_space, (unsigned) pg);
          if (size <= free_space && (result == 0 || max_size < free_space))
          { // free space size enougth
            result = 1;
            max_size = free_space;
            *addr = ptr;
            *page = pg;
          }
          break;
        }
        else if (hdr != TFS_SIG_DATA && hdr != TFS_SIG_DELETED)
        { // unknown (bad) hdr signature
          return TFS_ERR_FORMAT;
        }
        offset += TFS_ALIGN_OFF(rsize) + TFS_HDR_SIZE;
      } while (offset < max_offset);
    } // if (hdr..
    paddr += self->page_size;
  } // for (pg..
  
  if (result) return TFS_SUCCESS;

  // free space not found
  return TFS_ERR_NOSPACE;
}
//-----------------------------------------------------------------------------
// find pointer to full empty page 
// return: TFS_SUCCESS or TFS_ERR_NOTFOUND
static uint16_t tfs_find_empty_page(const tfs_t *self,
                                    unsigned *addr, uint8_t *page)
{
  unsigned paddr = *addr = self->address;
  uint8_t pg;
  *page = 0;
  
  for (pg = 0; pg <= self->max_page; pg++)
  {
    uint8_t result = 1; // 0-not found, 1-found (page empty)
    uint16_t i;
    for (i = 0; i < self->page_size; i += 2)
    {
      uint16_t word = TFS_READ_WORD(paddr + i);
      if (word != TFS_SIG_FREE)
      { // not empty page
        result = 0;
        break;
      }
    }
    if (result)
    { // full page empty
      *addr = paddr;
      *page = pg;
      return TFS_SUCCESS;
    }
    paddr += self->page_size;
  } // for (pg..
  return TFS_ERR_NOTFOUND;
}
//-----------------------------------------------------------------------------
// init TFS structure (initial set EEPROM space)
void tfs_init(tfs_t    *self,     // EEPROM space
              uint8_t  num_pages, // number of pages (1 or 2)
              unsigned address,   // base EEPROM address
              unsigned page_size) // size of page (bytes)
{
  self->address   = address;
  self->page_size = page_size;
  self->max_page  = num_pages <= 1 ? 0 : 1;
  TFS_DBG("init: num_pages=%u address=%u page_size=%u",
          (unsigned) (self->max_page + 1), address, page_size);
}
//-----------------------------------------------------------------------------
// erase all EEPROM space
// return code may be:
//   TFS_SUCCESS   - success erase EEPROM
//   TFS_ERR_ERASE - error erase EEPROM
uint16_t tfs_erase(const tfs_t *self)
{
  uint8_t pg;
  unsigned paddr = self->address;

  TFS_DBG("erase all (%u) " TFS_MEMTYPE " page(s) (page_size=%u)",
          (unsigned) (self->max_page + 1), self->page_size);

  for (pg = 0; pg <= self->max_page; pg++)
  {
    int8_t rv = TFS_ERASE(paddr, self->page_size);
    if (rv)
    {
      TFS_DBG("page %u erased on address=%u", (unsigned) pg, paddr);
      paddr += self->page_size;
    }
    else
    {
      TFS_DBG("error: can't erase page %u (retv=%i)", (unsigned) pg, (int) rv);
      return TFS_ERR_ERASE;
    }
  }

#ifdef TFS_COMMIT
  TFS_DBG("commit " TFS_MEMTYPE);
  TFS_COMMIT();
#endif

  return TFS_SUCCESS;
}
//-----------------------------------------------------------------------------
// get address (offset) to data record in EEPROM space and check sum
// return error code, address (offset) to data record in EEPROM space and record size
// return code:
//   TFS_SUCCESS                  - find record success
//   TFS_ERR_DELETED              - find only old deleted rrecord
//   TFS_ERR_NOTFOUND             - record not found
//   TFS_ERR_FORMAT               - corrupt TFS format (page(s) need erase) 
uint16_t tfs_get(const tfs_t *self,              // EEPROM space
                 unsigned *addr, uint16_t *size, // record region (offset, size)
                 uint16_t *cnt,                  // (re)write counter
                 uint16_t *cs)                   // check sum
{
  uint8_t page;
  uint16_t err = tfs_find_hdr(self, 0, addr, &page); // find any

  if ((err & (~TFS_ERR_DELETED)) == TFS_SUCCESS)
  { // found record (may be deleted)
    *cnt  = TFS_READ_WORD(*addr + TFS_OFF_CNT);
    *size = TFS_READ_WORD(*addr + TFS_OFF_SIZE);
    *cs   = TFS_READ_WORD(*addr + TFS_OFF_CS);
    TFS_DBG("record found (page=%u deleted=%u cnt=%u "
            "addr=%u size=%u cs=0x%04X err=0x%04X)",
            (unsigned) page,
            (unsigned) !!(err & TFS_ERR_DELETED),
            (unsigned) *cnt,
            (unsigned) *addr,
            (unsigned) *size,
            (unsigned) *cs,
            (unsigned) err);
  }
  else
  { // not fount any record or error
    TFS_DBG("record not found (err=0x%04X)", (unsigned) err);
    return err; // TFS_ERR_NOTFOUND or TFS_ERR_FORMAT
  }

  *addr += TFS_HDR_SIZE; // header offset -> data offset
  return err;
}
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
uint16_t tfs_read(const tfs_t *self,                  // EEPROM space
                  void *buffer, uint16_t buffer_size, // destination buffer
                  uint16_t *size,                     // size of record
                  uint16_t *cnt)                      // (re)write counter
{
  unsigned addr;;
  uint16_t cs, err = tfs_get(self, &addr, size, cnt, &cs);

  if ((err & (~TFS_ERR_DELETED)) != TFS_SUCCESS || *size == 0)
    return err; // error

  TFS_DBG("read data from EEPROM (address=%u, size=%u)", addr, (unsigned) *size);
  TFS_READ(addr, *size, buffer);

  if (cs != tfs_cs(buffer, *size))
  { // bad check sum 
    TFS_DBG("bad check sum!");
    err |= TFS_ERR_CS;
    if (!(err & TFS_ERR_DELETED))
    { // try to read deleted record as backup
      uint8_t d_page;
      uint16_t d_err;
      unsigned d_hdr;
      TFS_DBG("try to read deleted record as backup");

      d_err = tfs_find_hdr(self, 2, &d_hdr, &d_page);
      *cnt  = TFS_READ_WORD(d_hdr + TFS_OFF_CNT);
      *size = TFS_READ_WORD(d_hdr + TFS_OFF_SIZE);
      cs    = TFS_READ_WORD(d_hdr + TFS_OFF_CS);
  
      if (d_err == TFS_ERR_DELETED && *size != 0)
      { // found deleted record
        addr = d_hdr + TFS_HDR_SIZE;

        TFS_DBG("deleted record found (page=%i address=%u err=0x%04X)",
                d_page, d_hdr, (unsigned) d_err);

        TFS_DBG("read data from EEPROM (address=%u, size=%u)", addr, (unsigned) *size);
        TFS_READ(addr, *size, buffer);

        if (cs == tfs_cs(buffer, *size))
        { // check sum of deleted record is OK
          TFS_DBG("check sum of deleted record is OK");
          err &= ~TFS_ERR_CS;
          err |= TFS_ERR_DELETED;
        }
      }
      else
      {
        TFS_DBG("deleted record not found (err=0x%04X)", (unsigned) d_err);
        *cnt = 0;
        *size = 0;
      }
    }
  }

  return err;
}
//-----------------------------------------------------------------------------
// write record to EEPROM space (mega funcion)
// return code:
//   TFS_SUCCESS    - success write record to EEPROM space
//   TFS_ERR_TOOBIG - too big record size
//   TFS_ERR_ERASE  - erase error
//   TFS_ERR_WRITE  - write error
//   TFS_ERR_VERIFY - verify error
uint16_t tfs_write(const tfs_t *self,               // EEPROM space
                   const void *data, uint16_t size) // record source
{
  uint16_t retv, err = TFS_SUCCESS; // return values
  int8_t rv;                        //
  uint8_t pg; // page index 0..1

  uint16_t hdr[TFS_HDR_SIZE / sizeof(uint16_t)]; // hedaer of new record
  
  unsigned last_hdr;      // pointer to last record header
  uint8_t  last_page = 0; // last record page index {0|1}

  uint16_t cnt = 0; // last record (re)write counter

  uint8_t found  = 0;  // last record found flag {0|1} 
  uint8_t delete = 0;  // delete last record flag {0|1}
  uint8_t close  = 0;  // last record page close flag {0|1}

  uint8_t erase[2] = {0, 0}; // erase page flags {0|1}

  unsigned free_hdr = 0; // pointer to free space in opened page
  uint8_t free_page = 0; // free space page index {0|1}

  if (size > (self->page_size - TFS_HDR_SIZE))
  {
    size = self->page_size - TFS_HDR_SIZE; // trancate size
    err = TFS_ERR_TOOBIG;
  }

  // find last record
  retv = tfs_find_hdr(self, 0, &last_hdr, &last_page);
  if ((retv & ~TFS_ERR_DELETED) == TFS_SUCCESS)
  { // old record found
    found = 1;
    delete = !(retv & TFS_ERR_DELETED); // mark to delete record
    cnt = TFS_READ_WORD(last_hdr + TFS_OFF_CNT);
    TFS_DBG("old record found (page=%u deleted=%u cnt=%u address=%u)",
            (unsigned) last_page, (unsigned) !delete,
            (unsigned) cnt, last_hdr);
    cnt++;
  }
  else if (retv == TFS_ERR_FORMAT)
  { // format error => page must be erased
    erase[0] = erase[1] = 1;
    TFS_DBG("format error => all page(s) must be erased");
  }

  // find pointer to maximal free space in opened page(s)
  TFS_DBG("find pointer to maximal free space in opened page(s)");
  retv = tfs_find_free_space(self, size, &free_hdr, &free_page);
  if (retv == TFS_ERR_NOSPACE)
  { // no free space in opened page(s) => try to find empty page
    TFS_DBG("no free space in opened page(s) => try to find empty page");
    retv = tfs_find_empty_page(self, &free_hdr, &free_page);
    if (retv == TFS_SUCCESS)
    { // empty page found
      TFS_DBG("empty page found");
      close = found;
    }
    else // retv == TFS_ERR_NOTFOUND
    { // empty page not found => erase page for write
      TFS_DBG("empty page not found => must erase page %u for write",
              (unsigned) free_page);
      free_page = (last_page ^ 1) & self->max_page;
      erase[free_page] = 1;
      free_hdr = self->address + free_page * self->page_size;
      close = free_page ^ last_page;
      delete &= close;
    }
  }
  else if (retv == TFS_ERR_FORMAT)
  { // format error => page(s) must be erased
    TFS_DBG("format error => page(s) must be erased");
    erase[0] = erase[1] = 1;
    free_hdr = self->address;
    free_page = close = delete = 0;
  }

  TFS_DBG("start modify " TFS_MEMTYPE);

  if (delete)
  { // delete record
    uint16_t rd, wr = TFS_SIG_DELETED;
    TFS_DBG("delete record from page %u", (unsigned) last_page);
    TFS_WRITE(last_hdr, sizeof(uint16_t), &wr);
    TFS_READ( last_hdr, sizeof(uint16_t), &rd);
    if (wr != rd)
    {
      TFS_DBG("error: cant't delete record");
      err |= TFS_ERR_WRITE;
      goto unlock_exit;
    }
  }

  // erase page(s) for write
  for (pg = 0; pg <= self->max_page; pg++)
  {
    if (erase[pg])
    {
      unsigned address = self->address + pg * self->page_size;
      TFS_DBG("erase page %u for write", (unsigned) pg);
      rv = TFS_ERASE(address, self->page_size);
      if (!rv) goto error_cant_erase;
    }
  }

  // write record
  TFS_DBG("write record to page %u (address=%u size=%u cnt=%u)",
          (unsigned) free_page, (unsigned) free_hdr,
          (unsigned) size, (unsigned) cnt);

  hdr[TFS_OFF_SIG  / sizeof(uint16_t)] = TFS_SIG_DATA;
  hdr[TFS_OFF_CNT  / sizeof(uint16_t)] = cnt;
  hdr[TFS_OFF_SIZE / sizeof(uint16_t)] = size;
  hdr[TFS_OFF_CS   / sizeof(uint16_t)] = tfs_cs(data, size);

  TFS_WRITE(free_hdr, TFS_HDR_SIZE, hdr);
  if (!TFS_VERIFY(free_hdr, TFS_HDR_SIZE, hdr))
  {
    TFS_DBG("error: cant't write header (TFS_HDR_SIZE=%u)",
            (unsigned) TFS_HDR_SIZE);
    err |= TFS_ERR_WRITE;
    goto unlock_exit;
  }

  TFS_WRITE(free_hdr + TFS_HDR_SIZE, size, data);
  if (!TFS_VERIFY(free_hdr + TFS_HDR_SIZE, size, data))
  { 
    TFS_DBG("error: cant't write data (size=%u)", (unsigned) size);
    err |= TFS_ERR_WRITE;
    goto unlock_exit;
  }


#ifdef TFS_DEBUG
  // show free space for debug
  if (1)
  {
    int free_space = 
         (int) (self->address + self->page_size * (free_page + 1)) -
         (int) (free_hdr + TFS_HDR_SIZE) -
	       (int) size;
    if (free_space > 0)
      TFS_DBG("free_space=%i on page %u", (int) free_space, (unsigned) free_page);
    else
      TFS_DBG("page %i is full", (int) free_page);
  }
#endif
  
  // verify
  if (1)
  {
    uint8_t page;
    unsigned ptr;
    retv = tfs_find_hdr(self, 1, &ptr, &page);
    if (retv != TFS_SUCCESS)
    {
      TFS_DBG("error: cant't find record");
      err |= TFS_ERR_VERIFY;
      goto unlock_exit;
    }

    if (!TFS_VERIFY(ptr, TFS_HDR_SIZE, hdr))
    {
      TFS_DBG("error: verify header fail");
      err |= TFS_ERR_VERIFY;
      goto unlock_exit;
    }

    if (!TFS_VERIFY(ptr + TFS_HDR_SIZE, size, data))
    {
      TFS_DBG("error: verify data fail");
      err |= TFS_ERR_VERIFY;
      goto unlock_exit;
    }
  }

  TFS_DBG("verify success");

  if (close)
  { // erase closed page
    unsigned address = self->address + last_page * self->page_size;
    TFS_DBG("erase closed page %u", (unsigned) last_page);
    rv = TFS_ERASE(address, self->page_size);
    if (!rv)
    {
error_cant_erase:
      TFS_DBG("error: can't erase page");
      err |= TFS_ERR_ERASE;
      goto unlock_exit;
    }
  }

#ifdef TFS_COMMIT
  TFS_DBG("commit " TFS_MEMTYPE);
  TFS_COMMIT();
#endif

  return err;
      
unlock_exit:
  TFS_DBG("write error (err=0x%04X)", (unsigned) err);
  return err;
}
//-----------------------------------------------------------------------------
// delete record in EEPROM space (mark record as deleted)
// return code:
//   TFS_SUCCESS      - success write record to EEPROM space
//   TFS_ERR_NOTFOUND - record not found
//   TFS_ERR_WRITE    - write EEPROM error
uint16_t tfs_delete(const tfs_t *self)
{
  unsigned hdr;
  uint8_t  page;
  uint16_t err = tfs_find_hdr(self, 1, &hdr, &page);
  
  if (err == TFS_SUCCESS)
  {
    uint16_t rd, wr = TFS_SIG_DELETED;

    TFS_DBG("delete record from page %u", (unsigned) page);
    TFS_WRITE(hdr, sizeof(uint16_t), &wr);
    TFS_READ( hdr, sizeof(uint16_t), &rd);

    if (rd != wr)
    {
      TFS_DBG("error: cant't delete record");
      return TFS_ERR_WRITE;
    }

#ifdef TFS_COMMIT
    TFS_DBG("commit " TFS_MEMTYPE);
    TFS_COMMIT();
#endif
  }

  return err;
}
//-----------------------------------------------------------------------------

/*** end of "tfs.c" file ***/

