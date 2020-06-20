PDFLATEX?=xelatex
OO=out
all: ICRA2019-multirobotVoxelExp.pdf

ICRA2019-multirobotVoxelExp.pdf: $(OO)/main.pdf $(OO)/type1font-check-passed.flag
	cp $< $@

$(OO)/type1font-check-passed.flag: $(OO)/main.pdf
	@foundNoneType1Fonts=$$(pdffonts $< | tail -n +3 | sed -e 's/.*Type \([0-9]\).*/\1/g' | grep -v 1 | wc -l)
	@[[ $${foundNonType1Fonts} -eq 0 ]] || { echo "Your pdf has non Type 1 fonts" && false; }
	@pdffonts "$<" > "$@"


$(OO)/main.pdf: $(if $(wildcard $(OO)/main.aux),sources.bib,)

sources.bib: $(OO)/main.aux $(wildcard $(HOME)/wrk/group-bib/shared.bib)
	nohup aux2bib $< 2>/dev/null | tail -n +5 > $@

media/%:
	$(MAKE) -C media/ $*

include pdflatex.mk
