#!/bin/sh

echo "*** CA:"
openssl rsa -text -in ca/ca.key

echo
echo "*** Server:"
openssl rsa -text -in server/server.key

echo
echo "*** Client:"
openssl rsa -text -in client/client.key


