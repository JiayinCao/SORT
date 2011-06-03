SRC_PATH = ../src
OBJ_PATH = obj

sort : $(OBJ_PATH) sort.o vector.o
	g++ -o sort $(OBJ_PATH)/sort.o $(OBJ_PATH)/vector.o

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

sort.o : $(SRC_PATH)/sort.cpp
	g++ -c $(SRC_PATH)/sort.cpp -o $(OBJ_PATH)/sort.o

vector.o : $(SRC_PATH)/geometry/vector.cpp
	g++ -c $(SRC_PATH)/geometry/vector.cpp -o $(OBJ_PATH)/vector.o
