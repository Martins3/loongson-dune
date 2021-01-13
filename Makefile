SUBDIRS	= kern libdune

all: $(SUBDIRS)
	echo "sync code firstly"
	rsync --delete -avzh maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/loongson/dune/
