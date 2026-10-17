# AES-128 Encryption Implementation in C

A from-scratch implementation of the **AES-128 (Advanced Encryption Standard)** block cipher in C, organized into three progressive modules — a basic pedagogical implementation, a modes-of-operation extension, and a hardware-accelerated variant using Intel AES-NI intrinsics.

---

## Repository Structure

```
crs2323_suparna/
├── aes128basic/          # Standalone AES-128 encrypt/decrypt (no modes)
│   ├── aes128.h          # Function prototypes
│   ├── aes128.c          # Core AES-128 implementation
│   └── main.c            # Test driver (NIST test vector)
│
├── aesmodes/             # AES-128 + Modes of Operation (file I/O)
│   ├── aes128.h          # AES function prototypes
│   ├── aes128.c          # Core AES-128 implementation
│   ├── modes.h           # Mode function prototypes
│   ├── modes.c           # ECB, CBC, CFB, OFB, CTR mode implementations
│   ├── main.c            # Test driver (file encryption/decryption)
│   └── guide.txt         # Compilation instructions
│
├── aesintrinsic/         # AES-128 using Intel AES-NI hardware intrinsics
│   ├── aesni128.c        # Hardware-accelerated AES-128 implementation
│   └── guide.txt         # Compilation instructions
│
└── AES_128_Implementation.pdf   # Reference documentation
```

---

## Modules

### 1. `aes128basic` — Core AES-128

A clean, well-commented implementation of the AES-128 algorithm built entirely from first principles. All four round transformations and the key schedule are implemented manually.

**Key components:**

| Function | Description |
|---|---|
| `key_expansion()` | Expands a 16-byte key into 176 bytes (11 round keys) using SubWord, RotWord, and Rcon |
| `aes_encrypt()` | Encrypts a 16-byte plaintext block |
| `aes_decrypt()` | Decrypts a 16-byte ciphertext block |
| `subbytes()` / `invsubbytes()` | S-box and inverse S-box byte substitution |
| `shift_row()` / `invshift_row()` | Row shift and inverse row shift |
| `mixcolumn()` / `invmixcolumn()` | Column mixing via Galois Field multiplication |
| `Add_roundkey()` | XOR state with the current round key |
| `galois_mult()` | Multiplication in GF(2⁸) with the AES irreducible polynomial (0x1b) |

**Test vector (from NIST FIPS-197):**

```
Key:       2b 7e 15 16 28 ae d2 a6 ab f7 15 88 09 cf 4f 3c
Plaintext: 32 43 f6 a8 88 5a 30 8d 31 31 98 a2 e0 37 07 34
Expected:  39 25 84 1d 02 dc 09 fb dc 11 85 97 19 6a 0b 32
```

---

### 2. `aesmodes` — Modes of Operation

Extends the core AES-128 implementation with all major **block cipher modes of operation**, operating directly on binary files. Supports both encryption and decryption for each mode.

**Implemented modes:**

| Mode | Function(s) | Notes |
|---|---|---|
| **ECB** (Electronic Codebook) | `ecb_encrypt()`, `ecb_decrypt()` | PKCS#7 padding |
| **CBC** (Cipher Block Chaining) | `cbc_encrypt()`, `cbc_decrypt()` | IV prepended to output; PKCS#7 padding |
| **CFB** (Cipher Feedback) | `cfb_encrypt()`, `cfb_decrypt()` | IV prepended to output; stream-cipher-like |
| **OFB** (Output Feedback) | `ofb()` | Symmetric encrypt/decrypt; same function used for both |
| **CTR** (Counter) | `ctr()` | 32-bit little-endian counter in the IV's LSBs |

**IV:** A hardcoded 128-bit initialization vector `{0x01, 0x02, ...}` is used. For production use, replace with a cryptographically random IV.

**File I/O:** Operates on binary files. Uncomment the desired mode call in `main.c` and point `ifile`/`ofile` to your input/output files.

---

### 3. `aesintrinsic` — AES-NI Hardware Acceleration

A high-performance AES-128 implementation using **Intel AES-NI** CPU instructions via the `<wmmintrin.h>` intrinsics header. Produces identical results to the software implementation but with significantly lower latency.

**Key components:**

| Function | Description |
|---|---|
| `AES_128_Key_Expansion()` | Key schedule using `_mm_aeskeygenassist_si128` |
| `AES_128_ASSIST()` | Helper for the key expansion assist step |
| `aes_encrypt()` | 10-round encryption using `_mm_aesenc_si128` / `_mm_aesenclast_si128` |
| `aes_decrypt()` | 10-round decryption using `_mm_aesdec_si128` / `_mm_aesdeclast_si128` and `_mm_aesimc_si128` |

