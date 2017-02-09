
export VERSION = $(shell cat VERSION)

export VMAJOR=$(shell echo $(VERSION) | cut -d. -f1)
export VMINOR=$(shell echo $(VERSION) | cut -d. -f2)
export VBUILD=$(shell echo $(VERSION) | cut -d. -f3)
export COMPILED = $(shell date -u +"%Y%m%dT%H%M%SZ")
export COPTS = -O1 -g
export CFLAGS += $(COPTS) -D _GNU_SOURCE \
                        -DCOMPILE_DATE=\"$(COMPILED)\" \
                        -DSANDSCRIPT_MAP_EXTENSION_VERSION=\"$(VERSION)\" \
                        -DSANDSCRIPT_MAP_EXTENSION_GITCOMMIT=\"$(shell git rev-parse HEAD)\"

SRCS = sandscript-map-extension.c

all: deps sandscript-map-extension.so

version:
	@install -m 0755 -d $(DESTDIR)/usr/local/sandvine/etc/versions
	@echo svproduct_id=810 > $(DESTDIR)/usr/local/sandvine/etc/versions/ZeroMQ
	@echo svproduct_major=$(VMAJOR) >> $(DESTDIR)/usr/local/sandvine/etc/versions/Map-Extension
	@echo svproduct_minor=$(VMINOR) >> $(DESTDIR)/usr/local/sandvine/etc/versions/Map-Extension
	@echo svproduct_patch=$(VBUILD) >> $(DESTDIR)/usr/local/sandvine/etc/versions/Map-Extension
	@echo svproduct_build= >> $(DESTDIR)/usr/local/sandvine/etc/versions/Map-Extension

install:  version
	./gen-quick-ref
	install -m 0755 -d $(DESTDIR)/usr/share/man/cat5
	install -m 0444 sandscript-map-extension.5 $(DESTDIR)/usr/share/man/cat5
	install -m 0755 -d $(DESTDIR)/usr/local/sandvine/loadable
	install -m 0755 -d $(DESTDIR)/usr/lib64
	install -m 0755 -d $(DESTDIR)/usr/include
	install -m 0555 sandscript-map-extension.so $(DESTDIR)/usr/local/sandvine/loadable

deps: sandscript-library-interface/.git

sandscript-library-interface/.git:
	git clone https://github.com/sandvine/sandscript-library-interface

sandscript-map-extension.so: sandscript-map-extension.c
	@echo
	$(CC) $(CFLAGS) -fPIC -Wall -shared -o $@ $(SRCS) -lpthread

clean:
	@rm -rf sandscript-map-extension.so sandscript-map-extension.5 a.out
	@(cd tests; $(MAKE) clean)

rpm-release:
	rpm/rpmbuild.sh .


