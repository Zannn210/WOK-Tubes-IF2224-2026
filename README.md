# WOK Tubes IF2224 (2026) - Syntax Analysis (Milestone 2)

## Identitas Kelompok

- Anggota 1 : Fauzan Mohamad Abdul Ghani (13524113)
- Anggota 2 : Safira Berlianti (13524128)
- Anggota 3 : Eduard Daniel Ariajaya (13524129)
- Anggota 4 : Muhammad Daffa Arrizki Yanma (13524133)
- Anggota 5 : Reysha Syafitri Mulya Ramadhan (13524137)

## Deskripsi Program

Program ini adalah **syntax analysis atau parser**. Parser berfungsi melakukan analisis sintaksis (syntax analysis) dengan menggunakan Recursive Descent untuk mengenali struktur gramatikal dalam deretan token.

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

- `test/milestone-2/input-1.txt`

Perilaku `make run`:

- Output **tetap tampil di terminal**.
- Output juga disimpan ke file baru berurutan: `token_output_1.txt`, `tree_output_2.txt`, dst.
- Lokasi output disimpan ke folder: `test/milestone-2/`.

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

## Pembagian Tugas

### Anggota 1:

- Mengimplementasikan prioritas ekspresi: expression, simple-expression, term, dan factor.
- Menangani pembacaan operator: relational-operator, additive-operator, dan multiplicative-operator.
- Mengembangkan logika untuk memformat keluaran Parse Tree agar ter-print ke terminal dan tersimpan ke file keluaran .txt.

### Anggota 2:

- Mengimplementasikan aturan produksi untuk const-declaration dan constant.
- Mengimplementasikan pengenalan type-declaration, type, array-type, range, enumerated, dan record-type.
- Mengimplementasikan aturan var-declaration dan identifier-list.

### Anggota 3:

- Mengimplementasikan pembuatan fungsi/prosedur: subprogram-declaration, procedure-declaration, function-declaration, formal-parameter-list, dan parameter-group.
- Mengimplementasikan pemanggilan pemanggilan: procedure/function-call dan parameter-list.
- Mengimplementasikan penugasan nilai: assignment-statement, variable, component-variable, dan index-list.

### Anggota 4:

- Mengimplementasikan pernyataan: statement-list dan statement.
- Mengimplementasikan logika percabangan: if-statement, case-statement, dan case-block.
- Mengimplementasikan logika perulangan: while-statement, repeat-statement, dan for-statement.

### Anggota 5:

- Memperbaiki pembacaan separator (space, newline, tab) pada DFA Lexer sebelumnya.
- Membuat fungsi pembacaan file .txt yang berisi daftar token masukan.
- Mengimplementasikan Recursive Descent untuk node fondasi: program, program-header, dan penggabungan dengan compound-statement.
