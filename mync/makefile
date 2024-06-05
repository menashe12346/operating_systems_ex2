CC = gcc
CFLAGS = -Wall -g  -fprofile-arcs -ftest-coverage

TARGET1 = ttt
SRC1 = ttt.c

TARGET2 = mync2
SRC2 = mync2.c

TARGET3 = mync3And4
SRC3 = mync3And4.c

TARGET4 = mync3_5
SRC4 = mync3_5.c

TARGET5 = mync6
SRC5 = mync6.c

all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SRC1)

$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SRC2)

$(TARGET3): $(SRC3)
	$(CC) $(CFLAGS) -o $(TARGET3) $(SRC3)

$(TARGET4): $(SRC4)
	$(CC) $(CFLAGS) -o $(TARGET4) $(SRC4)

$(TARGET5): $(SRC5)
	$(CC) $(CFLAGS) -o $(TARGET5) $(SRC5)

clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) mync.socket *.gcno *.gcda

.PHONY: all clean
