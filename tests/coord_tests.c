#include "coord_tests.h"

bool testmethod_indexes_consistent(void *);

Test test_indexes_consistent = {
	.name  = "Consitency of index and anti-index",
	.t     = testmethod_indexes_consistent,
	.cases = (void **)all_coordinates,
};
Test *coord_pre_init[] = {
	&test_indexes_consistent,
	NULL
};
TestSuite coord_pre_init_suite = {
	.setup    = NULL,
	.tests    = coord_pre_init,
	.teardown = NULL,
};

TestSuite *coord_suites[] = {
	&coord_pre_init_suite,
	NULL
};

bool
testmethod_indexes_consistent(void *a)
{
	uint64_t ui, uj;
	Cube c;
	Coordinate *coord;

	coord = (Coordinate *)a;

	if (coord->type != COMP_COORD)
		return true; /* Not applicable */

	gen_coord(coord);
	for (ui = 0; ui < coord->max; ui++) {
		indexers_makecube(coord->i, ui, &c);
		uj = indexers_getind(coord->i, &c);
		if (ui != uj) {
			fprintf(stderr, "Error with coordinate %s: "
				"%" PRIu64 " != %" PRIu64 "\n",
				coord->name, uj, ui);
			return false;
		}
	}

	return true;
}
