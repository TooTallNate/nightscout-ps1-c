PREFIX?=/usr/local
OUTDIR?=out

build: $(OUTDIR)
	@$(MAKE) -C $(OUTDIR)

$(OUTDIR): build.gyp
	@./deps/gyp/gyp --no-parallel -f make build.gyp --depth=. --generator-output=$(shell pwd)/$(OUTDIR) -Goutput_dir=$(shell pwd)/$(OUTDIR) -Dcomponent=static_library -Dlibrary=static_library -Duv_library=static_library

install: build
	@cp "$^" "$(PREFIX)/bin/"

clean:
	@rm -vrf $(OUTDIR)

.PHONY: clean build install
