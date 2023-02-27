/*
 * File: "limit.h"
 */

#pragma once
#ifndef LIMIT_H
#define LIMIT_H
//-----------------------------------------------------------------------------
// limit arguments
#define LIMIT(x, min, max) \
  ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define LIMIT_ABS(x, max) \
  ((x) > (max) ? (max) : ((x) < (-max) ? (-max) : (x)))

#define LIMIT_UP(x, max) ((x) <= (max) ? (x) : (max))

#define LIMIT_DOWN(x, min) ((x) >= (min) ? (x) : (min))

// return minimum
#define LIMIT_MIN(x, y) ((x) < (y) ? (x) : (y))

// return maximum
#define LIMIT_MAX(x, y) ((x) > (y) ? (x) : (y))
//-----------------------------------------------------------------------------
#endif // LIMIT_H

/*** end of "limit.h" file ***/

