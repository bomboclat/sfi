# SFI (Simple File Importer)

Sfi is a simple, fast and minimal template engine inspired from [Smu](https://github.com/Gottox/smu) 
and written with very few lines of C code.
The original goal of this tool is to generate static web pages from little templates.
Sfi is able to import files recursively and print them as a unique output to the stdout.

To import a file in a template with sfi you must add `{{` before and `}}` after the 
name of the file to import.

	{{/path/to/file_to_import}}

You can also import data from stdin with:

	{{...}}

You can mix both and dynamically change the file name to import,
escaping the `{{` of the outer import:

	\{{/path/to/{{...}}\}}

after run **sfi**, the result will be:

	{{/path/to/file_from_variable}}

and **re-run sfi** to import that file.

## Usage
	
	sfi [OPTION] [file]
	-v      print the version and exit
	-h      print this help and exit
	
quick and dirty:

	git clone https://github.com/bomboclat/sfi.git
	cd sfi
	make
	./sfi examples/index.html > result
	cat result
	rm result

You can also pass the template from stdin, for example:
	
	cat [templatefile] | sfi > result

Also values can be passed through stdin (using `{{...}}` in template file):

	printf "this is a string value" | sfi templatename > result

You can find other template examples in `examples` dir.

## Build and install
To compile:

	make

To install:

	make install
	
You can edit the `$DESTDIR` variable to install all files on specific directory:

	mkdir destdir
	DESTDIR=destdir make install
	
To uninstall:
	
	make uninstall
	
or

	DESTDIR=destdir make uninstall
