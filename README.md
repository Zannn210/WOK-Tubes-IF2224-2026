# WOK Tubes IF2224 (2026) - Intermediate Code Generation and Interpreter (Milestone 4)

## Identitas Kelompok

- Anggota 1 : Reysha Syafitri Mulya Ramadhan (13524137)
- Anggota 2 : Safira Berlianti (13524128)
- Anggota 3 : Eduard Daniel Ariajaya (13524129)
- Anggota 4 : Muhammad Daffa Arrizki Yanma (13524133)
- Anggota 5 : Fauzan Mohamad Abdul Ghani (13524113)

## Deskripsi Program

Program ini adalah **Intermediate Code Generation and Interpreter** (Milestone 4). Komponen ini melengkapi *pipeline* kompilator Arion menjadi empat tahap utuh:

1. **Lexical Analysis** — `ArionLexer` membaca file sumber dan menghasilkan daftar token.
2. **Syntax Analysis** — `Parser` membangun *parse tree*/AST dari daftar token.
3. **Semantic Analysis** — `SemanticAnalyzer` mendekorasi AST dengan informasi tipe, *scope*, dan *symbol table*.
4. **IC Generation & Interpretation** — `IntermediateCodeGenerator` mengubah *Decorated AST* menjadi instruksi *stack machine*, lalu `StackInterpreter` menjalankan instruksi IC tersebut dan menghasilkan output program.

`IntermediateCodeGenerator` melakukan *traversal* pada *Decorated AST* secara *recursive descent* untuk menghasilkan rangkaian instruksi IC. Kelas ini bertanggung jawab atas *memory layouting*, *code emission*, serta *backpatching* (mengisi target *jump* yang belum diketahui saat instruksi dibuat).

`StackInterpreter` mengeksekusi rangkaian instruksi IC menggunakan model *stack machine*. Kelas ini mengelola *global memory*, *activation frames*, dan *operand stack* melalui siklus *fetch-decode-execute*.

Jika salah satu fase gagal (misalnya *syntax error* atau *semantic error*), fase-fase selanjutnya di-*skip* dan pesan *error* ditulis ke file output.

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

- `test/milestone-4/input-1.txt`

Perilaku `make run`:

- Output **tetap tampil di terminal**.
- Output juga disimpan ke file baru berurutan: `token_output_N.txt`, `ast_output_N.txt`, `tree_output_N.txt`, `ic_output_N.txt`, `runtime_output_N.txt`, dst.
- Lokasi output disimpan ke folder: `test/milestone-4/`.

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

### Anggota 1 (Reysha Syafitri MR — 13524137):

- Mengimplementasikan `genProcFuncCall` dan `genSubprogram` untuk pemanggilan *procedure*/*function* dengan parameter.
- Membuat 38 kasus uji komprehensif yang mencakup seluruh konstruksi bahasa Arion.
- Menyusun laporan PDF Milestone 4 dan mempersiapkan *release* GitHub.

### Anggota 2 (Safira Berlianti — 13524128):

- Mengimplementasikan seluruh kelas `StackInterpreter` termasuk `RuntimeValue`.
- Mengimplementasikan siklus *fetch-decode-execute* pada `run()`.
- Mengimplementasikan operasi aritmatika (`numericOp`), operasi perbandingan (`compareOp`), serta manajemen *activation frame* pada `execCal`.

### Anggota 3 (Eduard Daniel Ariajaya — 13524129):

- Mengimplementasikan `genExpression`, `genSimpleExpression`, `genTerm`, `genFactor`, dan `genConstant` untuk pembangkitan IC dari ekspresi.
- Merancang *memory layouting* (`prepareLayouts`, `collectGlobalVars`, `buildSubprogramLayout`) serta *variable addressing* (`genAddress`, `genVariableValue`).

### Anggota 4 (Muhammad Daffa Arrizki Yanma — 13524133):

- Mengimplementasikan fungsi `genIf`, `genCase`, `genWhile`, `genRepeat`, `genFor`, dan `genAssignment` pada `IntermediateCodeGenerator.cpp`.
- Mengimplementasikan logika *backpatching* untuk semua konstruksi kontrol (IF, WHILE, FOR, CASE).

### Anggota 5 (Fauzan Mohamad Abdul Ghani — 13524113):

- Merancang struktur data `Instruction` dan enum `OprCode` pada `IntermediateCode.hpp`.
- Mengintegrasikan Fase 4 ke dalam `main.cpp` sehingga *pipeline* berjalan *end-to-end* dari *lexer* hingga *interpreter*.
- Menambahkan penanganan *error* antar-fase (skip file output jika fase sebelumnya gagal).
