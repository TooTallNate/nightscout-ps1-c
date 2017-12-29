OUTDIR=out

$(OUTDIR)/Release/nightscout-ps1: $(OUTDIR)
	$(MAKE) -C $(OUTDIR)

$(OUTDIR): build.gyp
	./deps/gyp/gyp --no-parallel -f make build.gyp --depth=. --generator-output=$(OUTDIR) -Goutput_dir=$(OUTDIR) -Dcomponent=static_library -Dlibrary=static_library -Duv_library=static_library

clean:
	rm -rf $(OUTDIR)

.PHONY: clean
