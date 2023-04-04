#!/bin/bash
#
# Generate MQTT client keys

OLDPWD=`pwd`
WDIR=`dirname $0`
cd "$WDIR"

. ./config.sh
. ./func.sh

echo
echo "*** Generate a client key pair (client.key)"
mkdir -p client
#openssl genrsa -des3 -out client/client.key 2048
openssl genrsa -out client/client.key 2048

echo
echo "*** Generate a certificate signing request (CSR) to send to the CA (client.csr)"
read SAN_CNF SAN_OPT < <(san_cnf_opt $CLIENT_SAN_DNS)
openssl req -new -sha256 \
        -subj "/CN=$CLIENT_CN/O=$CLIENT_O/OU=$CLIENT_OU" \
        $SAN_OPT \
        -key client/client.key \
        -out client/client.csr
[ "$SAN_CNF" ] && rm "$SAN_CNF"

echo
echo "*** View client.csr:"
openssl req -in client/client.csr -noout -text 

echo
echo "*** 'Send' the client CSR to the CA, or sign it with your CA key (make client.crt)"
openssl x509 -req -days $CLIENT_DAYS \
        -CAcreateserial \
        -CAkey ca/ca.key \
        -CA    ca/ca.crt \
        -in  client/client.csr \
        -out client/client.crt

#echo
#echo "*** Delete client.csr"
#rm client/client.csr

echo
echo "*** View client.crt:"
openssl x509 -in client/client.crt -nameopt multiline -subject -noout

echo
echo "*** certinfo client.crt:"
certinfo client/client.crt

cd "$OLDPWD"


