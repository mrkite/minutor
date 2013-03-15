# closure compiler
COMPILER=~/compiler.jar
COMPLEVEL=ADVANCED_OPTIMIZATIONS

.DELETE_ON_ERROR:

JS=color.js editor.js zip.js mods.js display.js generator.js

all: editor.min.js

editor.min.js: $(JS)
	java -jar $(COMPILER) --warning_level VERBOSE --compilation_level $(COMPLEVEL) $(addprefix --js=,$^) --externs jquery_externs.js --js_output_file $@

clean:
	rm -f editor.min.js
