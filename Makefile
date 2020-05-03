all:
	(cd timerLib; make install)
	(cd lcdLib; make install)
	(cd shapeLib; make install)
	(cd circleLib; make install)
	(cd p2swLib; make install)
	(cd soundLib; make install)
	(cd game; make)

doc:
	rm -rf doxygen_docs
	doxygen Doxyfile
clean:
	(cd timerLib; make clean)
	(cd lcdLib; make clean)
	(cd shapeLib; make clean)
	(cd p2swLib; make clean)
	(cd circleLib; make clean)
	(cd soundLib; make clean)
	(cd game; make clean)
	rm -rf lib h
	rm -rf doxygen_docs/*

load:
	(cd game; make load)

fonttest:
	(cd lcdLib; make clean; make install)
	(cd game; make load)
