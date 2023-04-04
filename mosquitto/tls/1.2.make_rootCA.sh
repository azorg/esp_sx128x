#!/bin/bash
#
# Generate Root CA (self signed)


OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh
. ./func.sh

NODES="-nodes"

echo
echo "*** Generate the root sertificate authority's (CA) SELF SIGNED (ca.key, ca.crt)"
read SAN_CNF SAN_OPT < <(san_cnf_opt $CA_SAN_DNS)
mkdir -p ca
openssl req -new -x509 -days $CA_DAYS -extensions v3_ca $NODES \
        -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
        $SAN_OPT \
        -keyout ca/ca.key \
        -out    ca/ca.crt
[ "$SAN_CNF" ] && rm "$SAN_CNF"

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo
echo "*** View CA certificate (ca.crt):"
openssl x509 -in ca/ca.crt -nameopt multiline -subject -noout

echo
echo "*** certinfo ca.crt:"
certinfo ca/ca.crt

cd "$OLDPWD"

