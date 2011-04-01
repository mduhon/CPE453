CC = gcc

CFLAGS = -Wall -g

LDFLAGS =

TARGET = pipeit

SRC = pipeit.c 

HEADERS =  

EXTRACLEAN = $(TARGET) 

all: $(clean) $(TARGET) 

allclean: clean
	@rm -f $(EXTRACLEAN) 
clean:
	rm -f $(TARGET) 

$(TARGET): pipeit.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

			   
depend:
	@echo Regenerating local dependencies
	@makedepend $(SRC) $(HEADERS)
tags: $(SRC) $(HEADERS)
	cscope -b $(SRC) $(HEADERS)
test:
	./$(TARGET) test1.test
