# ===========================
#     CONFIGURACIÓN
# ===========================
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

# Archivos fuente
SRC = etp_ts.cpp

# Ejecutable final
TARGET = solver

# ===========================
#     REGLAS PRINCIPALES
# ===========================

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
