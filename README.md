# WOK Tubes IF2224 (2026) - Semantic Analysis (Milestone 3)

## Identitas Kelompok

- Anggota 1 : Reysha Syafitri Mulya Ramadhan (13524137)
- Anggota 2 : Safira Berlianti (13524128)
- Anggota 3 : Eduard Daniel Ariajaya (13524129)
- Anggota 4 : Muhammad Daffa Arrizki Yanma (13524133)
- Anggota 5 : Fauzan Mohamad Abdul Ghani (13524113)

## Deskripsi Program

Program ini adalah **semantic analysis**. Semantic analyzer berfungsi melakukan analisis makna (semantic analysis) dengan menggunakan Attributed Grammar untuk memastikan semantik dari program yang telah lolos analisis sintaksis. Tahapan ini mencakup type checking, symbol table management, scope resolution, dan control flow validation. Parse tree yang dihasilkan oleh parser ditelusuri secara top-down menggunakan fungsi visit pada setiap node-nya, menghasilkan Decorated AST dengan informasi tipe data dan referensi ke symbol table.

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

- `test/milestone-3/input-1.txt`

Perilaku `make run`:

- Output **tetap tampil di terminal**.
- Output juga disimpan ke file baru berurutan:`ast_output_1.txt`, `token_output_1.txt`, `tree_output_2.txt`, dst.
- Lokasi output disimpan ke folder: `test/milestone-3/`.

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

- Menambahkan field anotasi semantik pada ASTNode: semType, tabIdx, lexLevel, dan line untuk menyimpan hasil analisis tipe, referensi symbol table, dan nomor baris.
- Mengimplementasikan ASTDecoratedPrinter untuk mencetak Decorated AST beserta informasi tipe dan isi symbol table ke terminal dan file keluaran.
- Mengimplementasikan visitor untuk kerangka utama program: visitProgram, visitDeclarationPart, visitBlock, visitCompoundStatement, visitStatementList, dan visitStatement sebagai dispatcher ke visitor lainnya.

### Anggota 2:

- Mengimplementasikan 3 Symbol Table utama: tab, btab, atab.
- Mendaftarkan Predefined Identifier melalui initPredefined(): tipe Integer, Real, Char, Boolean, String, serta konstanta true/false dan prosedur writeln/readln.
- Membuat fungsi manajemen scope: enterBlock, leaveBlock, addIdentifier, dan lookupIdent.
- Mengimplementasikan visitor deklarasi: visitConstDeclaration, visitConstant, visitTypeDeclaration, visitType, dan visitVarDeclaration.

### Anggota 3:

- Mengimplementasikan visitor subprogram: visitSubprogramDeclaration, visitProcedureDeclaration, visitFunctionDeclaration, visitFormalParameterList, dan visitParameterGroup.
- Mengimplementasikan visitor akses variabel: visitVariable, visitComponentVariable, dan visitIndexList.
- Mengimplementasikan visitor visitProcFuncCall dan visitAssignmentStatement beserta validasi assignment-compatibility dan bounds checking untuk subrange.

### Anggota 4:

- Mengimplementasikan visitor visitStatementList dan visitStatement sebagai dispatcher ke semua jenis statement.
- Mengimplementasikan visitor percabangan: visitIfStatement, visitCaseStatement, dan visitCaseBlock.
- Mengimplementasikan visitor perulangan: visitWhileStatement, visitRepeatStatement, dan visitForStatement.


### Anggota 5:

- Mengimplementasikan visitor structured type: visitArrayType, visitRecordType, visitFieldList, dan visitFieldPart.
- Menerapkan error handling menyeluruh melalui fungsi semanticError dengan nomor baris, serta validasi tipe pada setiap node ekspresi.
- Menyiapkan minimal 5 test case unik dan memastikan Decorated AST serta Symbol Table tercetak ke terminal dan file keluaran.
