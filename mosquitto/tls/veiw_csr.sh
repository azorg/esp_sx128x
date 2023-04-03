#!/bin/sh

echo "*** Server:"
openssl req -in server/server.csr -noout -text

echo
echo "*** Client:"
openssl req -in client/client.csr -noout -text


