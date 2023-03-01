/*
 * File: "cli.h"
 */

#pragma once
#ifndef CLI_H
#define CLI_H
//-----------------------------------------------------------------------------
#include "config.h"
#include "mrl.h"
//-----------------------------------------------------------------------------
//#define CLI_HELP
//-----------------------------------------------------------------------------
// command/options description structure
typedef struct cli_cmd_ cli_cmd_t;

struct cli_cmd_ {
  int16_t id;        // ID in tree (>=0)
  int16_t parent_id; // parent ID for options (or -1 for root)
  void (*fn)(int argc, char* const argv[], const cli_cmd_t*); // callback function
  const char *name;  // command/option name
#ifdef CLI_HELP
  const char *args;  // arguments for help
  const char *help;  // help (description) string 
#endif // CLI_HELP
};

extern cli_cmd_t const cli_tree[];
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
void cli_init();
void cli_loop();
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // CLI_H

/*** end of "cli.h" file ***/




