#
# Makefile for lcal v1.2
#
# v1.2: "make uuencode" creates compressed/uuencoded tar file;
#	"make compress" creates compressed tar file;
#	"make clean" leaves lcal intact but removes other by-products;
#       "make clobber" blows everything away;
#	"make fresh" rebuilds from scratch
#

VERSION  = 1.2

# Set the configuration variables below to taste.

CC	=gcc -march=i686
CFLAGS	=-Wall -Werror -pedantic -O6

# Set COMPRESS to your preferred compression utility (compress, gzip, etc.),
# Z to the file suffix that it generates (.Z, .gz, etc.), and UUC to the file
# suffix to denote the uuencoded version.

COMPRESS = gzip
Z	 = .gz
UUC	 = .uuc

# TAR, et. al. are for creating the 'tar' file to be subsequently compressed.
TAR	= tar
TMP     = /tmp
LCALTMP = lcal.$(VERSION)
TARFILE = lcal.$(VERSION).$(TAR)
ZFILE	= $(TARFILE)$(Z)
UUCFILE = $(TARFILE)$(Z)$(UUC)

TARSRC = lcal.c pcalinit.c lcallang.h lcaldefs.h protos.h Makefile \
	 Makefile.os2 ReadMe lcal_l.ps lcal_p.ps lcal.man

# PACK is for packing the man page.  Note that setting PACK to ":" will cause
# no packing to be done; otherwise, choose "compress" or "pack" as your system
# requires.
PACK	= :
# PACK	= compress
# PACK	= pack

# directories for installing executable and man page(s)
PREFIX	=/usr/local
BINDIR	= ${PREFIX}/bin
MANDIR	= ${PREFIX}/man/man1
CATDIR	= ${PREFIX}/man/cat1

OBJECTS = lcal.o

# Site-specific defaults which may be overridden here (cf. lcallang.h);
# uncomment the examples below and/or change them to your liking

# redefine title and date fonts
# D_TITLEFONT = '-DTITLEFONT="Helvetica-Bold"'
# D_DATEFONT  = '-DDATEFONT="Helvetica-Bold"'

# specify default time zone for moon phases: "0" = GMT; "5" = Boston (-z)
# D_TIMEZONE   = '-DTIMEZONE="5 [Boston]"'

# specify local default X/Y offsets
# D_XOFFSET = '-DX_OFFSET="-20/20"'
# D_YOFFSET = '-DY_OFFSET="-20/20"'

# generate EPS-like pseudo-comments in PostScript output
# D_EPS = '-DEPS'

# is target system Un_x or clone thereof?
# D_UN_X = '-DUN_X'

COPTS = $(D_TITLEFONT) $(D_DATEFONT) $(D_TIMEZONE) $(D_XOFFSET) $(D_YOFFSET) \
	$(D_EPS) $(D_UN_X)

all:	lcal

lcal:	$(OBJECTS)
	$(CC) -o lcal $(OBJECTS) -lm

lcal.o:	lcal.c lcaldefs.h lcallang.h protos.h lcal_p.h lcal_l.h
	$(CC) $(CFLAGS) $(COPTS) -c lcal.c

pcalinit: pcalinit.c
	$(CC) $(CFLAGS) -o pcalinit pcalinit.c

lcal_p.h: pcalinit lcal_p.ps
	./pcalinit lcal_p.ps lcal_p.h header_p

lcal_l.h: pcalinit lcal_l.ps
	./pcalinit lcal_l.ps lcal_l.h header_l

clean:
	${RM} $(OBJECTS) pcalinit lcal_p.h lcal_l.h lcal.cat \
		$(TARFILE) $(ZFILE) $(UUCFILE)

distclean clobber: clean
	${RM} lcal

fresh:	clobber lcal

lcal.cat:	lcal.man
	nroff -man lcal.man > lcal.cat

install:	lcal lcal.man
	install -d ${BINDIR}
	install -c -s lcal ${BINDIR}
	install -d ${MANDIR}
	install -c -m 0444 lcal.man ${MANDIR}/lcal.1

compress:
	rm -rf $(TMP)/$(LCALTMP) $(TMP)/$(TARFILE) $(TARFILE) $(ZFILE)
	mkdir $(TMP)/$(LCALTMP)
	cp $(TARSRC) $(TMP)/$(LCALTMP)
	cd $(TMP); $(TAR) cf $(TARFILE) $(LCALTMP)
	mv $(TMP)/$(TARFILE) .
	$(COMPRESS) $(TARFILE)
	rm -rf $(TMP)/$(LCALTMP)
	@ echo "Created $(ZFILE)"

uuencode: compress
	@ echo "# lcal v$(VERSION) - uuencoded/compressed Unix tar file" > $(UUCFILE)
	@ echo "#" >> $(UUCFILE)
	@ echo "# Last updated: `date`" >> $(UUCFILE)
	@ echo "#" >> $(UUCFILE)
	@ echo "# to extract:" >> $(UUCFILE)
	@ echo "#" >> $(UUCFILE)
	@ echo "#   uudecode $(UUCFILE)" >> $(UUCFILE)
	@ echo "#   uncompress $(ZFILE)" >> $(UUCFILE)
	@ echo "#   tar xvf $(TARFILE)" >> $(UUCFILE)
	@ echo "#" >> $(UUCFILE)
	uuencode $(ZFILE) < $(ZFILE) >> $(UUCFILE)
	@ echo "Created $(UUCFILE)"
