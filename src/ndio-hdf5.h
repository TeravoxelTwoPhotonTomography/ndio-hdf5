//ndio-hdf5.h

/*
ndio_fmt_t fmt = ndioFormat("hdf5");
ndio_hdf5_param_t params = {"\my\data\set"};
ndioFormatSet(fmt,&params,sizeof(params));
*/

typedef struct ndio_hdf5_param_t_ {
  char dataset[1024];
} ndio_hdf5_param_t;