> **CPU requirement:** Requires a processor with AES-NI support (Intel Westmere / AMD Bulldozer or later).

---

## Building & Running

### Prerequisites

- GCC (MinGW on Windows or GCC on Linux/macOS)
- AES-NI capable CPU (for the `aesintrinsic` module only)

---

### `aes128basic`

```bash
cd aes128basic

# Compile
gcc -o aesprogram.exe aes128.c main.c

# Run
./aesprogram.exe
```

**Expected output:**
```
Expanded round key:
2b7e1516 ...

Plaintext : 3243f6a8885a308d313198a2e0370734
ciphertext : 3925841d02dc09fbdc118597196a0b32
Decrypted text : 3243f6a8885a308d313198a2e0370734
```

---

### `aesmodes`

```bash
cd aesmodes

# Step 1: Compile object files
gcc -c aes128.c          # produces aes128.o
gcc -c modes.c           # produces modes.o

# Step 2: Link and compile
gcc -o a.exe main.c aes128.o modes.o

# Run (after uncommenting the desired mode in main.c)
./a.exe
```

To select a mode, edit `main.c` and uncomment exactly one of:
```c
ecb_encrypt(ifile, ofile, ExpandedKey);
// ecb_decrypt(ifile, ofile, ExpandedKey);
// cbc_encrypt(ifile, ofile, ExpandedKey);
// cbc_decrypt(ifile, ofile, ExpandedKey);
// cfb_encrypt(ifile, ofile, ExpandedKey);
// cfb_decrypt(ifile, ofile, ExpandedKey);
// ofb(ifile, ofile, ExpandedKey);
// ctr(ifile, ofile, ExpandedKey);
```

---

### `aesintrinsic`

```bash
cd aesintrinsic

# Compile with AES-NI and SSE2 flags
gcc -maes -msse2 -o aesni128.exe aesni128.c

# Run
./aesni128.exe
```

---

## AES-128 Algorithm Overview

AES-128 operates on a 4×4 byte **state matrix** with a 128-bit key over **10 rounds**.

**Encryption round structure:**
1. **AddRoundKey** — XOR state with round key
2. **SubBytes** — Non-linear byte substitution via S-box
3. **ShiftRows** — Cyclically shift rows left (row `i` shifts by `i` positions)
4. **MixColumns** — Linear mix of each column in GF(2⁸) *(skipped in the final round)*

**Decryption** applies the inverse of each operation in reverse order using the inverse S-box, right shifts, and inverse MixColumns coefficients (`0x0e`, `0x0b`, `0x0d`, `0x09`).

**Key Schedule:** The 16-byte key is expanded to 176 bytes providing one 16-byte round key per round (rounds 0–10).

---

## Notes

- All implementations use **column-major** state ordering consistent with the AES specification.
- The `aesmodes` module uses **PKCS#7 padding** for ECB and CBC modes.
- The AES-NI implementation uses `_mm_aesimc_si128` (Inverse MixColumns) during decryption to convert the forward key schedule to the equivalent inverse key schedule on the fly.
- The `ftruncate` call in `ecb_decrypt` / `cbc_decrypt` is POSIX-specific and may require adjustment on Windows (use `_chsize` or `SetEndOfFile`).

---

## References

- [NIST FIPS 197 — Advanced Encryption Standard](https://csrc.nist.gov/publications/detail/fips/197/final)
- [Intel AES-NI White Paper](https://www.intel.com/content/www/us/en/developer/articles/technical/advanced-encryption-standard-instructions-aes-ni.html)


---

## Author

**Suparna**
Course: CrS 2323
Academic implementation for educational purposes.

---

## Disclaimer

This project is a **learning implementation** of AES-128 written for educational purposes. It is **not intended for production use**.

Specific limitations to be aware of:

- The IV in `aesmodes` is hardcoded (`{0x01, 0x02, ...}`). A secure implementation must use a cryptographically random IV generated fresh for every encryption operation.
- No authenticated encryption (no MAC or AEAD). The ciphertext is not integrity-protected and is vulnerable to tampering.
- ECB mode is included for completeness but is **inherently insecure** for most data — it leaks patterns in the plaintext.
- The `ftruncate` call used in `ecb_decrypt` / `cbc_decrypt` is POSIX-specific; on Windows, use `_chsize` or `SetEndOfFile` instead.
- Side-channel resistance (timing, cache) has not been considered.

For real-world encryption, use a well-audited library such as [OpenSSL](https://www.openssl.org/) or [libsodium](https://libsodium.org/).

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
