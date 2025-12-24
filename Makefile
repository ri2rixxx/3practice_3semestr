CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -Isrc -Isrc/structures

SRC_DIR = src
STRUCT_DIR = $(SRC_DIR)/structures

# Исходники структур данных
STRUCTURES_SRC = $(STRUCT_DIR)/stack.cpp $(STRUCT_DIR)/queue.cpp \
                 $(STRUCT_DIR)/hash_table.cpp $(STRUCT_DIR)/set.cpp

.PHONY: all clean

all: server client dbms url_shortener

# TCP сервер СУБД
server:
	$(CXX) $(CXXFLAGS) -o server $(SRC_DIR)/server.cpp $(SRC_DIR)/database.cpp $(STRUCTURES_SRC)

# TCP клиент СУБД
client:
	$(CXX) $(CXXFLAGS) -o client $(SRC_DIR)/client.cpp

# Консольная версия СУБД
dbms:
	$(CXX) $(CXXFLAGS) -o dbms $(SRC_DIR)/main.cpp $(SRC_DIR)/database.cpp $(STRUCTURES_SRC)

# URL Shortener сервис
url_shortener:
	$(CXX) $(CXXFLAGS) -o url_shortener $(SRC_DIR)/url_shortener.cpp $(SRC_DIR)/db_client.cpp

clean:
	rm -f server client dbms url_shortener *.o

