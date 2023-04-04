#!/bin/sh

if [ -f ca/ca.csr ]
then
  echo "*** CA:"
  openssl req -in ca/ca.csr -noout -text
  echo
fi

echo "*** Server:"
openssl req -in server/server.csr -noout -text

echo
echo "*** Client:"
openssl req -in client/client.csr -noout -text


