#!/bin/bash
#
# Generate MQTT server keys

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh
. ./func.sh

echo
echo "*** Generate a server key pair (server.key)"
mkdir -p server
#openssl genrsa -des3 -out server/server.key 2048
openssl genrsa -out server/server.key 2048

echo
echo "*** Generate a certificate signing request (CSR) to send to the CA (server.csr)"
read SAN_CNF SAN_OPT < <(san_cnf_opt $SERVER_SAN_DNS)
openssl req -new -sha256 \
        -subj "/CN=$SERVER_CN/O=$SERVER_O/OU=$SERVER_OU/emailAddress=$SERVER_EMAIL" \
        $SAN_OPT \
        -key server/server.key \
        -out server/server.csr
[ "$SAN_CNF" ] && rm "$SAN_CNF"

echo
echo "*** View server.csr:"
openssl req -in server/server.csr -noout -text 

echo
echo "*** 'Send' the server CSR to the CA, or sign it with your CA key (make server.crt)"
openssl x509 -req -days $SERVER_DAYS \
        -CAcreateserial \
        -CAkey ca/ca.key \
        -CA    ca/ca.crt \
        -in  server/server.csr \
        -out server/server.crt

#echo
#echo "*** Delete server.csr"
#rm server/server.csr

echo
echo "*** View server.crt:"
openssl x509 -in server/server.crt -nameopt multiline -subject -noout

echo
echo "*** certinfo server.crt:"
certinfo server/server.crt

cd "$OLDPWD"

