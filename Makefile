CC = g++
CFLAGS = -Wall -std=c++17
TARGET = arion_lexer

SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

# Mengambil semua file source code
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

OUTPUT = $(BIN_DIR)/$(TARGET)

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)
	@echo "Kompilasi berhasil! Gunakan 'make run' untuk melanjutkan."

run: $(OUTPUT)
	@read -p "Masukkan File input (contoh: test/input2.txt): " filepath; \
	if [ -z "$$filepath" ]; then echo "Input kosong."; exit 1; fi; \
	if [ ! -f "$$filepath" ]; then echo "File tidak ditemukan: $$filepath"; exit 1; fi; \
	outdir="$(TEST_DIR)/milestone-1"; \
	mkdir -p "$$outdir"; \
	last_num=$$(ls -1 "$$outdir"/output_[0-9]*.txt 2>/dev/null \
		| sed -E 's|.*/output_([0-9]+)\.txt|\1|' \
		| grep -E '^[0-9]+$$' \
		| sort -n \
		| tail -n 1); \
	if [ -z "$$last_num" ]; then last_num=0; fi; \
	next_num=$$((last_num + 1)); \
	outpath="$$outdir/output_$${next_num}.txt"; \
	echo "Menyimpan output ke $$outpath"; \
	./$(OUTPUT) "$$filepath" | tee "$$outpath"

test: $(OUTPUT)
	@echo "Menjalankan Lexer pada file di folder $(TEST_DIR)..."
	@mkdir -p $(TEST_DIR)
	@for file in $(TEST_DIR)/input*.txt; do \
		if [ -f "$$file" ]; then \
			echo "Menguji $$file"; \
			basename=$$(basename "$$file" .txt); \
			inputnum=$$(echo "$$basename" | sed 's/input//'); \
			if [ -z "$$inputnum" ]; then inputnum="1"; fi; \
			outfile="$(TEST_DIR)/output_$${inputnum}.txt"; \
			echo "Menyimpan hasil ke $$outfile"; \
			./$(OUTPUT) "$$file" > "$$outfile"; \
			echo "Output disimpan di $$outfile"; \
			echo "-----------------------------"; \
		fi; \
	done

test-all: $(OUTPUT)
	@echo "Menjalankan Lexer pada semua file .txt di folder $(TEST_DIR)..."
	@mkdir -p $(TEST_DIR)
	@last_num=$$(ls -1 $(TEST_DIR)/output_[0-9]*.txt 2>/dev/null \
		| sed -E 's|.*/output_([0-9]+)\.txt|\1|' \
		| grep -E '^[0-9]+$$' \
		| sort -n \
		| tail -n 1); \
	if [ -z "$$last_num" ]; then last_num=0; fi; \
	next_num=$$((last_num + 1)); \
	for file in $(TEST_DIR)/*.txt; do \
		if [ -f "$$file" ] && [[ "$$file" != *output_* ]]; then \
			echo "Menguji $$file"; \
			outfile="$(TEST_DIR)/output_$${next_num}.txt"; \
			echo "Menyimpan hasil ke $$outfile"; \
			./$(OUTPUT) "$$file" > "$$outfile"; \
			echo "Output disimpan di $$outfile"; \
			echo "-----------------------------"; \
			next_num=$$((next_num + 1)); \
		fi; \
	done

clean:
	rm -f $(OUTPUT)
	@echo "File executable berhasil dihapus."

clean-output:
	rm -f $(TEST_DIR)/output_*.txt
	@echo "Semua file output berhasil dihapus."

clean-all: clean clean-output
	@echo "Semua file executable dan output berhasil dihapus."

help:
	@echo "=== Arion Lexer Makefile ==="
	@echo "Target yang tersedia:"
	@echo "  make              - Kompilasi program"
	@echo "  make run          - Jalankan lexer dengan input manual (tampilkan + simpan ke output_N.txt)"
	@echo "  make test         - Test semua file input*.txt (output ke output_N.txt)"
	@echo "  make test-all     - Test semua file .txt (output auto-increment)"
	@echo "  make clean        - Hapus executable"
	@echo "  make clean-output - Hapus semua file output_*.txt"
	@echo "  make clean-all    - Hapus executable dan output"
	@echo "  make help         - Tampilkan pesan ini"

.PHONY: all clean clean-output clean-all test test-all run help
