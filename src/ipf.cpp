// IPF disk image support (http://www.softpres.org/)
// Contributed by softpres.org

#ifdef WITH_IPF

#ifdef WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "cap32.h"
#include "disk.h"
#include "errors.h"
#include "fileutils.h"
#include "ipf.h"
#include "log.h"
#include "slotshandler.h"

extern t_CPC CPC;

// Track decoding variables
static bool fWrapped;
static unsigned int uPos, uLastPos, uDecoded;
static byte bLastData;
static int nDataClock;
static byte abDecoded[0x200000];	// 2MB
static word s_wCRC;

static struct CapsTrackInfoT1 cti;
static dword dwLockFlags = DI_LOCK_UPDATEFD|DI_LOCK_TYPE;

// Declare here all the CAPS functions we will need to import.
// This is not very elegant but we do not want to use a lib wrapper
// to keep things simpler.
static CapsLong (*_CAPSInit)(void);
static CapsLong (*_CAPSExit)(void);
static CapsLong (*_CAPSAddImage)(void);
static CapsLong (*_CAPSRemImage)(CapsLong);
static CapsLong (*_CAPSLockImage)(CapsLong, char *);
static CapsLong (*_CAPSUnlockImage)(CapsLong);
static CapsLong (*_CAPSGetImageInfo)(struct CapsImageInfo *, CapsLong id);
static CapsLong (*_CAPSLockTrack)(void *, CapsLong, CapsULong, CapsULong, CapsULong);
static CapsLong (*_CAPSUnlockTrack)(CapsLong, CapsULong, CapsULong);
static CapsLong (*_CAPSGetVersionInfo)(void *, CapsULong);
#ifdef WINDOWS
static HMODULE handle;
#else
static void *handle;
#endif

static int unload_caps_library(void)
{
#ifdef WINDOWS
	if (!FreeLibrary(handle)) return ERR_IPF_DYNLIB_LOAD;
	return 0;
#else
	if (dlclose(handle) != 0) return ERR_IPF_DYNLIB_LOAD;
	return 0;
#endif
}

