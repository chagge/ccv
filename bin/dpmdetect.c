#include "ccv.h"
#include <sys/time.h>
#include <ctype.h>

unsigned int get_current_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(int argc, char** argv)
{
	assert(argc >= 3);
	int i, j;
	ccv_enable_default_cache();
	ccv_dense_matrix_t* image = 0;
	ccv_read(argv[1], &image, CCV_IO_ANY_FILE);
	ccv_dpm_mixture_model_t* model = ccv_load_dpm_mixture_model(argv[2]);
	ccv_dpm_param_t params = { .interval = 8, .min_neighbors = 1, .flags = 0, .threshold = 0.4 };
	if (image != 0)
	{
		unsigned int elapsed_time = get_current_time();
		ccv_array_t* seq = ccv_dpm_detect_objects(image, &model, 1, params);
		elapsed_time = get_current_time() - elapsed_time;
		if (seq)
		{
			for (i = 0; i < seq->rnum; i++)
			{
				ccv_root_comp_t* comp = (ccv_root_comp_t*)ccv_array_get(seq, i);
				printf("%d %d %d %d %f %d\n", comp->rect.x, comp->rect.y, comp->rect.width, comp->rect.height, comp->confidence, comp->pnum);
				for (j = 0; j < comp->pnum; j++)
				printf("| %d %d %d %d %f\n", comp->part[j].rect.x, comp->part[j].rect.y, comp->part[j].rect.width, comp->part[j].rect.height, comp->part[j].confidence);
			}
			printf("total : %d in time %dms\n", seq->rnum, elapsed_time);
			ccv_array_free(seq);
		} else {
			printf("elapsed time %dms\n", elapsed_time);
		}
		ccv_matrix_free(image);
	} else {
		FILE* r = fopen(argv[1], "rt");
		if (argc == 4)
			chdir(argv[3]);
		if(r)
		{
			size_t len = 1024;
			char* file = (char*)malloc(len);
			ssize_t read;
			while((read = getline(&file, &len, r)) != -1)
			{
				while(read > 1 && isspace(file[read - 1]))
					read--;
				file[read] = 0;
				image = 0;
				ccv_read(file, &image, CCV_IO_GRAY | CCV_IO_ANY_FILE);
				assert(image != 0);
				ccv_array_t* seq = ccv_dpm_detect_objects(image, &model, 1, params);
				if (seq != 0)
				{
					for (i = 0; i < seq->rnum; i++)
					{
						ccv_root_comp_t* comp = (ccv_root_comp_t*)ccv_array_get(seq, i);
						printf("%s %d %d %d %d %f %d\n", file, comp->rect.x, comp->rect.y, comp->rect.width, comp->rect.height, comp->confidence, comp->pnum);
						for (j = 0; j < comp->pnum; j++)
						printf("| %d %d %d %d %f\n", comp->part[j].rect.x, comp->part[j].rect.y, comp->part[j].rect.width, comp->part[j].rect.height, comp->part[j].confidence);
					}
					ccv_array_free(seq);
				}
				ccv_matrix_free(image);
			}
			free(file);
			fclose(r);
		}
	}
	ccv_drain_cache();
	return 0;
}
