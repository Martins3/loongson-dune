a.out: hello_world.c libdune.a
	gcc $< ../dune/libdune.a  -o $@
	./$@

so.out: hello_world.c libdune.so
	gcc -L../dune -l:libdune.so $< -o $@ -Wl,-R../dune
	# hello_world.c libdune.so
	./$@



libdune.a:
	$(MAKE) -C ../dune 

libdune.so:
	$(MAKE) -C ../dune 
