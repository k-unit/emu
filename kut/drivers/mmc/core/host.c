static int kut_host_index;

void kut_host_index_reset(void)
{
	kut_host_index = 0;
}

int kut_host_index_get_next(void)
{
	return kut_host_index++;
}

