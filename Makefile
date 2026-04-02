CC = g++
CFLAGS = -Wall -std=c++17
TARGET = arion_lexer

SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin 

#mengambil semua file source code
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

OUTPUT = $(BIN_DIR)/$(TARGET)

all: $(TARGET)

$(TARGET): $(TARGET)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Kompilasi berhasil! Gunakan ./$(TARGET) untuk melanjutkan."

test: $(TARGET)
	@echo "menjalankan Lexer pada file di folder $(TEST_DIR)..."
	@for file in $(wildcard $(TEST_DIR)/*.txt); do \
		echo "Menguji $$file"; \
		./$(TARGET) $$file; \
		echo "-----------------------------"; \
	done

clean:
	rm -f $(TARGET)
	@echo "File berhasil bin dihapus."

.PHONY: all clean test
