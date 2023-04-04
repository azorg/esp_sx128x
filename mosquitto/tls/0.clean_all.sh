#!/bin/bash

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

rm -rf ca
rm -rf server
rm -rf client

cd "$OLDPWD"

