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
read SAN_CNF SAN_OPT < <(san_cnf_opt $CA_SAN_DNS)
openssl req -new -sha256 -extensions v3_ca \
        -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
        -reqexts SAN -config <(cat "$SAN_CNF") \
        $SAN_OPT \
        -key ca/ca.key \
        -out ca/ca.csr
#cat "$SAN_CNF"
[ "$SAN_CNF" ] && rm "$SAN_CNF"

echo
echo "*** View ca.csr:"
openssl req -in ca/ca.csr -noout -text 

echo
echo "*** Create the root CA's SELF SIGNED certificate (ca.crt)"
openssl x509 -req -days $CA_DAYS -sha256 \
        -in ca/ca.csr \
        -signkey ca/ca.key \
        -out ca/ca.crt 

#echo "*** Delete ca.csr"
#rm ca/ca.csr

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo
echo "*** View CA certificate (ca.crt):"
openssl x509 -in ca/ca.crt -nameopt multiline -subject -noout

echo
echo "*** certinfo ca.crt:"
certinfo -expiry -chains ca/ca.crt

cd "$OLDPWD"

