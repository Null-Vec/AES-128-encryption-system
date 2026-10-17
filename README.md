<div align="center">

# 🔐 AES-128 in C 

**End-to-end implementation of AES-128 encryption — no external crypto libraries.**

[![Language](https://img.shields.io/badge/Language-C-blue?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard](https://img.shields.io/badge/Standard-NIST%20FIPS%20197-green?style=flat-square)](https://csrc.nist.gov/publications/detail/fips/197/final)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square)]()

</div>

---

## 📖 What Is This?

AES (Advanced Encryption Standard) is the world's most widely used symmetric encryption algorithm — powering HTTPS, disk encryption, VPNs, and more. Most developers treat it as a black box through libraries like OpenSSL. This project opens that box completely.

Every single component is built by hand in plain C:

- The **S-box** substitution tables
- **Galois Field (GF 2⁸)** arithmetic
- The **key schedule** that derives 11 round keys from a single 16-byte key
- All four round transformations: `SubBytes`, `ShiftRows`, `MixColumns`, `AddRoundKey`
- All five standard **modes of operation** (ECB, CBC, CFB, OFB, CTR)
- **Intel AES-NI** hardware acceleration via CPU intrinsics

Each function is commented to explain *why* it exists, not just *what* it does.

---

## 📁 Project Structure

```
📦 AES-128-in-C/
│
├── 📂 aes128basic/            ← Module 1: Start here — pure AES-128
│   ├── aes128.h               #   Function prototypes
│   ├── aes128.c               #   Complete AES-128 encrypt + decrypt engine
│   └── main.c                 #   NIST FIPS-197 test vector driver
│
├── 📂 aesmodes/               ← Module 2: AES + 5 modes, works on real files
│   ├── aes128.h / aes128.c    #   Same core engine
│   ├── modes.h / modes.c      #   ECB, CBC, CFB, OFB, CTR
│   └── main.c                 #   File encrypt/decrypt driver
│
├── 📂 aesintrinsic/           ← Module 3: Hardware-accelerated via Intel AES-NI
│   └── aesni128.c             #   Uses _mm_aesenc_si128 and friends
│
└── 📄 AES_128_Implementation.pdf  ← Full reference documentation
```

> **Recommended learning path:** Module 1 → Module 2 → Module 3

---

## 🧩 Module 1 — Core AES-128 (`aes128basic`)

### How AES-128 Works

AES treats every 16 bytes of data as a **4×4 grid of bytes** called the *state*. It then scrambles that grid through **10 rounds**, each applying four transformations in sequence:

```
┌──────────────────────────────────────────────────────────────────┐
│                        ONE AES ROUND                             │
│                                                                  │
│  ① SubBytes    — Swap each byte through the 256-entry S-box.    │
│                  This adds non-linearity (breaks patterns).      │
│                                                                  │
│  ② ShiftRows   — Cyclically shift each row left by its index.   │
│                  Row 0 stays, Row 1 shifts 1, Row 2 shifts 2...  │
│                                                                  │
│  ③ MixColumns  — Matrix-multiply each column in GF(2⁸).        │
│                  Diffuses changes across bytes. Skipped in       │
│                  the final (10th) round.                         │
│                                                                  │
│  ④ AddRoundKey — XOR the state with this round's 16-byte key.   │
│                  The only step that actually uses the key.       │
└──────────────────────────────────────────────────────────────────┘
```

Decryption is the exact reverse: each step has an inverse (`InvSubBytes`, `InvShiftRows`, `InvMixColumns`), applied in opposite order.

### The Key Schedule

Your 16-byte secret key never gets used directly in each round. Instead, it gets **expanded** into 176 bytes — 11 separate 16-byte round keys. This expansion involves:

1. **RotWord** — rotate 4 bytes left by one position
2. **SubWord** — pass all 4 bytes through the S-box
3. XOR with a **round constant (Rcon)** to prevent symmetry between rounds

```
Original Key (16 bytes)
       │
       ▼
  Key Schedule
  ┌─────────┐
  │ Round 0 │  ← used in initial AddRoundKey (before round 1)
  │ Round 1 │  ← used in round 1
  │ Round 2 │  ← used in round 2
  │  ...    │
  │ Round 10│  ← used in final round
  └─────────┘
  (176 bytes total)
```

### Galois Field Arithmetic

`MixColumns` multiplies bytes using **GF(2⁸)** arithmetic — that is, polynomial multiplication modulo the irreducible polynomial `x⁸ + x⁴ + x³ + x + 1` (= `0x11b`). The `galois_mult()` function implements this as a simple bit-shift loop, which is both efficient and readable.

### Function Reference

| Function | Description |
|---|---|
| `key_expansion(key, expandedkey)` | Expand 16-byte key → 176-byte schedule |
| `aes_encrypt(expandedkey, plaintext, ciphertext)` | Encrypt one 16-byte block |
| `aes_decrypt(expandedkey, ciphertext, plaintext)` | Decrypt one 16-byte block |
| `subbytes(state)` | Apply S-box to all 16 bytes |
| `invsubbytes(state)` | Apply inverse S-box to all 16 bytes |
| `shift_row(state)` | Cyclically shift rows left |
| `invshift_row(state)` | Cyclically shift rows right |
| `mixcolumn(state)` | GF(2⁸) column mixing |
| `invmixcolumn(state)` | Inverse GF(2⁸) column mixing |
| `Add_roundkey(state, roundkey)` | XOR state with round key |
| `galois_mult(a, b)` | Multiply two bytes in GF(2⁸) |
| `subword(word)` | S-box substitution for key schedule |
| `rotate_word(word)` | Left-rotate 4-byte word for key schedule |

---

## 🔄 Module 2 — Modes of Operation (`aesmodes`)

AES itself only encrypts **exactly 16 bytes** at a time. To encrypt a file of any length, you need a **mode of operation** — a rule for how to chain AES calls together.

### The 5 Modes

#### ① ECB — Electronic Codebook
Each 16-byte block is encrypted completely independently.

```
Plaintext Block 1 ──► [AES] ──► Ciphertext Block 1
Plaintext Block 2 ──► [AES] ──► Ciphertext Block 2
Plaintext Block 3 ──► [AES] ──► Ciphertext Block 3
```

> ⚠️ **Do not use for real data.** Equal plaintext blocks produce equal ciphertext blocks — an attacker can see patterns without decrypting anything. Included here for educational comparison only.

---

#### ② CBC — Cipher Block Chaining ✅ *(most common)*
Each block is XORed with the previous ciphertext block before encryption. An **Initialization Vector (IV)** handles the first block.

```
     IV
      │
      ▼
PT₁ ─XOR─► [AES] ──► CT₁
                        │
                        ▼
PT₂ ─────► [AES] ──► CT₂   (CT₁ is XORed into PT₂ first)
```

- Hides repeated plaintext patterns ✅
- Uses **PKCS#7 padding** to handle inputs that aren't a multiple of 16 bytes
- IV is prepended to the output file automatically

---

#### ③ CFB — Cipher Feedback
Turns AES into a **stream cipher** — no padding needed, can encrypt any length.

```
IV ──► [AES] ──► XOR ──► CT₁
                  ↑
                 PT₁        (CT₁ feeds back as the next IV)
```

---

#### ④ OFB — Output Feedback
The **AES output** (keystream) — not the ciphertext — feeds back into the next block. Encrypt and decrypt use the *exact same function*.

```
IV ──► [AES] ──► KS₁ ──► XOR ──► CT₁
                  │         ↑
             (feed back)   PT₁
```

---

#### ⑤ CTR — Counter Mode
Encrypts a monotonically increasing counter to generate a keystream. Fully **parallelizable** — great for performance.

```
IV||0 ──► [AES] ──► XOR ──► CT₁
              ↑
             PT₁

IV||1 ──► [AES] ──► XOR ──► CT₂
              ↑
             PT₂
```

### Modes Function Reference

| Function | Mode | Direction |
|---|---|---|
| `ecb_encrypt(ifile, ofile, key)` | ECB | Encrypt |
| `ecb_decrypt(ifile, ofile, key)` | ECB | Decrypt |
| `cbc_encrypt(ifile, ofile, key)` | CBC | Encrypt |
| `cbc_decrypt(ifile, ofile, key)` | CBC | Decrypt |
| `cfb_encrypt(ifile, ofile, key)` | CFB | Encrypt |
| `cfb_decrypt(ifile, ofile, key)` | CFB | Decrypt |
| `ofb(ifile, ofile, key)` | OFB | Both (same function) |
| `ctr(ifile, ofile, key)` | CTR | Both (same function) |

---

## ⚡ Module 3 — Hardware Acceleration (`aesintrinsic`)

Modern Intel/AMD CPUs have AES built directly into silicon. This module uses those instructions via the `<wmmintrin.h>` intrinsics header. The result is **bit-for-bit identical** to Module 1 — just dramatically faster.

### AES-NI Instructions Used

| Intrinsic | What It Does |
|---|---|
| `_mm_aeskeygenassist_si128` | Hardware key schedule generation |
| `_mm_aesenc_si128` | Full AES encryption round (rounds 1–9) |
| `_mm_aesenclast_si128` | Final encryption round (no MixColumns) |
| `_mm_aesdec_si128` | Full AES decryption round |
| `_mm_aesdeclast_si128` | Final decryption round |
| `_mm_aesimc_si128` | Convert forward key schedule for decryption |

> **CPU requirement:** Intel Westmere (2010+) or AMD Bulldozer (2011+). Nearly all modern desktop/laptop CPUs qualify.

---

## 🚀 Build & Run

### Prerequisites

| Tool | Windows | Linux / macOS |
|---|---|---|
| Compiler | [MinGW-w64 (GCC)](https://www.mingw-w64.org/) | GCC via package manager |
| AES-NI support | Required only for Module 3 | Required only for Module 3 |

Check AES-NI support on your machine:
```bash
# Linux
grep -m1 aes /proc/cpuinfo

# Windows PowerShell
(Get-WmiObject -Class Win32_Processor).Name
```

---

### 📦 Module 1 — `aes128basic`

```bash
cd aes128basic

# Compile
gcc -o aesprogram aes128.c main.c

# Windows
gcc -o aesprogram.exe aes128.c main.c

# Run
./aesprogram
```

**Expected output:**
```
Expanded round key:
2b 7e 15 16 28 ae d2 a6 ab f7 15 88 09 cf 4f 3c
a0 fa fe 17 88 54 2c b1 23 a3 39 39 2a 6c 76 05
...

Plaintext  : 3243f6a8885a308d313198a2e0370734
Ciphertext : 3925841d02dc09fbdc118597196a0b32   ← must match NIST FIPS-197 ✅
Decrypted  : 3243f6a8885a308d313198a2e0370734   ← must match original ✅
```

---

### 📦 Module 2 — `aesmodes`

```bash
cd aesmodes

# Step 1: Compile object files
gcc -c aes128.c          # produces aes128.o
gcc -c modes.c           # produces modes.o

# Step 2: Link
gcc -o aesmodes main.c aes128.o modes.o

# Windows
gcc -o aesmodes.exe main.c aes128.o modes.o
```

**Before running**, open `main.c` and:

1. Point the file handles at your input and output files:
```c
FILE *ifile = fopen("plaintext.txt", "rb");
FILE *ofile = fopen("encrypted.bin", "wb");
```

2. Uncomment exactly **one** mode function (all others must be commented out):
```c
// ecb_encrypt(ifile, ofile, ExpandedKey);
// ecb_decrypt(ifile, ofile, ExpandedKey);
   cbc_encrypt(ifile, ofile, ExpandedKey);  // ← uncomment this one
// cbc_decrypt(ifile, ofile, ExpandedKey);
// cfb_encrypt(ifile, ofile, ExpandedKey);
// cfb_decrypt(ifile, ofile, ExpandedKey);
// ofb(ifile, ofile, ExpandedKey);
// ctr(ifile, ofile, ExpandedKey);
```

3. Recompile and run.

---

### 📦 Module 3 — `aesintrinsic`

```bash
cd aesintrinsic

# Compile (the -maes and -msse2 flags enable AES-NI instructions)
gcc -maes -msse2 -o aesni128 aesni128.c

# Windows
gcc -maes -msse2 -o aesni128.exe aesni128.c

# Run
./aesni128
```

---

## 🧪 Step-by-Step Testing Guide

### ✅ Test 1 — Verify against the NIST Standard (Module 1)

This test confirms your AES core is mathematically correct.

```bash
cd aes128basic
gcc -o aesprogram aes128.c main.c
./aesprogram
```

Look for this line in the output:
```
Ciphertext : 3925841d02dc09fbdc118597196a0b32
```
This is the exact value in **NIST FIPS-197, Appendix B**. If it matches, the engine is correct.

---

### ✅ Test 2 — Encrypt then Decrypt a File (Module 2)

This test proves encrypt → decrypt gives back the original file.

**Step 1 — Create a test file**
```bash
# Linux/macOS
echo "Hello AES World! This is a test message." > plaintext.txt

# Windows PowerShell
"Hello AES World! This is a test message." | Out-File -Encoding ascii plaintext.txt
```

**Step 2 — Encrypt it**

In `aesmodes/main.c`:
```c
FILE *ifile = fopen("plaintext.txt", "rb");
FILE *ofile = fopen("encrypted.bin", "wb");
// uncomment:
cbc_encrypt(ifile, ofile, ExpandedKey);
```
Recompile and run → `encrypted.bin` is created.

**Step 3 — Decrypt it**

In `aesmodes/main.c`:
```c
FILE *ifile = fopen("encrypted.bin", "rb");
FILE *ofile = fopen("decrypted.txt", "wb");
// uncomment:
cbc_decrypt(ifile, ofile, ExpandedKey);
```
Recompile and run → `decrypted.txt` is created.

**Step 4 — Verify they match**
```bash
# Linux/macOS
diff plaintext.txt decrypted.txt
# (no output = files are identical ✅)

# Windows PowerShell
fc plaintext.txt decrypted.txt
```

Repeat this test with **different modes** (CFB, OFB, CTR) to see that they all round-trip correctly.

---

### ✅ Test 3 — Confirm AES-NI Gives the Same Result (Module 3)

The hardware and software implementations should produce **identical output**.

Edit `aesintrinsic/aesni128.c` to use the same key as Module 1:
```c
unsigned char userkey[16] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
```

Compile and run:
```bash
gcc -maes -msse2 -o aesni128 aesni128.c
./aesni128
```

Expected ciphertext:
```
3925841d02dc09fbdc118597196a0b32   ← identical to Module 1 ✅
```

This confirms the hardware path and software path are interchangeable.

---

### 🧩 Bonus — Try All 5 Modes on the Same File

Run this sequence to compare all five modes on the same input. Each should decrypt back to the original:

| Mode | Encrypt function | Decrypt function | Notes |
|---|---|---|---|
| ECB | `ecb_encrypt` | `ecb_decrypt` | Identical blocks → identical output (pattern leak) |
| CBC | `cbc_encrypt` | `cbc_decrypt` | Each block depends on previous |
| CFB | `cfb_encrypt` | `cfb_decrypt` | Stream cipher, no padding |
| OFB | `ofb` | `ofb` (same!) | Same function for both directions |
| CTR | `ctr` | `ctr` (same!) | Same function for both directions |

---

## ⚠️ Known Limitations

| Issue | Detail |
|---|---|
| **`ftruncate` is POSIX-only** | Used in `ecb_decrypt` / `cbc_decrypt` to remove PKCS#7 padding. On Windows with MSVC, replace with `_chsize(fileno(ofile), pos)`. MinGW (GCC on Windows) supports it fine. |
| **Hardcoded IV** | The IV in `aesmodes` is a fixed `{0x01, 0x02, ...}`. A real implementation must generate a **cryptographically random IV** for every encryption operation. |
| **No message authentication** | There is no MAC, HMAC, or AEAD. Ciphertext can be silently tampered with. For real use, consider AES-GCM instead. |
| **No side-channel mitigations** | Table-lookup-based S-box is vulnerable to **cache-timing attacks**. This is an educational implementation, not a hardened one. |
| **ECB mode** | Fundamentally insecure for any structured data — included only to illustrate *why* the other modes exist. |

---

## 📚 Further Reading

| Resource | What You'll Learn |
|---|---|
| [NIST FIPS-197](https://csrc.nist.gov/publications/detail/fips/197/final) | The official AES specification this code implements |
| [Intel AES-NI White Paper](https://www.intel.com/content/www/us/en/developer/articles/technical/advanced-encryption-standard-instructions-aes-ni.html) | How hardware acceleration works in Module 3 |
| [Rijndael S-box (Wikipedia)](https://en.wikipedia.org/wiki/Rijndael_S-box) | How the S-box was mathematically constructed |
| [Galois Field Arithmetic (Wikipedia)](https://en.wikipedia.org/wiki/Finite_field_arithmetic) | The math behind `MixColumns` and `galois_mult()` |
| [Block cipher modes (Wikipedia)](https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation) | Visual diagrams for ECB, CBC, CFB, OFB, CTR |

---

## 👤 Author

**Suparna** — Indian Statistical Institute, Kolkata
 
Academic implementation, written for educational purposes.

---

## 📄 License

Released under the [MIT License](LICENSE). Use freely for learning and reference.

---

<div align="center">

> ⚠️ **Not for production use.** This is a learning implementation. For real-world encryption,  
> use an audited library like [OpenSSL](https://www.openssl.org/) or [libsodium](https://libsodium.org/).

</div>