static int load_caps_library(void)
{
#ifdef WINDOWS
	handle = LoadLibrary("CAPSImg.dll");
	if (!handle)
	{
		LOG_ERROR("CAPSImg.dll is required for IPF support");
		return ERR_IPF_DYNLIB_LOAD;
	}
	else
	{
		_CAPSInit = reinterpret_cast<CapsLong (*)(void)>(GetProcAddress(handle, "CAPSInit"));
		if (_CAPSInit == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSExit = reinterpret_cast<CapsLong (*)(void)>(GetProcAddress(handle, "CAPSExit"));
		if (_CAPSExit == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSAddImage = reinterpret_cast<CapsLong (*)(void)>(GetProcAddress(handle, "CAPSAddImage"));
		if (_CAPSAddImage == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSRemImage = reinterpret_cast<CapsLong (*)(CapsLong)>(GetProcAddress(handle, "CAPSRemImage"));
		if (_CAPSRemImage == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSLockImage = reinterpret_cast<CapsLong (*)(CapsLong, char *)>(GetProcAddress(handle, "CAPSLockImage"));
		if (_CAPSLockImage == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSUnlockImage = reinterpret_cast<CapsLong (*)(CapsLong)>(GetProcAddress(handle, "CAPSUnlockImage"));
		if (_CAPSUnlockImage == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSGetImageInfo = reinterpret_cast<CapsLong (*)(struct CapsImageInfo *, CapsLong)>(GetProcAddress(handle, "CAPSGetImageInfo"));
		if (_CAPSGetImageInfo == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSLockTrack = reinterpret_cast<CapsLong (*)(void *, CapsLong, CapsULong, CapsULong, CapsULong)>(GetProcAddress(handle, "CAPSLockTrack"));
		if (_CAPSLockTrack == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSUnlockTrack = reinterpret_cast<CapsLong (*)(CapsLong, CapsULong, CapsULong)>(GetProcAddress(handle, "CAPSUnlockTrack"));
		if (_CAPSUnlockTrack == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		_CAPSGetVersionInfo = reinterpret_cast<CapsLong (*)(void *, CapsULong)>(GetProcAddress(handle, "CAPSGetVersionInfo"));
		if (_CAPSGetVersionInfo == NULL) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
		return 0;
	}
#else
	handle = dlopen("libcapsimage.so.4", RTLD_LAZY);
	if (!handle)
	{
		LOG_ERROR("Cannot open libcapsimage.so.4 needed for IPF support");
		return ERR_IPF_DYNLIB_LOAD;
	}
	dlerror();
	_CAPSInit = reinterpret_cast<CapsLong (*)(void)>(dlsym(handle, "CAPSInit"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSExit = reinterpret_cast<CapsLong (*)(void)>(dlsym(handle, "CAPSExit"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSAddImage = reinterpret_cast<CapsLong (*)(void)>(dlsym(handle, "CAPSAddImage"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSRemImage = reinterpret_cast<CapsLong (*)(CapsLong)>(dlsym(handle, "CAPSRemImage"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSLockImage = reinterpret_cast<CapsLong (*)(CapsLong, char *)>(dlsym(handle, "CAPSLockImage"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSUnlockImage = reinterpret_cast<CapsLong (*)(CapsLong)>(dlsym(handle, "CAPSUnlockImage"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSGetImageInfo = reinterpret_cast<CapsLong (*)(struct CapsImageInfo *, CapsLong)>(dlsym(handle, "CAPSGetImageInfo"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSLockTrack = reinterpret_cast<CapsLong (*)(void *, CapsLong, CapsULong, CapsULong, CapsULong)>(dlsym(handle, "CAPSLockTrack"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSUnlockTrack = reinterpret_cast<CapsLong (*)(CapsLong, CapsULong, CapsULong)>(dlsym(handle, "CAPSUnlockTrack"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	_CAPSGetVersionInfo = reinterpret_cast<CapsLong (*)(void *, CapsULong)>(dlsym(handle, "CAPSGetVersionInfo"));
	if (dlerror()) { unload_caps_library(); return ERR_IPF_DYNLIB_LOAD; }
	return 0;
#endif
}
// CRC-16 CCITT, for track level header and data checksums
static void Crc (byte b_)
{
   static word awCRC[256];

   if (!awCRC[1])
   {
      for (int i = 0 ; i < 256 ; i++)
      {
         word w = i << 8;
         for (int j = 0 ; j < 8 ; j++)
             w = (w << 1) ^ ((w & 0x8000) ? 0x1021 : 0);
         awCRC[i] = w;
      }
   }

   s_wCRC = (s_wCRC << 8) ^ awCRC[((s_wCRC >> 8) ^ b_) & 0xff];
}

// Read an MFM byte from the track
static byte ReadByte ()
{
	byte b;

	// Convert bit position to byte offset+shift, and advance by 8 MFM bits
	unsigned int uOffset = uPos >> 3, uShift = uPos & 7;
	uPos += 8;

	// Byte-aligned?
	if (!uShift)
		b = cti.trackbuf[uOffset];
	else
		b = (cti.trackbuf[uOffset] << uShift) | (cti.trackbuf[uOffset+1] >> (8 - uShift));

	// Track wrapped?
	if (uPos >= cti.tracklen)
	{
		// Add the remaining bits from the start of the track (assumes cti.tracklen >= 8)
		unsigned int uWrapBits = uPos - cti.tracklen;
		b &= ~(((1 << uWrapBits)) - 1);
		b |= cti.trackbuf[0] >> (8 - uWrapBits);

		uPos -= cti.tracklen;
		fWrapped = true;
	}

	return b;
}

// Read an MFM word from the track
static word ReadWord ()
{
	uLastPos = uPos;

	// Read 16 bits of interleaved MFM clock and data bits
	byte b1 = ReadByte(), b2 = ReadByte();

	// Extract the clock bits
	byte bClock =((b1 << 0) & 0x80) | ((b1 << 1) & 0x40) | ((b1 << 2) & 0x20) | ((b1 << 3) & 0x10) |
				 ((b2 >> 4) & 0x08) | ((b2 >> 3) & 0x04) | ((b2 >> 2) & 0x02) | ((b2 >> 1) & 0x01);

	// Extract the data bits
	byte bData = ((b1 << 1) & 0x80) | ((b1 << 2) & 0x40) | ((b1 << 3) & 0x20) | ((b1 << 4) & 0x10) |
				 ((b2 >> 3) & 0x08) | ((b2 >> 2) & 0x04) | ((b2 >> 1) & 0x02) | ((b2 >> 0) & 0x01);

	// Calculate the expected clock bits for the data byte
	byte bGoodClock = 0;
	if (!(bData & 0x80) && !(bLastData & 1)) bGoodClock |= 0x80;
	if (!(bData & 0xc0)) bGoodClock |= 0x40;
	if (!(bData & 0x60)) bGoodClock |= 0x20;
	if (!(bData & 0x30)) bGoodClock |= 0x10;
	if (!(bData & 0x18)) bGoodClock |= 0x08;
	if (!(bData & 0x0c)) bGoodClock |= 0x04;
	if (!(bData & 0x06)) bGoodClock |= 0x02;
	if (!(bData & 0x03)) bGoodClock |= 0x01;

	// Determine the actual clock difference, needed to recognise address marks
	bClock ^= bGoodClock;

	// Store the decoded data for easier sector data extraction
	abDecoded[uDecoded++] = bLastData = bData;

	// Return a word containing the separated clock and data bytes
	return (bClock << 8) | bData;
}

// Read a data byte, discarding the clock bits
static byte ReadDataByte()
{
	return ReadWord() & 0xff;
}

// Process the MFM track data to extract sector headers and data fields
static void ReadTrack (t_track *pt_)
{
	t_sector *ps = nullptr;
	unsigned int uHeaderOffset = 0;

	// Initialise scanning state
	uPos = uDecoded = 0;
	fWrapped = false;
	bLastData = 0x00;
	nDataClock = 0;

	// Return if no track data or if updating and track not flakey
	if (!cti.tracklen)
		return;
	else if (pt_->data && !(cti.type & CTIT_FLAG_FLAKEY))
		return;

	// Loop until end of track, completing sectors that spanning the track wrapping point
	while (!fWrapped || ps)
	{
		byte bAM;

		// Continue iff we find 3xA1 with missing clock bits
		if (ReadWord() != 0x04a1) { uPos -= 15; uDecoded--; continue; }	// advance by 1 bit on mismatches, discard decoded byte
		if (ReadWord() != 0x04a1) continue;
		if (ReadWord() != 0x04a1) continue;

		// CRC 3xA1 + address mark
		s_wCRC = 0xcdb4;
		Crc(bAM = ReadDataByte());

		switch (bAM)
		{
			case 0xfe:	// id address mark
			{
				// Check we've room for another sector
				if (pt_->sectors >= DSK_SECTORMAX)
					continue;

				// Allocate new sector
				ps = &pt_->sector[pt_->sectors++];

				// Read ID header and CRC
				Crc(ps->CHRN[0] = ReadDataByte());
				Crc(ps->CHRN[1] = ReadDataByte());
				Crc(ps->CHRN[2] = ReadDataByte());
				Crc(ps->CHRN[3] = ReadDataByte());
				Crc(ReadDataByte());
				Crc(ReadDataByte());

				// If the header CRC is bad we ignore it
				if (s_wCRC)
				{
					pt_->sectors--;
					ps = nullptr;
					continue;
				}

				// Remember the track position of the header
				uHeaderOffset = uLastPos;
				continue;
			}

			case 0xfb: case 0xfa:	// data address mark (+alt)
			case 0xf8: case 0xf9:	// data address mark with control mark (+alt)
			{
				// Remember where the data started and the wrap status
				unsigned int uDataPos = uPos;
				bool fDataWrapped = fWrapped;

				// Ignore the data field if there's no associated header
				if (!ps)
					continue;

				// Check the byte distance between header and data fields
				unsigned int uOffset = (uLastPos - uHeaderOffset) >> 4;

				// If it's too close or too far, the data isn't accessible
				if (uOffset < 32 || uOffset >= 64)
				{
					ps->flags[1] &= ~0x01;	// no data
					ps = nullptr;
					continue;
				}

				// Flag the control mark if the DAM indicates one
				if (bAM == 0xf8 || bAM == 0xf9)
					ps->flags[1] |= 0x40;

				// Set the data position in the buffer and sector size
				ps->setData(abDecoded + uDecoded);
				unsigned int sector_size = (ps->CHRN[3] <= 7) ? (128 << ps->CHRN[3]) : 0x8000;
				ps->setSizes(sector_size, sector_size);

				// Decode and CRC the data field
				for (unsigned int u = 0 ; u < ps->getTotalSize() ; u++)
					Crc(ReadDataByte());

				// Include data CRC bytes
				Crc(ReadDataByte());
				Crc(ReadDataByte());

				// Bad CRC?
				if (s_wCRC)
				{
					// Flag a data CRC error
					ps->flags[0] |= 0x20;
					ps->flags[1] |= 0x20;
				}

				// To allow for read-track protections, overread the first data field to 4K
				if (pt_->sectors == 1 && ps->getTotalSize() < 4096)
				{
					for (unsigned int u = 0 ; u < (4096 - ps->getTotalSize()) ; u++)
						Crc(ReadDataByte());
				}

				// Sector complete
				ps = nullptr;

				// Step back up to just after the data position to check for more address marks
				// as sectors could be overlapping
				uPos = uDataPos;
				fWrapped = fDataWrapped;
				continue;
			}
		}
	}

	// Data buffer not allocated yet?
	if (!pt_->data)
	{
		// Allocate enough for the full decoded size, allowing for expanded overlapping sectors
		memcpy(pt_->data = static_cast<byte*>(malloc(uDecoded)), abDecoded, pt_->size = uDecoded);
		auto offset = (pt_->data-abDecoded);

		// Set the sector data pointers for the new buffer
		for (unsigned int u = 0 ; u < pt_->sectors ; u++)
			pt_->sector[u].setData(pt_->sector[u].getDataForWrite()+offset);
	}
}

// Track hook, called each disk rotation to allow flakey data to be updated
void ipf_track_hook (t_drive *drive)
{
	byte cyl = drive->current_track, head = drive->current_side;
	long id = drive->ipf_id;

	// Re-lock and update the track (note: don't use CAPSUnlockTrack() first as it resets the flakey data RNG!)
	cti.type = 1;
	if (_CAPSLockTrack(reinterpret_cast<CapsTrackInfo*>(&cti), id, cyl, head, dwLockFlags) == imgeOk)
	{
		t_track *pt = &drive->track[cyl][head];

		if (!cti.tracklen)
			memset(pt, 0, sizeof(*pt));
		else
		{
			// Convert track length to bits if supported
			if (!(dwLockFlags & DI_LOCK_TRKBIT)) cti.tracklen <<= 3;

			ReadTrack(pt);
		}
	}
}

// Eject hook, for additional disk image clean-up
void ipf_eject_hook (t_drive *drive)
{
	long id = drive->ipf_id;

	_CAPSUnlockImage(id);
	_CAPSRemImage(id);
	_CAPSExit();
	unload_caps_library();
	drive->altered = 0; // discard modifications
	drive->eject_hook = nullptr;
}

int ipf_load (FILE *pfileIn, t_drive *drive)
{
  // IPF library needs a filename to be provided so we have to create a new temporary file.
  // This file is not deleted.
  // TODO(cpitrat): register the file for cleanup somewhere (e.g: at caprice exit)
  FILE *pfileOut = nullptr;
  char *tmpFilePath = nullptr;
  std::vector<std::string> prefixes = { "/tmp", "." };
  for (const auto &prefix : prefixes) {
    tmpFilePath = tempnam(prefix.c_str(), ".cap32_tmp_");
    if (tmpFilePath == nullptr) {
      LOG_ERROR("Couldn't load IPF file: Couldn't generate temporary file name: " << strerror(errno));
      return ERR_DSK_INVALID; // couldn't create output file
    }
    LOG_DEBUG("Using temporary file: " << tmpFilePath);
    pfileOut = fopen(tmpFilePath, "w+b");
    if (pfileOut != nullptr) {
      break;
    }
  }

  if (!file_copy(pfileIn, pfileOut)) {
    LOG_ERROR("Error while copying file");
    return ERR_DSK_INVALID;
  }
  if (fclose(pfileOut) != 0) {
    LOG_ERROR("Error while closing temporary file");
    return ERR_DSK_INVALID;
  }

  return ipf_load(tmpFilePath, drive);
}

// Attempt to load the supplied file as an IPF disk image
int ipf_load (const std::string &filename, t_drive *drive)
{
	char sz[4];
	long id = -1;
	struct CapsImageInfo cii;
	struct CapsVersionInfo vi = { 0, 0, 0, 0 };

	dsk_eject(drive);

	FILE *f = fopen(filename.c_str(), "rb");
	if (!f)
    {
		LOG_ERROR("Couldn't open file: " << filename);
		return ERR_DSK_INVALID;
    }

	// Check for IPF file signature
	if (!fread(sz, 4, 1, f) || fclose(f) || memcmp(sz, "CAPS", sizeof(sz)))
	{
		fclose(f);
		LOG_ERROR("Wrong IPF header in: " << filename);
		return ERR_DSK_INVALID;
	}

	// Ensure the SPS library is present before we try to use the delay-load calls
	int sts = load_caps_library();
	if (sts) return sts;

	// Check that the DLL supports the CapsTrackInfoT1 structure we need
	if (_CAPSGetVersionInfo(&vi, 0) != imgeOk || vi.release < 4) // compatible DLL?
	{
		LOG_ERROR("IPF shared library is too old. Requiring version >=4. Please upgrade it");
		return ERR_DSK_INVALID;
	}

	// Use bit lengths if available
	dwLockFlags |= vi.flag & (DI_LOCK_OVLBIT|DI_LOCK_TRKBIT);

	// Initialise the library
	if (_CAPSInit() != imgeOk)
	{
		LOG_ERROR("IPF shared library initialisation failed!");
		return ERR_DSK_INVALID;
	}

	// Create a new image container
	id = _CAPSAddImage();

	// Attach the IPF file to the container
	if (_CAPSLockImage(id, const_cast<char*>(filename.c_str())) != imgeOk)
	{
		_CAPSRemImage(id);
		_CAPSExit();
		unload_caps_library();
		LOG_ERROR("Couldn't lock image: " << filename);
		return ERR_DSK_INVALID;
	}

	// Get details about the contents of the image
	if (_CAPSGetImageInfo(&cii, id) != imgeOk)
	{
		_CAPSRemImage(id);
		_CAPSExit();
		unload_caps_library();
		LOG_ERROR("Couldn't get image info: " << filename);
		return ERR_DSK_INVALID;
	}

	// Set up the internal drive details
	drive->tracks = cii.maxcylinder+1;
	drive->sides = cii.maxhead;
	drive->altered = 0;
	drive->track_hook = ipf_track_hook;
	drive->eject_hook = ipf_eject_hook;

	// Load all tracks from the image
	for (byte cyl = static_cast<byte>(cii.mincylinder); cyl <= cii.maxcylinder ; cyl++)
	{
		for (byte head = static_cast<byte>(cii.minhead); head <= cii.maxhead ; head++)
		{
			cti.type = 1;
			if (_CAPSLockTrack(reinterpret_cast<CapsTrackInfo*>(&cti), id, cyl, head, dwLockFlags) != imgeOk)
			{
				LOG_ERROR("Failed to lock IPF track, please upgrade IPF shared library.");
				_CAPSUnlockImage(id);
				_CAPSRemImage(id);
				_CAPSExit();
				unload_caps_library();
				return ERR_DSK_INVALID;
			}

			t_track *pt = &drive->track[cyl][head];

			if (!cti.tracklen)
				memset(pt, 0, sizeof(*pt));
			else
				ReadTrack(pt);

			_CAPSUnlockTrack(id, cyl, head);
		}
	}

	// Store the IPF id for later use
	drive->ipf_id = id;

	return 0;
}
#endif
