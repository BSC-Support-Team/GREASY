docdir = $(prefix)/doc

if COMPILEDOC

GREASY_filename=greasy_userguide
EXTRA_DIST = doc

all: doc

doc: html pdf

html:
	multimarkdown ${GREASY_filename}.mmd > ${GREASY_filename}.html

pdf:
	mmd2pdf ${GREASY_filename}.mmd &> /dev/null
	@make clean

clean:
	@rm -rf ${GREASY_filename}.acn ${GREASY_filename}.acr \
	       ${GREASY_filename}.alg ${GREASY_filename}.dvi \
	       ${GREASY_filename}.glg ${GREASY_filename}.glo \
	       ${GREASY_filename}.gls ${GREASY_filename}.ist \
	       ${GREASY_filename}.aux ${GREASY_filename}.log \
	       ${GREASY_filename}.brf ${GREASY_filename}.log \
	       ${GREASY_filename}.fls ${GREASY_filename}.fdb_latexmk \
	       ${GREASY_filename}.tex 2> /dev/null

distclean: clean
	@rm -rf ${GREASY_filename}.pdf ${GREASY_filename}.html 

endif
