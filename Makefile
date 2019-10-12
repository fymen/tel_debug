TARGET = libtel_debug.a

CFLAGS += -Wall -g
CFLAGS += -O2 -fno-strict-aliasing

LDFLAGS += -r -Bstatic

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

OBJFILES = $(SOURCES:%.c=%.o)
OBJFILES += version.o

VERSION_FILE = version.c

$(VERSION_FILE):
	@echo "UPDATE  $@"
	@touch $(VERSION_FILE)
	@echo \char TEL_DEBUG_BUILDING_TIME "[]=" \"`date '+%F %R'`\" ";" > $(VERSION_FILE)
	@echo \char TEL_DEBUG_BUILDING_PERSON "[]=" \"`whoami`\" ";" >> $(VERSION_FILE)
	@echo \char TEL_DEBUG_BUILDING_HOST "[]=" \"`hostname`\" ";" >> $(VERSION_FILE)


.PHONY: clean all

all:	$(TARGET)



$(TARGET): $(OBJFILES) $(VERSION_FILE)
	@echo linking $@
	$(LD) $(LDFLAGS) -o $@ $(OBJFILES)

clean:
	@echo Removing generated files.
	@rm -rf $(OBJFILES) $(TARGET)
	$(RM) -f $(VERSION_FILE)
