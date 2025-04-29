@all: ./builddir/latex-depcheck

./builddir/latex-depcheck: ./builddir
	ninja -C ./builddir

./builddir: meson.build
	meson setup builddir --reconfigure

clean:
	rm -rf builddir

install: ./builddir/latex-depcheck
	install -d $(DESTDIR)/usr/bin
	install ./builddir/latex-depcheck $(DESTDIR)/usr/bin/latex-depcheck
