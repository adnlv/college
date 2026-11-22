#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="$SCRIPT_DIR/work"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

echo "=== TASK 1: Environment & Input Preparation (Variant 6) ==="
openssl version
python3 --version

printf 'VARIANT=06\nASSET=helios-web\nEVENT=Web service restarted during planned change.\nTOKEN=F2U7\n' > data.txt
printf 'HELIOS|restart|F2U7\n' > msg.txt
printf 'Lab-HELIOS!46\n' > password.txt
chmod 600 password.txt

wc -l -c data.txt msg.txt password.txt
ls -l data.txt msg.txt password.txt

echo ""
echo "=== TASK 2: SHA-2 Hashing & Avalanche Effect ==="
openssl dgst -sha256 data.txt
openssl dgst -sha512 data.txt
cp data.txt data_changed.txt
printf '!' >> data_changed.txt
openssl dgst -sha256 data_changed.txt
wc -c data.txt data_changed.txt
cmp data.txt data_changed.txt || true

echo ""
echo "=== TASK 3: Random Bytes & HMAC-SHA-256 ==="
openssl rand -hex 16 > rand16.hex
openssl rand -hex 32 > hmac.key
chmod 600 hmac.key
printf "rand16.hex: "; cat rand16.hex
printf "hmac.key hex chars: "; tr -d '\n' < hmac.key | wc -c

HMAC_KEY="$(cat hmac.key)"
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" data.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" data_changed.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out data.hmac data.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out data_changed.hmac data_changed.txt
wc -c data.hmac data_changed.hmac
cmp data.hmac data_changed.hmac || true

echo ""
echo "=== TASK 4: Symmetric Encryption AES-256-CBC ==="
openssl enc -aes-256-cbc -salt -pbkdf2 -iter 200000 -md sha256 -in data.txt -out aes_cbc.bin -pass file:password.txt
openssl enc -d -aes-256-cbc -pbkdf2 -iter 200000 -md sha256 -in aes_cbc.bin -out data_dec.txt -pass file:password.txt
if cmp -s data.txt data_dec.txt; then
    echo "cmp data.txt data_dec.txt: MATCH"
fi
openssl enc -aes-256-cbc -salt -pbkdf2 -iter 200000 -md sha256 -in data.txt -out aes_cbc2.bin -pass file:password.txt
cmp aes_cbc.bin aes_cbc2.bin || true
wc -c data.txt aes_cbc.bin aes_cbc2.bin data_dec.txt
python3 - <<'PY'
for name in ("aes_cbc.bin", "aes_cbc2.bin"):
    with open(name, "rb") as f:
        hdr = f.read(16)
    print(f"{name} header={hdr[:8]!r}, salt={hdr[8:16].hex()}")
PY

echo ""
echo "=== TASK 5: AES-256-CTR & Ciphertext Tampering ==="
openssl enc -aes-256-ctr -salt -pbkdf2 -iter 200000 -md sha256 -in data.txt -out aes_ctr.bin -pass file:password.txt
openssl enc -d -aes-256-ctr -pbkdf2 -iter 200000 -md sha256 -in aes_ctr.bin -out data_ctr_dec.txt -pass file:password.txt
if cmp -s data.txt data_ctr_dec.txt; then
    echo "cmp data.txt data_ctr_dec.txt: MATCH"
fi
python3 "$SCRIPT_DIR/flip_bit.py" aes_ctr.bin aes_ctr_tampered.bin
openssl enc -d -aes-256-ctr -pbkdf2 -iter 200000 -md sha256 -in aes_ctr_tampered.bin -out data_ctr_tampered.txt -pass file:password.txt
echo "Tampered CTR decryption exit code: $?"
cmp data.txt data_ctr_tampered.txt || true
wc -c data.txt aes_ctr.bin aes_ctr_tampered.bin data_ctr_tampered.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" aes_ctr.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" aes_ctr_tampered.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out aes_ctr.hmac aes_ctr.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out aes_ctr_tampered.hmac aes_ctr_tampered.bin
cmp aes_ctr.hmac aes_ctr_tampered.hmac || true

echo ""
echo "=== TASK 6: RSA Key Generation (2048-bit) ==="
RSA_BITS=2048
openssl genpkey -algorithm RSA -pkeyopt "rsa_keygen_bits:$RSA_BITS" -out rsa_priv.pem
chmod 600 rsa_priv.pem
openssl pkey -in rsa_priv.pem -pubout -out rsa_pub.pem
openssl pkey -in rsa_priv.pem -check -noout
openssl pkey -pubin -in rsa_pub.pem -text -noout
ls -l rsa_priv.pem rsa_pub.pem

