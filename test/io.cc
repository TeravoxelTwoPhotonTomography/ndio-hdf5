/** \file
    Testing reading of nD volumes from file hdf5.
    @cond TEST_F

    \todo APPEND test
*/

// solves a std::tuple problem in vs2012
#define GTEST_HAS_TR1_TUPLE     0
#define GTEST_USE_OWN_TR1_TUPLE 1

#include <gtest/gtest.h>
#include "config.h"
#include "nd.h"
#include "helpers.h"

#include <ndio-hdf5.h> // parameters


#define countof(e) (sizeof(e)/sizeof(*e))

static
struct _files_t
{ const char  *path;
  const char  *dset;
  nd_type_id_t type;
  size_t       ndim;
  size_t       shape[5];
}

file_table[] =
{ // Set a: Should be i16, but is read by mylib as u16
  {NDIO_HDF5_TEST_DATA_PATH"/test.h5" ,"/data",nd_i16,2,{333,222,1 ,1,1}},
  {NDIO_HDF5_TEST_DATA_PATH"/test.mat","/a"   ,nd_f64,3,{10 ,20 ,30,1,1}},
  {0}
};

struct HDF5:public testing::Test
{
  void SetUp()
  { ndioAddPluginPath(NDIO_BUILD_ROOT);
  }
};

TEST_F(HDF5,OpenClose)
{ struct _files_t *cur;
  // Examples that should fail to open
#if 0
  EXPECT_EQ(NULL,ndioOpen("does_not_exist.im.super.serious",ndioFormat("hdf5"),"r"));
  EXPECT_EQ(NULL,ndioOpen("does_not_exist.im.super.serious",ndioFormat("hdf5"),"w"));
  EXPECT_EQ(NULL,ndioOpen("",ndioFormat("hdf5"),"r"));
  EXPECT_EQ(NULL,ndioOpen("",ndioFormat("hdf5"),"w"));
  EXPECT_EQ(NULL,ndioOpen(NULL,ndioFormat("hdf5"),"r"));
  EXPECT_EQ(NULL,ndioOpen(NULL,ndioFormat("hdf5"),"w"));
#endif
  // Examples that should open
  for(cur=file_table;cur->path!=NULL;++cur)
  { ndio_t file=0;
    EXPECT_NE((void*)NULL,file=ndioOpen(cur->path,ndioFormat("hdf5"),"r"));
    EXPECT_STREQ("hdf5",ndioFormatName(file));
    ndioClose(file);
  }
}

TEST_F(HDF5,Shape)
{ struct _files_t *cur;
  for(cur=file_table;cur->path!=NULL;++cur)
  { ndio_t file=0;
    nd_t form;
    EXPECT_NE((void*)NULL,file=ndioOpen(cur->path,ndioFormat("hdf5"),"r"))<<cur->path;
    ASSERT_NE((void*)NULL,form=ndioShape(file))<<ndioError(file)<<"\n\t"<<cur->path;
    EXPECT_EQ(cur->type,ndtype(form))<<cur->path;
    EXPECT_EQ(cur->ndim,ndndim(form))<<cur->path;
    for(size_t i=0;i<cur->ndim;++i)
      EXPECT_EQ(cur->shape[i],ndshape(form)[i])<<cur->path;
    EXPECT_EQ(NULL,nderror(form));
    ndfree(form);
    ndioClose(file);
  }
}

TEST_F(HDF5,Read)
{ struct _files_t *cur;
  for(cur=file_table;cur->path!=NULL;++cur)
  { ndio_t file=0;
    nd_t vol;
    EXPECT_NE((void*)NULL,file=ndioOpen(cur->path,ndioFormat("hdf5"),"r"));
    ASSERT_NE((void*)NULL, vol=ndioShape(file))<<ndioError(file)<<"\n\t"<<cur->path;
    EXPECT_EQ(vol,ndref(vol,malloc(ndnbytes(vol)),nd_heap));
    EXPECT_EQ(file,ndioRead(file,vol));
    ndfree(vol);
    ndioClose(file);
  }
}

TEST_F(HDF5,ReadDataSet)
{ struct _files_t *cur;
  for(cur=file_table;cur->path!=NULL;++cur)
  { ndio_t file=0;
    nd_t vol;
    ndio_fmt_t *fmt=ndioFormat("hdf5");
    ndio_hdf5_param_t params={0};
    strcpy(params.dataset,cur->dset);
    EXPECT_NE((void*)NULL, ndioFormatSet(fmt,&params,sizeof(params)));
    EXPECT_NE((void*)NULL, file=ndioOpen(cur->path,fmt,"r"));
    ASSERT_NE((void*)NULL,  vol=ndioShape(file))<<ndioError(file)<<"\n\t"<<cur->path;
    EXPECT_EQ(vol,ndref(vol,malloc(ndnbytes(vol)),nd_heap));
    EXPECT_EQ(file,ndioRead(file,vol));
    ndfree(vol);
    ndioClose(file);
  }
}

