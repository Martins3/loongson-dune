all:
	rsync --delete -avzh maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/loongson/dune/
	gcc -DLOONGSON dune.c -o dune.out
	./dune.out

cross:
	$(CC) dune.c -o dune.out
