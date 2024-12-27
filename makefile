SRC_DIR = src
INCLUDE_DIR = include
SHARED_SRC = ../shared/src
BIN_DIR = bin

pjl: $(SRC_DIR)/pjl.c $(SRC_DIR)/ui.c $(SHARED_SRC)/jcstring.c $(SHARED_SRC)/jcprint.c $(SHARED_SRC)/jctermio.c $(SHARED_SRC)/jcmemory.c
	c99 -ggdb -o $(BIN_DIR)/pjl \
	$(SRC_DIR)/ui.c $(SRC_DIR)/pjl.c \
	$(SHARED_SRC)/jcstring.c $(SHARED_SRC)/jcprint.c $(SHARED_SRC)/jctermio.c $(SHARED_SRC)/jcmemory.c \
	-I$(INCLUDE_DIR) -I$(SHARED_SRC)

clean:
	rm -f $(BIN_DIR)/pjl

