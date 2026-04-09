TARGET = dsh
LIBS = -lreadline

CFLAGS = -I/usr/local/include
LDFLAGS = -L/usr/local/lib

$(TARGET): dsh.c
	$(CC) $(CFLAGS) dsh.c $(LDFLAGS) $(LIBS) -o $(TARGET)
