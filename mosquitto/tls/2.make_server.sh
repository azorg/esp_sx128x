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
read EXF CNF < <(san_cnf_opt $SERVER_SAN_DNS)
if [ "$EXF" ]
then
  OPT_ALT_NAMES=`cat $EXF`
  openssl req -new -sha256 \
          -subj "/CN=$SERVER_CN/O=$SERVER_O/OU=$SERVER_OU/emailAddress=$SERVER_EMAIL" \
          -reqexts SAN -reqexts req_ext -config $CNF -addext "$OPT_ALT_NAMES" \
          -key server/server.key \
          -out server/server.csr
else
  openssl req -new -sha256 \
          -subj "/CN=$SERVER_CN/O=$SERVER_O/OU=$SERVER_OU/emailAddress=$SERVER_EMAIL" \
          -key server/server.key \
          -out server/server.csr
fi

echo
echo "*** View server.csr:"
openssl req -in server/server.csr -noout -text 

echo
echo "*** 'Send' the server CSR to the CA, or sign it with your CA key (make server.crt)"
[ "$EXF" ] && OPT="-extfile $EXF" || OPT=""
openssl x509 -req -days $SERVER_DAYS \
        $OPT \
        -CAcreateserial \
        -CAkey ca/ca.key \
        -CA    ca/ca.crt \
        -in  server/server.csr \
        -out server/server.crt

[ "$EXF" ] && rm "$EXF"
[ "$CNF" ] && rm "$CNF"

#echo
#echo "*** Delete server.csr"
#rm server/server.csr

echo
echo "*** View server.crt:"
openssl x509 -in server/server.crt -nameopt multiline -noout -text

echo
echo "*** certinfo server.crt:"
certinfo server/server.crt

cd "$OLDPWD"

