CC = gcc

HEADER = ssu_header
SYNC = ssu_sync
HELP = ssu_help
INIT = ssu_init
STRUCT = ssu_struct

$(SYNC) : $(SYNC).o $(HELP).o $(HEADER).o $(INIT).o $(STRUCT).o
	$(CC) -g -o $@ $^ -lcrypto -fcommon

$(SYNC).o : $(SYNC).c
	$(CC) -g -c -o $@ $^ -lcrypto -fcommon

$(HELP).o : $(HELP).c
	$(CC) -g -c -o $@ $^ -lcrypto -fcommon

$(HEADER).o : $(HEADER).c
	$(CC) -g -c -o $@ $^ -lcrypto -fcommon

$(INIT).o : $(INIT).c
	$(CC) -g -c -o $@ $^ -lcrypto -fcommon

$(STRUCT).o : $(STRUCT).c
	$(CC) -g -c -o $@ $^ -lcrypto -fcommon

clean :
	rm -rf $(SYNC)
	rm -rf *.o