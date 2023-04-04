
# There's no (simple) way to pass subjAltName on the CLI so
# create a temp *.cnf file and get it to openssl
# prepare Subject AltNames (SAN) extension config and options
# return: tmp.cnf altnames.txt
san_cnf_opt() {
  CNF=`mktemp || echo /tmp/xxx_temp_openssl.cnf` # openssl Config
  EXF=`mktemp || echo /tmp/xxx_temp_openssl.exf` # AltNames list
  LIST=''
  for FQDN in $*
  do
    [ "$LIST" ] && LIST="${LIST}, DNS:${FQDN}" \
                || LIST="DNS:${FQDN}"
  done

  if [ "$LIST" ]
  then
    echo "subjectAltName=$LIST" > "$EXF"

    cat  "/etc/ssl/openssl.cnf" > "$CNF"

    # 1. subjAltName
    echo "[SAN]"                >> "$CNF"
    echo "subjectAltName=$LIST" >> "$CNF"
    
    # 2. DNS.n
    #echo "[req]"                       >> "$CNF"
    #echo "req_extensions = req_ext"    >> "$CNF"
    echo "[req_ext]"                   >> "$CNF"
    echo "subjectAltName = @alt_names" >> "$CNF"
    echo "[alt_names]"                 >> "$CNF"
    CNT=1
    for FQDN in $*
    do
      echo "DNS.${CNT} = ${FQDN}"      >> "$CNF"
      CNT=$(($CNT + 1))
    done

    chmod a+r "$EXF" # for openssl access
    chmod a+r "$CNF" # for openssl access

    echo "$EXF" "$CNF"
  else
    rm "$CNF"
    rm "$EXF"
  fi
}