TEST_F(HDF5,ReadSubarray)
{ struct _files_t *cur;
  for(cur=file_table;cur->path!=NULL;++cur)
  { ndio_t file=0;
    nd_t vol;
    size_t n;
    size_t pos[32]={0}; // assume the file has less than 32 dimensions
    EXPECT_NE((void*)NULL,file=ndioOpen(cur->path,ndioFormat("hdf5"),"r"));
    EXPECT_NE((void*)NULL, vol=ndioShape(file))<<ndioError(file)<<"\n\t"<<cur->path;
    ASSERT_GT(countof(pos),ndndim(vol));
    // setup to read center hypercube
    for(size_t i=0;i<ndndim(vol);++i)
      ndShapeSet(vol,i,ndshape(vol)[i]/2);
    EXPECT_EQ(vol,ndref(vol,malloc(ndnbytes(vol)),nd_heap));
    for(size_t i=0;i<ndndim(vol);++i)
      pos[i]=ndshape(vol)[i]/2;
    EXPECT_EQ(file,ndioReadSubarray(file,vol,pos,NULL));
    ndfree(vol);
    ndioClose(file);
  }
}

typedef ::testing::Types<
#if 0
  uint8_t
#else
  uint8_t,uint16_t,uint32_t,uint64_t,
   int8_t, int16_t, int32_t, int64_t,
  float, double
#endif
  > BasicTypes;

template<class T>
class HDF5_Typed:public ::testing::Test
{
public:
  nd_t a;
  HDF5_Typed() :a(0) {}
  void SetUp()
  { size_t shape[]={134,513,52,34};
    ndioAddPluginPath(NDIO_BUILD_ROOT);
    EXPECT_NE((void*)NULL,ndreshape(cast<T>(a=ndinit()),countof(shape),shape))<<nderror(a);
    EXPECT_NE((void*)NULL,ndref(a,malloc(ndnbytes(a)),nd_heap))<<nderror(a);
  }
  void TearDown()
  { ndfree(a);
  }
};

TYPED_TEST_CASE(HDF5_Typed,BasicTypes);
TYPED_TEST(HDF5_Typed,Write)
{ nd_t vol;
  ndio_t file;
  ndio_t fin;
  EXPECT_NE((void*)NULL,ndioWrite(file=ndioOpen("testout.h5",NULL,"w"),this->a));
  ndioClose(file);
  EXPECT_NE((void*)NULL,fin=ndioOpen("testout.h5",NULL,"r"));
  ASSERT_NE((void*)NULL,vol=ndioShape(fin))<<ndioError(fin)<<"\n\t"<<"testout.h5";
  ndioClose(fin);
  { int i;
    EXPECT_EQ(-1,i=firstdiff(ndndim(this->a),ndshape(this->a),ndshape(vol)))
        << "\torig shape["<<i<<"]: "<< ndshape(this->a)[i] << "\n"
        << "\tread shape["<<i<<"]: "<< ndshape(vol)[i] << "\n";
  }
}

TYPED_TEST(HDF5_Typed,WriteDataSet)
{ nd_t vol;
  ndio_t file;
  ndio_t fin;
  ndio_fmt_t *fmt=0;
  ndio_hdf5_param_t params={0};
  strncpy(params.dataset,"/custom",sizeof(params.dataset));
  EXPECT_NE((void*)NULL, fmt=ndioFormatIncRef(ndioFormat("hdf5")));
  EXPECT_NE((void*)NULL, ndioFormatSet(fmt,&params,sizeof(params)));

  EXPECT_NE((void*)NULL,ndioWrite(file=ndioOpen("testout.h5",fmt,"w"),this->a));
  ndioClose(file);
  EXPECT_NE((void*)NULL,fin=ndioOpen("testout.h5",fmt,"r"));
  ASSERT_NE((void*)NULL,vol=ndioShape(fin))<<ndioError(fin)<<"\n\t"<<"testout.h5";
  ndioClose(fin);
  ndioFormatDecRef(fmt);
  { int i;
    EXPECT_EQ(-1,i=firstdiff(ndndim(this->a),ndshape(this->a),ndshape(vol)))
        << "\torig shape["<<i<<"]: "<< ndshape(this->a)[i] << "\n"
        << "\tread shape["<<i<<"]: "<< ndshape(vol)[i] << "\n";
  }
}
/// @endcond
