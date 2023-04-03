#!/bin/bash
#
# Generate Root CA (self signed)


OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh

NODES="-nodes"

echo "*** Generate the root sertificate authority's (CA) SELF SIGNED (ca.key, ca.crt)"
mkdir -p ca
openssl req -new -x509 -days $CA_DAYS -extensions v3_ca $NODES \
        -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
        -keyout ca/ca.key -out ca/ca.crt

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo "*** View CA certificate (ca.crt):"
openssl x509 -in ca/ca.crt -nameopt multiline -subject -noout

certinfo ca/ca.crt

cd "$OLDPWD"

