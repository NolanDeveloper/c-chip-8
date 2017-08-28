CFLAGS += -std=c11 -pedantic -Wall -Wextra -MMD

.PHONY: all
all: emu dis

.PHONY: debug
debug: emu-deb dis-deb

.PHONY: test
test: CFLAGS += -g
test: tests
	./$<

.PHONY: clean
clean:
	$(RM) *.o *.d \
	emu \
	dis \
	emu-deb \
	dis-deb \
	tests

LINK = $(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

emu emu-deb: CFLAGS += `sdl-config --cflags`
emu emu-deb: LDLIBS += `sdl-config --libs`
emu emu-deb: chip8emu.o chip8.o
	$(LINK)

dis dis-deb: chip8dis.o chip8.o
	$(LINK)

tests: chip8-tests.o
	$(LINK)

-include *.d
