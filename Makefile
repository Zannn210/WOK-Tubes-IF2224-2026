CC = g++
CFLAGS = -Wall -std=c++17
TARGET = arion_lexer

SRC_DIR = src
TEST_DIR = test
M4_DIR = $(TEST_DIR)/milestone-4
BIN_DIR = bin

# Mengambil semua file source code dari folder src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OUTPUT = $(BIN_DIR)/$(TARGET)

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)
	@echo "Kompilasi berhasil! Gunakan 'make run' atau 'make test-m4'."

run: $(OUTPUT)
	./$(OUTPUT)

# Jalankan satu file tertentu.
# Contoh: make run-file FILE=test/milestone-4/full.txt
run-file: $(OUTPUT)
	@if [ -z "$(FILE)" ]; then \
		echo "Pakai: make run-file FILE=test/milestone-4/nama_file.txt"; \
		exit 1; \
	fi
	./$(OUTPUT) "$(FILE)"

# Jalankan semua input langsung di test/milestone-4/*.txt.
# Output tiap input masuk ke test/milestone-4/output/<nama_input>/
test: test-m4

test-m4: $(OUTPUT)
	@echo "Menjalankan semua file .txt di folder $(M4_DIR)..."
	@mkdir -p $(M4_DIR)
	@found=0; \
	for file in $(M4_DIR)/*.txt; do \
		if [ -f "$$file" ]; then \
			found=1; \
			echo "Menguji $$file"; \
			./$(OUTPUT) "$$file"; \
			echo "Output disimpan di $(M4_DIR)/output/$$(basename "$$file" .txt)/"; \
			echo "-----------------------------"; \
		fi; \
	done; \
	if [ "$$found" -eq 0 ]; then \
		echo "Tidak ada file input .txt di $(M4_DIR)."; \
	fi

clean:
	rm -f $(OUTPUT)
	@echo "File executable berhasil dihapus."

clean-output:
	rm -rf $(M4_DIR)/output
	@echo "Folder output milestone-4 berhasil dihapus."

clean-all: clean clean-output
	@echo "Semua file executable dan output berhasil dihapus."

help:
	@echo "=== Arion Compiler Makefile ==="
	@echo "Target yang tersedia:"
	@echo "  make                         - Kompilasi program"
	@echo "  make run                     - Jalankan mode interaktif"
	@echo "  make run-file FILE=<path>    - Jalankan satu file input tertentu"
	@echo "  make test / make test-m4     - Test semua .txt di test/milestone-4"
	@echo "  make clean                   - Hapus executable"
	@echo "  make clean-output            - Hapus folder output milestone-4"
	@echo "  make clean-all               - Hapus executable dan output"
	@echo "  make help                    - Tampilkan pesan ini"

.PHONY: all clean clean-output clean-all test test-m4 run run-file help
