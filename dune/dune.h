int dune_enter();

#define DUNE_ENTER                                                             \
	do {                                                                   \
		if (dune_enter()) {                                            \
			return 1;                                              \
		}                                                              \
	} while (0)
