CC = gcc

HEADER = ssu_header
REPO = ssu_repo
INIT = ssu_init
STRUCT = ssu_struct
ADD = ssu_add
REMOVE = ssu_remove
HELP = ssu_help

all: $(REPO) $(ADD) $(REMOVE) $(HELP)

$(ADD) : $(ADD).o $(HEADER).o $(INIT).o $(STRUCT).o
	$(CC) -g $^ -o $@ -lcrypto -fcommon

$(REPO) : $(REPO).o $(HEADER).o $(INIT).o $(STRUCT).o
	$(CC) -g $^ -o $@ -lcrypto -fcommon

$(REMOVE) : $(REMOVE).o $(HEADER).o $(INIT).o $(STRUCT).o
	$(CC) -g $^ -o $@ -lcrypto -fcommon

$(HELP) : $(HELP).o
	$(CC) -g $^ -o $@ -lcrypto -fcommon

$(REPO).o : $(REPO).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(HEADER).o : $(HEADER).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(INIT).o : $(INIT).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(STRUCT).o : $(STRUCT).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(ADD).o : $(ADD).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(REMOVE).o : $(REMOVE).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

$(HELP).o : $(HELP).c
	$(CC) -c -g $^ -o $@ -lcrypto -fcommon

clean :
	rm -rf $(REPO)
	rm -rf $(ADD)
	rm -rf $(REMOVE)
	rm -rf $(HELP)
	rm -rf *.o