echo ""
echo "=== TASK 7: Asymmetric Encryption RSA-OAEP ==="
openssl pkeyutl -encrypt -pubin -inkey rsa_pub.pem -in msg.txt -out msg.enc \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
openssl pkeyutl -decrypt -inkey rsa_priv.pem -in msg.enc -out msg_dec.txt \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
if cmp -s msg.txt msg_dec.txt; then
    echo "cmp msg.txt msg_dec.txt: MATCH"
fi
wc -c msg.txt msg.enc msg_dec.txt

echo ""
echo "=== TASK 8: Digital Signature RSA-PSS ==="
openssl dgst -sha256 -sign rsa_priv.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out data.rsa.sig data.txt
openssl dgst -sha256 -verify rsa_pub.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -signature data.rsa.sig data.txt
openssl dgst -sha256 -verify rsa_pub.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -signature data.rsa.sig data_changed.txt || true
openssl dgst -sha256 -sign rsa_priv.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out data2.rsa.sig data.txt
cmp data.rsa.sig data2.rsa.sig || true
wc -c data.rsa.sig data2.rsa.sig

echo ""
echo "=== TASK 9: EC Keys & ECDSA (secp384r1 / SHA-384) ==="
EC_CURVE=secp384r1
EC_HASH=sha384
openssl genpkey -algorithm EC -pkeyopt "ec_paramgen_curve:$EC_CURVE" -out ec_priv.pem
chmod 600 ec_priv.pem
openssl pkey -in ec_priv.pem -pubout -out ec_pub.pem
openssl pkey -pubin -in ec_pub.pem -text -noout
openssl dgst "-$EC_HASH" -sign ec_priv.pem -out data.ec.sig data.txt
openssl dgst "-$EC_HASH" -verify ec_pub.pem -signature data.ec.sig data.txt
openssl dgst "-$EC_HASH" -verify ec_pub.pem -signature data.ec.sig data_changed.txt || true
wc -c data.rsa.sig data.ec.sig

echo ""
echo "=== TASK 10: Hybrid Scheme (AES-256-CTR + RSA-OAEP + HMAC-SHA-256) ==="
openssl rand -hex 32 > aes.key
openssl rand -hex 32 > mac.key
openssl rand -hex 16 > iv.hex
chmod 600 aes.key mac.key
cat aes.key mac.key > bundle.txt
chmod 600 bundle.txt
wc -l -c bundle.txt iv.hex
printf "iv.hex: "; cat iv.hex

openssl pkeyutl -encrypt -pubin -inkey rsa_pub.pem -in bundle.txt -out bundle.rsa \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
openssl enc -aes-256-ctr -K "$(cat aes.key)" -iv "$(cat iv.hex)" -in data.txt -out hybrid.enc
cat iv.hex hybrid.enc > hybrid.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac.key)" hybrid.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac.key)" -binary -out hybrid.hmac hybrid.authdata
wc -c bundle.rsa iv.hex hybrid.enc hybrid.authdata hybrid.hmac

# Receiver simulation
openssl pkeyutl -decrypt -inkey rsa_priv.pem -in bundle.rsa -out bundle_dec.txt \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
chmod 600 bundle_dec.txt
sed -n '1p' bundle_dec.txt > aes_rx.key
sed -n '2p' bundle_dec.txt > mac_rx.key
chmod 600 aes_rx.key mac_rx.key
cat iv.hex hybrid.enc > hybrid_rx.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" -binary -out hybrid_rx.hmac hybrid_rx.authdata

if cmp -s hybrid.hmac hybrid_rx.hmac; then
    echo "HMAC_OK"
    openssl enc -d -aes-256-ctr -K "$(cat aes_rx.key)" -iv "$(cat iv.hex)" -in hybrid.enc -out hybrid_dec.txt
    if cmp -s data.txt hybrid_dec.txt; then
        echo "cmp data.txt hybrid_dec.txt: MATCH"
    fi
else
    echo "HMAC_FAIL"
    exit 1
fi

# Negative tampering test
python3 "$SCRIPT_DIR/flip_bit.py" hybrid.enc hybrid_tampered.enc
cat iv.hex hybrid_tampered.enc > hybrid_tampered.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" hybrid_tampered.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" -binary -out hybrid_tampered.hmac hybrid_tampered.authdata
if cmp -s hybrid.hmac hybrid_tampered.hmac; then
    echo "ERROR: tampering undetected!"
    exit 1
else
    echo "HMAC_FAIL (tampering detected; decryption aborted)"
fi
