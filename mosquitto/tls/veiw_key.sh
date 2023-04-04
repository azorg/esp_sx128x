#!/bin/sh

echo "*** CA:"
openssl rsa -in ca/ca.key -noout -text

echo
echo "*** Server:"
openssl rsa -in server/server.key -noout -text

echo
echo "*** Client:"
openssl rsa -in client/client.key -noout -text


