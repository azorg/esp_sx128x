#!/bin/bash

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

rm -f ca/ca.srl
rm -f ca/ca.csr

rm -f server/server.csr

rm -f client/client.csr

cd "$OLDPWD"

