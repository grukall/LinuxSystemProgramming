CC = gcc
TARGET = ssu_backup

SRCS =	ssu_backup.c		\
		ssu_backup_utils.c	\
		cmd_help.c			\
		cmd_backup.c		\
		cmd_remove.c		\
		cmd_recover.c		\

OBJS =	$(SRCS:%.c=%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) -g $(OBJS) -o ssu_backup -lcrypto  

%.o : %.c
	$(CC) -g -c $< -o $@

clean:
	rm -f $(OBJS)
