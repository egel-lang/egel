
# source files and objects
SOURCES= \
	egel-black-transparent.svg \
	egel-black-white.svg \
	egel-white-black.svg \
	egel-white-transparent.svg

OBJECTS48=$(SOURCES:.svg=-48x48.png)
OBJECTS256=$(SOURCES:.svg=-256x256.png)

# targets
all: $(OBJECTS48) $(OBJECTS256)

$(OBJECTS48): %-48x48.png: %.svg
	@echo "creating  " $@ " from " $^
	@inkscape --export-filename=$@ -w 48 -h 48 $^ 

#	@mogrify -quiet -strip $@

$(OBJECTS256): %-256x256.png: %.svg
	@echo "creating  " $@ " from " $^
	@inkscape --export-filename=$@ -w 256 -h 256 $^ 

#	@mogrify -quiet -strip $@

clean:
	@rm -rf $(OBJECTS48)
	@rm -rf $(OBJECTS256)


