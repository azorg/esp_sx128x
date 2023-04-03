#!/bin/bash
#
# Generate MQTT server keys

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh

echo "*** Generate a server key pair (server.key)"
mkdir -p server
#openssl genrsa -des3 -out server/server.key 2048
openssl genrsa -out server/server.key 2048

echo "*** Generate a certificate signing request (CSR) to send to the CA (server.csr)"
openssl req -new -sha256 \
        -subj "/CN=$SERVER_CN/O=$SERVER_O/OU=$SERVER_OU/emailAddress=$SERVER_EMAIL" \
        -key server/server.key \
        -out server/server.csr

echo "*** View server.csr:"
openssl req -noout -text -in server/server.csr

echo "*** 'Send' the server CSR to the CA, or sign it with your CA key (make server.crt)"
openssl x509 -req -days $SERVER_DAYS \
        -CAcreateserial \
        -CA ca/ca.crt -CAkey ca/ca.key \
        -in  server/server.csr \
        -out server/server.crt

#echo "*** Delete server.csr"
#rm server/server.csr

echo "*** View server.crt:"
#openssl x509 -noout -text -in server/server.crt
openssl x509 -in server/server.crt -nameopt multiline -subject -noout

certinfo server/server.crt

cd "$OLDPWD"

