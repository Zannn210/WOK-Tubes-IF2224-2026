# WOK Tubes IF2224 (2026) — Arion Lexer (Milestone 1)

## Identitas Kelompok
- Anggota 1 : Fauzan Mohamad Abdul Ghani (13524113)
- Anggota 2 : Muhammad Daffa Arrizki Yanma (13524133)
- Anggota 3 : Eduard Daniel Ariajaya (13524129)
- Anggota 4 : Safira Berlianti (13524128)
- Anggota 5 : Reysha Syafitri Mulya Ramadhan (13524137)

## Deskripsi Program

Program ini adalah **lexer/tokenizer** untuk kebutuhan Tugas Besar WOK IF2224 (Milestone 1). Lexer membaca source code dari file input, memproses karakter demi karakter menggunakan prinsip **DFA (Deterministic Finite Automata)**, lalu menghasilkan daftar token (mis. `programsy`, `ident(x)`, operator, delimiter, konstanta, string, komentar, dll.) sesuai spesifikasi.

Fokus milestone ini adalah:

- Kerangka program C++ yang modular (lexer dipisah per kategori token).
- Implementasi logika DFA untuk pengenalan token.
- Run melalui `Makefile`.

## Requirements

- OS: macOS, Linux, dan Windows via WSL.
- GNU Make
- Compiler C++ yang mendukung C++17:
	- `g++` (GCC) atau
	- `clang++`

## Cara Instalasi dan Penggunaan Program

### 1) Build / Kompilasi

```bash
make
```

Executable akan dibuat di:

- `bin/arion_lexer`

### 2) Menjalankan Program (interaktif)

Gunakan target `run`, lalu masukkan path input saat diminta:

```bash
make run
```

Contoh input yang bisa dimasukkan:

- `test/milestone-1/input-6.txt`

Perilaku `make run`:

- Output **tetap tampil di terminal**.
- Output juga disimpan ke file baru berurutan: `output_1.txt`, `output_2.txt`, dst.
- Lokasi output disimpan ke folder: `test/milestone-1/`.

### 3) Membersihkan hasil build

Hapus executable:

```bash
make clean
```

Hapus semua file output di folder `test/`:

```bash
make clean-output
```

Hapus executable dan output:

```bash
make clean-all
```

## Struktur Folder (ringkas)

- `src/` — source code C++ lexer
- `bin/` — hasil kompilasi (`arion_lexer`)
- `test/` — input/output pengujian
	- `test/milestone-1/` — kumpulan input/output untuk milestone 1

## Pembagian Tugas

### Anggota 1:
- Membangun kerangka utama program menggunakan bahasa C/C++ GNU.
- Membuat Makefile
- Melakukan 10 pengujian, mengambil tangkapan layar, dan menyusun file input/output di folder `test/milestone-1/`.

### Anggota 2:
- Merancang diagram transisi DFA untuk seluruh keywords (29-51) dan ident.
- Menerjemahkan diagram ke dalam kode program C++.
- Membuat `README.md`.

### Anggota 3:
- Merancang DFA untuk mengenali intcon, realcon, charcon, dan string (termasuk penanganan tanda petik).
- Mengimplementasikan logika pengabaian karakter untuk blok komentar `{ ... }` atau `(* ... *)` pada C++.
- Merapihkan diagram DFA.
  
### Anggota 4:
- Merancang DFA untuk operator seperti plus (`+`), becomes (`:=`), neq (`<>`), leq (`<=`), dan geq (`>=`).
- Mengimplementasikan DFA untuk operator seperti plus (`+`), becomes (`:=`), neq (`<>`), leq (`<=`), dan geq (`>=`) pada C++.

### Anggota 5:
- Menulis laporan PDF yang berisi teori singkat, penjelasan rancangan DFA, dan pembagian tugas.
- Memastikan pembuatan Release GitHub (tag `v0.1.Y`) dilakukan tepat waktu sebelum deadline
