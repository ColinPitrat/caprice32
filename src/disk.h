#ifndef DISK_H
#define DISK_H

#include <string>

// FDC constants
#define DSK_BPTMAX      8192
#define DSK_TRACKMAX    102   // max amount that fits in a DSK header
#define DSK_SIDEMAX     2
#define DSK_SECTORMAX   29    // max amount that fits in a track header

#define FDC_TO_CPU      0
#define CPU_TO_FDC      1

#define CMD_PHASE       0
#define EXEC_PHASE      1
#define RESULT_PHASE    2

#define SKIP_flag       1     // skip sectors with DDAM/DAM
#define SEEKDRVA_flag   2     // seek operation has finished for drive A
#define SEEKDRVB_flag   4     // seek operation has finished for drive B
#define RNDDE_flag      8     // simulate random DE sectors
#define OVERRUN_flag    16    // data transfer timed out
#define SCAN_flag       32    // one of the three scan commands is active
#define SCANFAILED_flag 64    // memory and sector data does not match
#define STATUSDRVA_flag 128   // status change of drive A
#define STATUSDRVB_flag 256   // status change of drive B

// This is only for debug purposes
std::string chrn_to_string(unsigned char* chrn);

typedef struct {
   char id[34];
   char unused1[14];
   unsigned char tracks;
   unsigned char sides;
   unsigned char unused2[2];
   unsigned char track_size[DSK_TRACKMAX*DSK_SIDEMAX];
} t_DSK_header;

typedef struct {
   char id[12];
   char unused1[4];
   unsigned char track;
   unsigned char side;
   unsigned char unused2[2];
   unsigned char bps;
   unsigned char sectors;
   unsigned char gap3;
   unsigned char filler;
   unsigned char sector[DSK_SECTORMAX][8];
} t_track_header;

class t_sector {
  public:
   unsigned char CHRN[4]; // the CHRN for this sector
   unsigned char flags[4]; // ST1 and ST2 - reflects any possible error conditions
   // TODO: Make data_ private once the last direct usage in src/ipf.cpp is removed
   unsigned char *data_; // pointer to sector data

   void setData(unsigned char* data) {
     data_ = data;
   }

   unsigned char* getDataForWrite() {
     return data_;
   }

   unsigned char* getDataForRead() {
     weak_read_version_ = (weak_read_version_ + 1) % weak_versions_;
     return &data_[weak_read_version_*size_];
   }

   void setSizes(unsigned int size, unsigned int total_size);

   unsigned int getTotalSize() {
     return total_size_;
   }

 private:
   unsigned int size_; // sector size in bytes
   unsigned int total_size_; // total data size in bytes
   unsigned int weak_versions_; // number of versions of this sector (should be 1 except for weak/random sectors)
   unsigned int weak_read_version_; // version of the sector to return when reading
};

typedef struct {
   unsigned int sectors; // sector count for this track
   unsigned int size; // track size in bytes
   unsigned char *data; // pointer to track data
   t_sector sector[DSK_SECTORMAX]; // array of sector information structures
} t_track;

struct t_drive {
   unsigned int tracks; // total number of tracks
   unsigned int current_track; // location of drive head
   unsigned int sides; // total number of sides
   unsigned int current_side; // side being accessed
   unsigned int current_sector; // sector being accessed
   unsigned int altered; // has the image been modified?
   unsigned int write_protected; // is the image write protected?
   unsigned int random_DEs; // sectors with Data Errors return random data?
   unsigned int flipped; // reverse the side to access?
   long ipf_id; // IPF ID if the track is loaded with a IPF image
   void (*track_hook)(struct t_drive *);	// hook called each disk rotation
   void (*eject_hook)(struct t_drive *);	// hook called on disk eject
   t_track track[DSK_TRACKMAX][DSK_SIDEMAX]; // array of track information structures
};

typedef struct {
   std::string label; // label to display in options dialog
   unsigned int tracks; // number of tracks
   unsigned int sides; // number of sides
   unsigned int sectors; // sectors per track
   unsigned int sector_size; // sector size as N value
   unsigned int gap3_length; // GAP#3 size
   unsigned char filler_byte; // default byte to use
   unsigned char sector_ids[2][16]; // sector IDs - indices: side, sector
} t_disk_format;

#endif
