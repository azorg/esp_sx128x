#!/bin/bash

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh

DATA=""

while read LINE
do
  [ "$DATA" ] && DATA="${DATA} \\\\\n"
  DATA="${DATA}  \"$LINE\\\\n\""
done < ca/ca.crt

CODE=`cat << EOF
/*
 * Root CA
 * File: "root_ca.h"
 */

#pragma once
#ifndef MQTT_ROOT_CA_H
#define MQTT_ROOT_CA_H
//-----------------------------------------------------------------------------
#define $CA_NAME \\\\
$DATA
//-----------------------------------------------------------------------------
#endif MQTT_ROOT_CA_H

/*** end of "root_ca.h" file ***/\n
EOF`

echo -e "$CODE" > ca/ca_root.h

cd "$OLDPWD"

