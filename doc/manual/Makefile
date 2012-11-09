PROJECT  = MinotaurDevelGuide
TEX_SRCS = ${PROJECT}.tex \
	   install.tex \
	   qg-ex.tex \
	   title.tex \
	   trouble.tex

.PHONY: clean clean_all 

default: html

pdf: dvi
	@dvipdfm -p letter ${PROJECT}.dvi

dvi: ${TEX_SRCS}
	latex ${PROJECT}.tex  ;\
	latex ${PROJECT}.tex  ;\
	latex ${PROJECT}.tex 

html: ${TEX_SRCS}
	@htlatex ${PROJECT} "guide.cfg,index=2"

clean:
	rm -fv *.log *.aux *.bbl *.blg *.toc *.out *.4tc *.4ct *.css *.lg *.idv *.html *.tmp

clean_all: clean
	rm -fv ${PROJECT}.pdf ${PROJECT}.dvi
