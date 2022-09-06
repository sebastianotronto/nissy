int
array_ep_to_epos(int *ep, int *ss)
{
	int epos[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int eps[4];
	int i, j, is;

	for (i = 0, is = 0; i < 12; i++) {
		for (j = 0; j < 4; j++) {
			if (ep[i] == ss[j]) {
				eps[is++] = j;
				epos[i] = 1;
			}
		}
	}

	for (i = 0; i < 4; i++)
		swap(&epos[ss[i]], &epos[i+8]);

	return 24 * subset_to_index(epos, 12, 4) + perm_to_index(eps, 4);
}

void
epos_to_compatible_ep(int epos, int *ep, int *ss)
{
	int i, j, k, other[8];
	bool flag;

	for (i = 0; i < 12; i++)
		ep[i] = -1;

	epos_to_partial_ep(epos, ep, ss);

	for (i = 0, j = 0; i < 12; i++) {
		flag = false;
		for (k = 0; k < 4; k++)
			flag = flag || (i == ss[k]);
		if (!flag)
			other[j++] = i;
	}
		
	for (i = 0, j = 0; i < 12; i++)
		if (ep[i] == -1)
			ep[i] = other[j++];
}

void
epos_to_partial_ep(int epos, int *ep, int *ss)
{
	int i, is, eposs[12], eps[4];

	index_to_perm(epos % FACTORIAL4, 4, eps);
	index_to_subset(epos / FACTORIAL4, 12, 4, eposs);

	for (i = 0; i < 4; i++)
		swap(&eposs[ss[i]], &eposs[i+8]);

	for (i = 0, is = 0; i < 12; i++)
		if (eposs[i])
			ep[i] = ss[eps[is++]];
}

void
fix_eorleoud(CubeArray *arr)
{
	int i;

	for (i = 0; i < 12; i++) {
		if ((edge_slice(i) == 0 && edge_slice(arr->ep[i]) != 0) ||
		    (edge_slice(i) != 0 && edge_slice(arr->ep[i]) == 0)) {
			arr->eorl[i] = 1 - arr->eofb[i];
		} else {
			arr->eorl[i] = arr->eofb[i];
		}

		if ((edge_slice(i) == 2 && edge_slice(arr->ep[i]) != 2) ||
		    (edge_slice(i) != 2 && edge_slice(arr->ep[i]) == 2)) {
			arr->eoud[i] = 1 - arr->eofb[i];
		} else {
			arr->eoud[i] = arr->eofb[i];
		}
	}
}

void
fix_cofbcorl(CubeArray *arr)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (i % 2 == arr->cp[i] % 2) {
			arr->cofb[i] = arr->coud[i];
			arr->corl[i] = arr->coud[i];
		} else {
			if (arr->cp[i] % 2 == 0) {
				arr->cofb[i] = (arr->coud[i]+1)%3;
				arr->corl[i] = (arr->coud[i]+2)%3;
			} else {
				arr->cofb[i] = (arr->coud[i]+2)%3;
				arr->corl[i] = (arr->coud[i]+1)%3;
			}
		}
	}
}

Cube
admissible_ep(Cube cube, PieceFilter f)
{
	CubeArray *arr = new_cubearray(cube, f);
	Cube ret;
	bool used[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	for (i = 0; i < 12; i++)
		if (arr->ep[i] != -1)
			used[arr->ep[i]] = true;

	for (i = 0, j = 0; i < 12; i++) {
		for ( ; j < 11 && used[j]; j++);
		if (arr->ep[i] == -1)
			arr->ep[i] = j++;
	}

	ret = arrays_to_cube(arr, pf_ep);
	free_cubearray(arr, f);

	return ret;
}

int
edge_slice(Edge e) {
	if (e < 0 || e > 11)
		return -1;

	if (e == FR || e == FL || e == BL || e == BR)
		return 0;
	if (e == UR || e == UL || e == DR || e == DL)
		return 1;

	return 2;
}

int
piece_orientation(Cube cube, int piece, char *orientation)
{
	int arr[12], n, b, x;

	if (!strcmp(orientation, "eofb")) {
		x = cube.eofb;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eorl")) {
		x = cube.eorl;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eoud")) {
		x = cube.eoud;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "coud")) {
		x = cube.coud;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "corl")) {
		x = cube.corl;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "cofb")) {
		x = cube.cofb;
		n = 8;
		b = 3;
	} else {
		return -1;
	}

	int_to_sum_zero_array(x, b, n, arr);
	if (piece < n)
		return arr[piece];

	return -1;
}
