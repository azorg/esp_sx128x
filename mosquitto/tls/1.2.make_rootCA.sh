#!/bin/bash
#
# Generate Root CA (self signed)

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh
. ./func.sh

echo
echo "*** Generate the root certificate authority's (CA) signing key (ca.key)"
mkdir -p ca
#openssl genrsa -des3 -out ca/ca.key 2048
openssl genrsa -out ca/ca.key 2048

echo
echo "*** Generate a certificate signing request for the root CA (ca.csr)"
read EXF CNF < <(san_cnf_opt $CA_SAN_DNS)
if [ "$EXF" ]
then
  OPT_ALT_NAMES=`cat $EXF`
  openssl req -new -sha256 -extensions v3_ca \
          -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
          -reqexts SAN -reqexts req_ext -config $CNF -addext "$OPT_ALT_NAMES" \
          -key ca/ca.key \
          -out ca/ca.csr
else
  openssl req -new -sha256 -extensions v3_ca \
          -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
          -key ca/ca.key \
          -out ca/ca.csr
fi

echo
echo "*** View ca.csr:"
openssl req -in ca/ca.csr -noout -text 

echo
echo "*** Create the root CA's SELF SIGNED certificate (ca.crt)"
[ "$EXF" ] && OPT="-extfile $EXF" || OPT=""
openssl x509 -req -days $CA_DAYS -sha256 \
        $OPT \
        -signkey ca/ca.key \
        -in      ca/ca.csr \
        -out     ca/ca.crt 

[ "$EXF" ] && rm "$EXF"
[ "$CNF" ] && rm "$CNF"

#echo "*** Delete ca.csr"
#rm ca/ca.csr

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo
echo "*** View CA certificate (ca.crt):"
openssl x509 -in ca/ca.crt -nameopt multiline -noout -text

echo
echo "*** certinfo ca.crt:"
certinfo -expiry -chains ca/ca.crt

cd "$OLDPWD"

