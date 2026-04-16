CC = gcc
CFLAGS = -Iinclude -Wall

SRC = src/main.c src/isp.c src/demosaic.c src/awb.c src/v4l2_capture.c

all:
	$(CC) $(SRC) -o isp $(CFLAGS)

clean:
	rm -f isp