#!/bin/bash
#
# Generate Root CA (self signed)


OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh
. ./func.sh

NODES="-nodes" # off encription

echo
echo "*** Generate the root sertificate authority's (CA) SELF SIGNED (ca.key, ca.crt)"
mkdir -p ca
read EXF CNF < <(san_cnf_opt $CA_SAN_DNS)
if [ "$EXF" ]
then
  OPT_ALT_NAMES=`cat $EXF`
  openssl req -new -x509 -newkey rsa:2048 -days $CA_DAYS -extensions v3_ca $NODES \
          -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
          -reqexts SAN -reqexts req_ext -config "$CNF" -addext "$OPT_ALT_NAMES" \
          -keyout ca/ca.key \
          -out    ca/ca.crt
else
  openssl req -new -x509 -newkey rsa:2048 -days $CA_DAYS -extensions v3_ca $NODES \
          -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
          -keyout ca/ca.key \
          -out    ca/ca.crt
fi

[ "$EXF" ] && rm "$EXF"
[ "$CNF" ] && rm "$CNF"

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo
echo "*** View CA certificate (ca.crt):"
openssl x509 -in ca/ca.crt -nameopt multiline -noout -text

echo
echo "*** certinfo ca.crt:"
certinfo ca/ca.crt

cd "$OLDPWD"

