#!/bin/bash
#
# Generate Root CA (self signed)

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh

echo "*** Generate the root certificate authority's (CA) signing key (ca.key)"
mkdir -p ca
#openssl genrsa -des3 -out ca/ca.key 2048
openssl genrsa -out ca/ca.key 2048

echo "*** Generate a certificate signing request for the root CA (ca.csr)"
openssl req -new -sha256 \
        -subj "/CN=$CA_CN/O=$CA_O/OU=$CA_OU/emailAddress=$CA_EMAIL" \
        -key ca/ca.key -out ca/ca.csr

echo "*** View ca.csr:"
openssl req -noout -text -in ca/ca.csr

echo "*** Create the root CA's SELF SIGNED certificate (ca.crt)"
openssl x509 -req -days $CA_DAYS -sha256 \
        -in ca/ca.csr \
        -signkey ca/ca.key \
        -out ca/ca.crt 

#echo "*** Delete ca.csr"
#rm ca/ca.csr

chmod 400 ca/ca.key
chmod 444 ca/ca.crt

echo "*** View CA certificate (ca.crt):"
#openssl x509 -noout -text -in ca/ca.crt
openssl x509 -in ca/ca.crt -nameopt multiline -subject -noout

certinfo ca/ca.crt

cd "$OLDPWD"

