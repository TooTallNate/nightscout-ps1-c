NAME?=nightscout-ps1
PREFIX?=/usr/local
OUTDIR?=out
BIN?=$(OUTDIR)/Default/$(NAME)
VERSION?=$(shell git describe --tags --always)
PLATFORM?=$(shell ./detect-platform.sh)
DISTDIR?=packed
DISTBIN?=$(DISTDIR)/$(NAME)-v$(VERSION)-$(PLATFORM)-x64

build: $(BIN)

dist: $(DISTBIN)

$(BIN): $(OUTDIR) src/*
	@$(MAKE) -C "$(OUTDIR)"
	@strip "$(BIN)"

$(DISTBIN): $(BIN)
	@mkdir -p "$(DISTDIR)"
	@cp -v "$(BIN)" "$(DISTBIN)"

$(OUTDIR): build.gyp
	@./deps/gyp/gyp \
		--no-parallel \
		-f make \
		build.gyp \
		--depth=. \
		--generator-output=$(shell pwd)/$(OUTDIR) \
		-Goutput_dir=$(shell pwd)/$(OUTDIR) \
		-Dcomponent=static_library \
		-Dlibrary=static_library \
		-Duv_library=static_library

install: build
	@cp -v "$(BIN)" "$(PREFIX)/bin/$(NAME)"

clean:
	@rm -vrf $(OUTDIR)

test:

.PHONY: clean build dist install test
