PREFIX?=/usr/local
OUTDIR?=out

$(OUTDIR)/Default/nightscout-ps1: $(OUTDIR)
	$(MAKE) -C $(OUTDIR)

$(OUTDIR): build.gyp
	./deps/gyp/gyp --no-parallel -f make build.gyp --depth=. --generator-output=$(shell pwd)/$(OUTDIR) -Goutput_dir=$(shell pwd)/$(OUTDIR) -Dcomponent=static_library -Dlibrary=static_library -Duv_library=static_library

install: $(OUTDIR)/Default/nightscout-ps1
	cp "$^" "$(PREFIX)/bin/"

clean:
	rm -rf $(OUTDIR)

.PHONY: clean install
