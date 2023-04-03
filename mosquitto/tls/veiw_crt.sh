#!/bin/sh

echo "*** CA:"
openssl x509 -in ca/ca.crt -noout -text

echo
echo "*** Server:"
openssl x509 -in server/server.crt -noout -text

echo
echo "*** Client:"
openssl x509 -in client/client.crt -noout -text

