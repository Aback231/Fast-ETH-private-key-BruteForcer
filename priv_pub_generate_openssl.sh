#!/bin/bash

# arch install xxd-standalone from AUR
# to call it: printf privateKey | ./priv_pub_generate_openssl.sh

#privateKey="privateKey"
privateKey=""
wdir=$(pwd)

function clean_up() {
    cd "$wdir/tmp/"
    rm *bin "$wdir/tmp/template.der" "$wdir/tmp/private_key.der" &> /dev/null
    cd
}

clean_up

function generate_addr_from_private_key() {
openssl ecparam -name secp256k1 -genkey -noout -outform DER | \
     openssl ec -inform DER -no_public -outform DER -out "$wdir/tmp/template.der"

head -c 7 "$wdir/tmp/template.der" > "$wdir/tmp/header.bin"

echo $privateKey | xxd -r -p > "$wdir/tmp/key.bin"

tail -c +40 "$wdir/tmp/template.der" > "$wdir/tmp/footer.bin"

cat "$wdir/tmp/header.bin" "$wdir/tmp/key.bin" "$wdir/tmp/footer.bin" > "$wdir/tmp/private_key.der"

cat "$wdir/tmp/private_key.der" | openssl asn1parse -in - -inform DER

keys=$(openssl ec -check -inform DER -in "$wdir/tmp/private_key.der" -noout -text)

# extract private key in hex format, removing newlines, leading zeroes and semicolon 
priv=$(printf "%s\n" $keys | grep priv -A 3 | tail -n +2 | tr -d '\n[:space:]:' | sed 's/^00//')

# extract public key in hex format, removing newlines, leading '04' and semicolon 
pub=$(printf "%s\n" $keys | grep pub -A 5 | tail -n +2 | tr -d '\n[:space:]:' | sed 's/^04//')

# get the keecak hash, removing the trailing ' -' and taking the last 40 chars
# https://github.com/maandree/sha3sum
#addr=0x$(echo $pub | "/home/aback/c++ projects/Brute force list creator/Brute force all posibillities/keccak-256sum" -x -l | tr -d ' -' | tail -c 41)
addr=0x$(echo $pub | "$wdir/keccak-256sum" -x -l | tr -d ' -' | tail -c 41)

echo 'Private key:' $priv
echo 'Public key:' $pub
echo 'Address:' $addr

clean_up
}

echo "Enter the privateKey: "
read privateKey
generate_addr_from_private_key
