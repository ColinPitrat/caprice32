#include <gtest/gtest.h>

#include "disk.h"

namespace
{

class SectorReadTest : public testing::Test
{
  public:
    SectorReadTest()
    {
      data[0] = 1;
      data[512] = 2;
      sector.setData(data);
    }

  protected:
    t_sector sector;
    unsigned char data[1024] = {0};
};

TEST_F(SectorReadTest, NormalSector)
{
  sector.setSizes(1024, 1024);

  unsigned char* read1 = sector.getDataForRead();
  unsigned char* read2 = sector.getDataForRead();

  ASSERT_EQ(1, read1[0]);
  ASSERT_EQ(1, read2[0]);
}

TEST_F(SectorReadTest, WeakSector)
{
  sector.setSizes(512, 1024);

  unsigned char* read1 = sector.getDataForRead();
  unsigned char* read2 = sector.getDataForRead();

  // There's no reason to force a given order as long as 2 consecutive reads return 2 different versions
  ASSERT_NE(read1[0], read2[0]);
  // And the value is one of the 2 provided
  ASSERT_TRUE(read1[0] == 1 || read1[0] == 2);
  ASSERT_TRUE(read2[0] == 1 || read2[0] == 2);
}

TEST_F(SectorReadTest, LongSector)
{
  // Should work just as a normal sector of size 512
  sector.setSizes(1024, 512);

  unsigned char* read1 = sector.getDataForRead();
  unsigned char* read2 = sector.getDataForRead();

  ASSERT_EQ(1, read1[0]);
  ASSERT_EQ(1, read2[0]);
  ASSERT_EQ(512, sector.getTotalSize());
}

}
