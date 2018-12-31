/////////////////////////////////////////////////////
//
// EpsLin v1.44 (c) Juha Forsten 29 Dec / 2018
// -------------------------------------------------
//  * File utility for Ensoniq samplers (EPS/ASR)
//
//
//  ------------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// -------------------------------------------------------------------------
//
//  TO COMPILE:
//  ===========
//
//  Linux /  Mac OS X 
//  ------------------
//
//    Just type:
//
//      gcc EpsLin_v1.44.c -o epslin
//
//  Windows
//  -------
//
//    1) Install Cygwin 32-bit (with GCC): https://www.cygwin.com/
//
//    2) To access floppy you have to install "fdrawcmd.sys" from
//          http://simonowen.com/fdrawcmd/
//
//       and get the header file from
//          http://simonowen.com/fdrawcmd/fdrawcmd.h
//
//       (like using: "curl https://simonowen.com/fdrawcmd/fdrawcmd.h --output fdrawcmd.h")
//
//       and put it in the same dir as the EpsLin source
//   
//    3) gcc EpsLin_v1.44.c -o epslin
//
//    4) You need cygwin DLL (cygwin1.dll) in the same folder when running epslin.exe outside cygwin shell.
//       (This DLL is found in C:/cygwin/bin/ or you can load it from the cygwin web site)
//
//
//  HISTORY
//  =======
//
//  v.1.44:
//        - Mac OSX support
//        - JSON output
//        - crash fixes
//
//  v.1.43:
//        - Small fixes for modern Linux versions
//          (write permissions, getopt changes)
//        - Verified Rasberry Pi compatibility (excluding floppy of course)

//  v.1.42:
//        - [Windows] Had to optimize the I/O functions involved
//          when Efes are GET/ERASED/PUT from/to media. This increased
//          the run time memory consumption, as the whole FAT
//          table are loaded to memory in order to avoid
//          seeks in CD-ROM (/dev/scd) and to write Iomega
//		      Zip-drives which don't support single byte writes.
//          As the load/save time of the FAT table depends on the size
//          of the media, it _might_ take quite a long time for some media,
//          thus making the get/put/erase slow. As the Zip drive is the only
//	        device to test this, it happens to take only 2 seconds to
//          read+write the table, so it's quite reasonable.
//          If you send me a SCSI device, that is very slow for this reason,
//          I'll try to optimize the functions more - until that,
//          for me it's fast enough.. ;-)
//        - Options that accept parameter 'a' (= all), now starts
//          at idx 1 (not 0 as previous versions).
//        - Added progress info when Efes are put/get.
//
//  v.1.41:
//        - [Windows] Added support for /dev/scd layer in Cygwin..
//          Since the seeks are posible only in 2048 byte multiples
//          had to make some mods to ReadBlock etc. functions.
//          Minor penalties in performance..
//
//  v.1.40: NOW PORTED TO WINDOWS!!
//        - Uses Simon Owens "fdrawcmd.sys" (http://simonowen.com/fdrawcmd/)
//          driver for low-level floppy access. Thank's Simon!
//        - Minor differencies between Cygwin (Win32) and Linux in command
//          line arguments (for ex. Cygwin requires allways parameter for -p)
//        - Should support also SCSI devices trough Cygwin layer
//        - Some minor fixes
//
//  v.1.37:
//        - Fixed nasty bug in dir-argument (-d) parsing, that
//          sometimes leads an error "ERROR: Index '0' is not a directory!".
//          Hopefully now solved..
//        - More investigation had to be done to find out how EPS knows, when to
//          load Song (instuments has the mask,that tells what location to load -
//          for song there are no such info). It seems that if index is 0, it is
//          not counted as a valid song, so no song data is loaded.
//          The BankInfo function is modified based on that knowledge.
//
//          BTW: You can make deeper dir structures with EPS than banks can handle, so..
//               don't do it if you want to use files in those dirs with banks.. :-)
//
//        - Minor changes to BankInfo output. Paths starts with '/' and ends with no '/'
//
//
//  v.1.36:
//        - Option to join multifile EFEs to single EFE file
//
//  v.1.35:
//        - Added missing CR,LF,EOF bytes to EFE header (cosmetic)
//        - Option to show info from Ensoniq Bank EFE
//        - Option to split EFE to multifile parts added
//        - Option to print directory listing in compact mode to make the
//          parsing for external programs easier.
//        - Some code clean-ups...
//
//  v.1.34:
//         - Support for Multi Files (ie. instruments, that don't fit in one floppy)
//         - Fixed (new) gcc errors
//
//  NOTES!
//  ======
//
//
//  * Some _commercial_ EPS/ASR tools corrupts the FAT.. You should avoid
//    those programs.. IF you have to use them, please at least check
//    the disks and image-files before use with EpsLin 'check' option (-C).
//    If you use those corrupted disk/images you may loose data!!
//
//   TODO:
//   =====
//
//   [CYGWIN]:
//   !! PutEfe doesn't work without idx number.. It's the Optget that
//      doesn't support optional arguments, so we'll wait for proper version...
//
//   [GENERAL]
//
//	 !! Format -fi optimize (now redundancy in allocated fat_block write)
//
//   !! MkDir max level.. (Bank can load files only max 11-level deep dirs)
//
//   !! Fix MKDIR Kludge in PutEFE...
//
//   !! Using Floppy with ReadImage sometimes mess up the FD controller !?
//
//   !! Add 'image copy' -feature (ie. make it possible to transfer
//      data in SCSI HD <-> ZIP, Zip requires '0x6d' at first byte in disk)
//
//   ! WAV to EFE & EFE to WAV conversions.
//
//   ! In EPS, the OS has to be in one contiguos part
//     * ASR don't mind...
//
//   - Dir-transfer function (copy files recursively..)
//     Usefull to transfer whole dir structures from medium to an another.
//     (Must take care of changing SCSI-ID in Bank-files ?)
//
//   - Erase: Recurse Dirs !!
//
//   - Format_n_Write -support (?!)
//
//   - First block not used by EPS/ASR. Write some info to that block ie. use
//     it as a "comments & info" for that disk.. The first letter has to be
//     'm' (='0x6d).. see above.
//

// Uncommnet to enter debug state...
//#define DEBUG

#define VERSION "v1.44"

#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#ifdef __APPLE__
#include <sys/uio.h>
#elif !defined(__CYGWIN__) 
#include <sys/io.h>
#endif

#include <sys/stat.h>
#include <sys/ioctl.h>

#ifdef __CYGWIN__

#include <windows.h>
#include <stdlib.h>
#include <ctype.h>

#include "fdrawcmd.h" // from http://simonowen.com/fdrawcmd/fdrawcmd.h

#else

// LINUX
//#include <linux/fd.h>
//#include <linux/fdreg.h>

// Floppy controller related stuff - from linux/fd.h

#define FDRAWCMD _IO(2, 0x58)
#define FDSETPRM _IOW(2, 0x42, struct floppy_struct) 

struct floppy_struct {
	unsigned int	size,	sect, head,	track, stretch;
	unsigned char	gap, rate, spec1,	fmt_gap; const char	* name;
};
struct floppy_raw_cmd {
	unsigned int flags; int *data; char *kernel_data; 
	struct floppy_raw_cmd *next; long length, phys_length; int buffer_length; 
	unsigned char rate, cmd_count, cmd[16], reply_count, reply[16];
	int track, resultcode, reserved1, reserved2;
};

#define O_BINARY 0 // In windows open is set to binary mode       \
                   // In linux this is not defined so set it to 0 \
                   // (this walue is ORed, so no harm is done..)

#endif

#ifdef __CYGWIN__
#define FIRST_BLOCK_MESSAGE "Created with EpsLin for Windows"
#else
#define FIRST_BLOCK_MESSAGE "Created with EpsLin for Linux"
#endif

#define BLOCK_SIZE 512
#define DIR_BLOCKS 2
#define DIR_START_BLOCK 3
#define DIR_END_BLOCK 4
#define MAX_DIR_DEPTH 10

#define ID_BLOCK 1
#define OS_BLOCK 2
#define DISK_LABEL_SIZE 8

#define MAX_NUM_OF_DIR_ENTRIES 39
#define EFE_SIZE 26

#define FAT_START_BLOCK 5

#define MAX_DISK_SECT 20

#define EPS_IMAGE_SIZE 819200
#define ASR_IMAGE_SIZE 1638400

// Buffer size for image copy
#define IMAGE_COPY_BUFFER_BLOCKS 100

#define DEFAULT_DISK_LABEL "DISK000 "

#define EDE_LABEL "EPS-16 Disk"
#define EDE_SKIP_START 0xA0
#define EDE_SKIP_SIZE 200
#define EDE_ID 0x03

#define EDA_LABEL "ASR-10 Disk"
#define EDA_SKIP_START 0x60
#define EDA_SKIP_SIZE 400
#define EDA_ID 0xCB

// Imagefile types
#define EPS_TYPE 'e'
#define ASR_TYPE 'a'
#define GKH_TYPE 'g'
#define EDE_TYPE 'E'
#define EDA_TYPE 'A'
#define OTHER_TYPE 'o'

//Modes for Bank Info
#define MODE_EPS 4
#define MODE_E16 23
#define MODE_ASR 30

// OS version info offsets (in EFE)
#define EPS_OS_POS 0x3A8
#define E16_OS_POS 0x390
#define ASR_OS_POS 0x6F2

// Modes
#define NONE 0
#define READ 1
#define WRITE 2
#define GET 3
#define PUT 4
#define ERASE 5
#define MKDIR 6
#define FORMAT 7
#define TEST 99

// Print modes
#define HUMAN_READABLE 0
#define COMPUTER_READABLE 1
#define JSON 2

// Return values
#define ERR -1
#define OK 0

// Error print-out & exit
#ifdef DEBUG
#define EEXIT(args) (fprintf(stderr, "(line: %d) ", __LINE__), fprintf args, exit(ERR))
#else
#define EEXIT(args) (fprintf args, exit(ERR))
#endif

// For 'getopt'
//extern char *optarg;
//extern int optind, opterr, optopt;

// Datatype for floppy disk access
#ifdef __CYGWIN__
typedef HANDLE FD_HANDLE;
#else
typedef int FD_HANDLE;
#endif

#ifdef __CYGWIN__
char *LastError()
{
  static char sz[256];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), sz, sizeof(sz), NULL);
  return GetLastError() ? sz : "no error";
}
#endif

// Temp-file name
static char tmp_file[13] = "EpsLinXXXXXX";
//static char tmp_file[64];

// Declaration of ReadBlocks
int ReadBlocks(char media_type, FD_HANDLE fd, int file, unsigned int start_block,
               unsigned int length, unsigned char *buffer);

// Temp-file cleanup -function (called by 'atexit')
static void CleanTmpFile(void)
{
  unlink(tmp_file);
}

// EPS/ASR-file types
static char *EpsTypes[40] = {
    //  0         1         2         3         4         5         6         7         8         9
    "(empty)",
    "EPS-OS ",
    "DIR/   ",
    "Instr  ",
    "EPS-Bnk",
    "EPS-Seq",
    "EPS-Sng",
    "EPS-Sys",
    ".. /   ",
    "EPS-Mac",

    // 10        11        12        13        14        15        16        17        18        19
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",
    "       ",

    // 20        21        22        23        24        25        26        27        28        29
    "       ",
    "       ",
    "       ",
    "E16-Bnk",
    "E16-Eff",
    "E16-Seq",
    "E16-Sng",
    "E16-OS ",
    "ASR-Seq",
    "ASR-Sng",

    // 30        31        32        33        34        35        36        37        38        39
    "ASR-Bnk",
    "ASR-Trk",
    "ASR-OS ",
    "ASR-Eff",
    "ASR-Mac",
    "       ",
    "       ",
    "       ",
    "       ",
    " ???   ",

};

//////////////
// ShowUsage
void ShowUsage()
{
  printf("\nEpsLin %s (c) J. Forsten (%s)\n", VERSION, __DATE__);
  printf("==========================================\n");
  printf("\nUsage: epslin [options] [imagefile or device] [Efe #0] [Efe #1] ... [Efe #N]\n\n");
  printf("Options:\n-------- \n\n");
  printf("   -r           READ image from disk. (Disk type (EPS/ASR) is autodetected!)\n");
  printf("                File extension (img/ede/eda) selects the format.\n\n");

  printf("   -w           WRITE image to disk. (Disk type (EPS/ASR) is autodetected!)\n");
  printf("                File extension (img/gkh/ede/eda) selects the format.\n\n");

  printf("   -fe,-fa, -fi FORMAT disk/image/device (a=asr, e=eps, i=image/device)\n");
  printf("                Use '-l' option to define disk label.\n");
  printf("                If imagefile doesn't exists it will be created. \n");
  printf("                Image size is then needed as an last argument.\n");
  printf("                Size is in bytes ('K' and 'M' can be used as a suffix)\n");
  printf("                If EPS/ASR DISK image is wanted, use size 'eps' or 'asr'\n");
  printf("                instead of numeric values!! See examples..\n\n");

  printf("                Examples: 'epslin -fe'                  : Format EPS-Disk\n");
  printf("                          'epslin -fa'                  : Format ASR-Disk\n");
  printf("                          'epslin -fi epsdisk.img eps'  : Create EPS disk image\n");
  printf("                          'epslin -fi asrdisk.img asr'  : Create ASR disk image\n");
  printf("                          'epslin -fi MyImage.img'      : Format MyImage.img\n");
  printf("                                                          NOTE: File exists!\n");
  printf("                          'epslin -fi MyCDROM.img 650M' : Create and format\n");
  printf("                                                          file size 650MB\n");
  printf("                          'epslin -l EpsHD -fi /dev/sda : Format SCSI disk\n");
  printf("                                                        : with label 'EpsHD'\n");

  printf("   -c           CONVERT imagefile from one format to another.\n");
  printf("                Supported conversions:\n");
  printf("                                               .img -> ede (EPS)\n");
  printf("                                               .img -> eda (ASR)\n");
  printf("                                               .gkh -> img (EPS)\n");
  printf("                                               .ede -> img (EPS)\n");
  printf("                                               .eda -> img (ASR)\n");
  printf("                Examples: 'epslin -c my_disk.img my_disk.ede'\n\n");

  printf("   -g index_list \n");
  printf("                GET Efe(s) from image/dev.\n");
  printf("                Examples: -g1,2,4   : Efe's 1,2 and 4.\n");
  printf("                          -g10-20   : from 10 to 20.\n");
  printf("                          -g10-     : from 10 to end.\n");
  printf("                          -g1,3-5,8 : 1,3 to 5 and 5.\n");
  printf("                          -ga       : All from that dir.\n\n");
#ifdef __CYGWIN__
  printf("   -p start_index \n");
  printf("                PUT Efe(s) to image/dev. The efe is saved to the\n");
  printf("                first empty index found. The search of empty index\n");
  printf("                can be set to start at start_index.\n");
  printf("                The Efe-files must be defined after image-file.\n");
  printf("                Examples: -p1  efe_files.efe : Put Efe in the idx 1 or\n");
  printf("                                               first empty index.\n");
  printf("                          -p1  *.efe         : Put all efes found in\n");
  printf("                                               the current dir.\n");
  printf("                          -p0 os_file.efe    : Put Operating System\n");
  printf("                                               to index 0.\n\n");

#else // Linux
  printf("   -p [start_index] \n");
  printf("                PUT Efe(s) to image/dev. The efe is saved to the\n");
  printf("                first empty index found. The search of empty index\n");
  printf("                can be set to start at start_index (default=1).\n");
  printf("                The Efe-files must be defined after image-file.\n");
  printf("                Examples: -p  efe_files.efe  : Put Efe in the first \n");
  printf("                                               empty index.\n");
  printf("                          -p  *.efe          : Put all efes found in\n");
  printf("                                               the current dir.\n");
  printf("                          -p0 os_file.efe    : Put Operating System\n");
  printf("                                               to index 0.\n\n");
#endif
  printf("   -e index_list \n");
  printf("                ERASE Efe(s) from image/dev.\n\n");

  printf("   -d path      DIRECTORY definition for most operations. \n");
  printf("                Use sub-directory _numbers_.\n");
  printf("                Example: -d12/4/7\n\n");

  printf("   -m dir_name  MAKE directory.\n\n");

  printf("   -C level     CHECK the disk/image. Gives detailed info\n");
  printf("                about the structure of the disk/image.\n");
  printf("                Level can be either 0 or 1.\n\n");

  printf("   -s efe_to_split slice_type\n");
  printf("                SPLIT big Efe to smaller pieces.\n");
  printf("                Example: -s Big.efe eps       : Slices to fit in EPS-Disk\n");
  printf("                Example: -s Big.efe asr       : Slices to fit in ASR-Disk\n\n");

  printf("   -b Bank.efe  BANK-info. Prints useful(?) inside info about bank efe\n\n");

  printf("   -P           PARSE-friendly output. Use with GUI/Fortned-softwares\n\n");
  printf("Image_file = Ensoniq(EPS/ASR)-Disk imagefile\n\n");
}

///////////////
// ParseRange
void ParseRange(char *optarg, char process_efe[MAX_NUM_OF_DIR_ENTRIES])
{
  size_t ssize;
  char tmp_str[80];
  int from_efe, to_efe, i;

  strcpy(tmp_str, optarg);

  ssize = strspn(tmp_str, "0123456789");
  tmp_str[ssize] = '\0';
  from_efe = atoi(tmp_str);

  if (strlen(optarg) == ssize + 1)
  {
    to_efe = MAX_NUM_OF_DIR_ENTRIES - 1;
  }
  else
  {
    to_efe = atoi(tmp_str + ssize + 1);
  }
  // Mark the range of efes
  for (i = from_efe; i <= to_efe; i++)
    process_efe[i] = 1;
}

////////////////
// ParseList
void ParseList(char *optarg, char process_efe[MAX_NUM_OF_DIR_ENTRIES])
{
  size_t ssize;
  char tmp_str[80];
  int j;

  strcpy(tmp_str, optarg);
  j = 0;
  // Slice the argument using ',' as a separator.
  while ((ssize = strspn(tmp_str + j, "0123456789-")) != 0)
  {
    tmp_str[ssize + j] = '\0';
    // List element is 'range'
    if ((index(tmp_str + j, '-')) != NULL)
    {
      ParseRange(tmp_str + j, process_efe);
    }
    else
    {
      process_efe[atoi(tmp_str + j)] = 1;
    }
    j = j + (ssize + 1);
  }
}

/////////////////////////
// ParseEntry
void ParseEntry(char *optarg, char process_efe[MAX_NUM_OF_DIR_ENTRIES])
{
  char *idx_new;
  int i;

  // All?
  if (strcmp(optarg, "a") == 0)
  {
    //Mark all
    for (i = 1; i < MAX_NUM_OF_DIR_ENTRIES; i++)
      process_efe[i] = 1;
  }
  else
  {
    // List ?
    if ((idx_new = index(optarg, ',')) == NULL)
    {
      // Range ?
      if (index(optarg, '-'))
      {
        ParseRange(optarg, process_efe);
      }
      else
      {
        // One number - Mark it
        process_efe[atoi(optarg)] = 1;
      }
    }
    else
    {
      ParseList(optarg, process_efe);
    }
  }
}

//////////////////////
// ParseDir
int ParseDir(char *dirpath_str, unsigned int *DirPath, unsigned int *subdir_cnt)
{

  char *idx_new, *idx, dir_str[80];

  // Validity check for 'dirpath_str' ie. can't
  // be over 70 characters (even that's too much!)
  if (strlen(dirpath_str) > 70)
  {
    EEXIT((stderr, "ERROR: Invalid directory path '%s'.\n", dirpath_str));
  }

  // Clear the 'dir_str'-buffer so that
  // parsing doesn't fail if buffer contains
  // character '/' at the "right" place.. (ie. bug fix :-)
  dir_str[strlen(dirpath_str) + 1] = 0;

  // Make a copy of path string
  strcpy(dir_str, dirpath_str);

  // Check if path starts with '/'
  if (dir_str[0] == '/')
  {
    if (dir_str[1] == '\0')
      return (OK);
    else
      dir_str[0] = ' ';
  }

  *subdir_cnt = 0;

  // Parse 'path'
  if (dir_str[strlen(dir_str) - 1] != '/')
  {
    dir_str[strlen(dir_str)] = '/';
    dir_str[strlen(dir_str) + 1] = '\0';
  }

  idx = dir_str;
  while ((idx_new = (char *)index(idx, '/')) != NULL)
  {
    *idx_new = '\0';
    DirPath[*subdir_cnt] = atoi(idx);
    idx = idx_new + 1;
    (*subdir_cnt)++;
  }
  return (OK);
}

////////////////////
// Name to DosName
void DosName(register char *dosname, char *name)
{
  register char *p;

  strcpy(dosname, name);
  while ((p = (char *)strchr(dosname, '*')) != NULL)
  {
    *p = '#';
  }

  // Uncomment if 'space' to '_' -conversion needed
  /*
    strcpy(dosname,name);
    while((p=strchr(dosname,' '))!=NULL) {
    *p='_';
    }    
  */
  strcat(dosname, ".efe");
}

////////////////////////////////
// Print the Dir-List (EFEs)
void PrintDir(unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE], unsigned int mode,
              char process_efe[MAX_NUM_OF_DIR_ENTRIES], char in_file[FILENAME_MAX],
              char media_type, char *DiskLabel, unsigned int free_blks, unsigned int used_blks,
              int printmode)
{
  unsigned int size, cont, start;
  unsigned int type, real_type, j, k;
  char name[13], dosname[64];
  char media[FILENAME_MAX];
  char type_text[10];
  int first_item = 1;

  switch (media_type)
  {
  case 'f':
    strcpy(media, "FILE: ");
    strcat(media, in_file);
    break;
  case 'e':
    strcpy(media, "EPS-DISK ");
    //strcat(media,DiskLabel);
    break;
  case 'a':
    strcpy(media, "ASR-DISK ");
    //strcat(media,DiskLabel);
    break;
  }

  // Print DirectoryList
  // ===================

  if (printmode == HUMAN_READABLE)
  {
    printf("\n------------------------------------------+---------------------------------+\n");
    printf(" Disk Label: %-28s |", DiskLabel);
    printf(" %-31s |\n", media);
    printf("------+----------+--------------+---------+------------------+--------------+\n");
    printf("  Idx |Type      | Name         |Blocks   | FileName         | FileSize     |\n");
    printf("------+----------+--------------+---------+------------------+--------------+\n");
  }
  else if (printmode == JSON)
  {
    printf("{\n");
    printf(" \"file\":\"%s\",\n", in_file);
    printf(" \"label\":\"%s\",\n", DiskLabel);
    printf(" \"used_blocks\":\"%ld\",\n", used_blks);
    printf(" \"used_bytes\":\"%ld\",\n", (unsigned long)(used_blks)*512);
    printf(" \"free_blocks\":\"%ld\",\n", free_blks);
    printf(" \"free_bytes\":\"%ld\",\n", (unsigned long)(free_blks)*512);
    printf(" \"items\":[\n");  
  }
  else
  {
    //Computer readable: Label,media, used block, used bytes, free blocks, free bytes
    printf("%s,%s,%d,%ld,%d,%ld\n", DiskLabel, media, used_blks, (unsigned long)(used_blks)*512, free_blks, (unsigned long)(free_blks)*512);
  }

  for (j = 0; j < MAX_NUM_OF_DIR_ENTRIES; j++)
  {

    //Name
    for (k = 0; k < 12; k++)
    {
      name[k] = Efe[j][k + 2];
    }
    name[12] = 0;

    DosName(dosname, name);

    size = (unsigned int)((Efe[j][14] << 8) + Efe[j][15]);
    cont = (unsigned int)((Efe[j][16] << 8) + Efe[j][17]);
    start = (unsigned long)((Efe[j][18] << 24) + (Efe[j][19] << 16) +
                            (Efe[j][20] << 8) + Efe[j][21]);
    type = Efe[j][1];
    real_type = type;

    // Check If type in unknown
    if (type > 39)
    {
      type = 39;
    }

    //if(1) {
    if (type != 0)
    {

      if (printmode == HUMAN_READABLE)
      {
        if (process_efe[j] == 1)
        {
          switch (mode)
          {
          case GET:
            printf("<-");
            break;

          case PUT:
          case MKDIR:
            printf("->");
            break;

          case ERASE:
            printf("!!");
            break;

          default:
            break;
          }
        }
        else
        {
          printf("  ");
        }
      }

      if (printmode == HUMAN_READABLE)
      {
        if ((type == 2) || (type == 8))
        {
          printf(" %02d | %s  | %-12s |         |                  |              |\n", j, EpsTypes[type], name);
        }
        else
        {
          if (type != 0)
          {
            strcpy(type_text, EpsTypes[type]);
            if (Efe[j][22] != 0)
            {
              char tmp_type_text[10];
              type_text[5] = '\0';
              sprintf(tmp_type_text, "%s(%2d)", type_text, Efe[j][22]);
              printf(" %02d | %s| %-12s | %7d | %s | %12ld |\n", j, tmp_type_text, name, size, dosname, (unsigned long)(size + 1) * 512);
            }
            else
            {
              printf(" %02d | %s  | %-12s | %7d | %s | %12ld |\n", j, type_text, name, size, dosname, (unsigned long)(size + 1) * 512);
            }
          }
        } // else
      }
      else if (printmode == JSON)
      {
        if (first_item == 0) { printf(",\n"); }
        
        printf("   {\n");
        printf("     \"index\":\"%d\",\n", j);
        printf("     \"type_id\":\"%d\",\n", real_type);
        printf("     \"type\":\"%s\",\n", EpsTypes[type]);
        printf("     \"name\":\"%s\",\n", name);
        printf("     \"blocks\":\"%d\",\n", size);
        printf("     \"bytes\":\"%d\",\n", (unsigned long)(size + 1) * 512);

        char tmp_name[64];
        char tmp_type_text[10];

        //Add type (and multifile) prefix to filename
        if (Efe[j][22] != 0)
        {
          strcpy(type_text, EpsTypes[type]);
          type_text[5] = '\0';
          sprintf(tmp_type_text, "%s%2d", type_text, Efe[j][22]);
          sprintf(tmp_name, "[%s] %s", tmp_type_text, dosname);
        }
        else
        {
          sprintf(tmp_name, "[%s] %s", EpsTypes[type], dosname);
        }
        sprintf(dosname, "%s", tmp_name);

        //Add index prefix to filename
        sprintf(tmp_name, "[%02d]%s", j, dosname);
        sprintf(dosname, "%s", tmp_name);

        printf("     \"filename\":\"%s\"\n", dosname);
        printf("   }");

        first_item = 0;
      }
      else
      {
        // Computer readable
        printf("%d,%s,%d,%s,%d,%d,%s,%ld\n", j, EpsTypes[type], real_type, name, Efe[j][22], size, dosname, (unsigned long)(size + 1) * 512);
      }

      // This line is for debuggin. Uncomment it if needed
      //printf(" %02d | %s(%d)  | %-12s | %4d | start=%d(%x) , cont=%d(%x) |\n",j,EpsTypes[type],real_type,name ,size, start,start,cont,cont);
    }
    //else if (printmode == COMPUTER_READABLE) {
    // If empty dir entry in this index, print "null" line for easier coputer parsing...
    //   printf("%d,%s,%d,%s,%d,%d,%s,%ld\n",j, "(empty)",0 ,"", 0,0,"",0);
    //}
  }

  if (printmode == JSON) {
    printf("]\n}\n");
  }

  if (printmode == HUMAN_READABLE)
  {
    printf("------+----------+--------------+---------+---------------------------------+\n");
    printf(" Used:  %23d Blocks    |  Used: %16ld Bytes   |\n", used_blks, (unsigned long)(used_blks)*512);
    printf(" Free:  %23d Blocks    |  Free: %16ld Bytes   |\n", free_blks, (unsigned long)(free_blks)*512);
    printf("----------------------------------------------------------------------------+\n");
    //printf(" Total: %23d Blocks    |  Total:%16ld Bytes   |\n", free_blks+used_blks, (unsigned long) (free_blks+used_blks)*512);
    //printf("----------------------------------------------------------------------------+\n\n");
  }
}

/////////////////////////////////////
// PrintBankInfo
// =============
// - Prints the structure of bank including
//   instruments and songs.
int PrintBankInfo(char filename[FILENAME_MAX], int printmode)
{
  int mode, count;
  int instValid[8];
  int effectFound = 0;
  int fd;
  int i, j, k;
  unsigned char data[1024];

  if (printmode == COMPUTER_READABLE)
  {
    // COMPUTER READABLE

    //Open & Read
    if ((fd = open(filename, O_RDONLY | O_BINARY, 0)) <= 0)
    {
      printf("1\n");
      return (1);
    }
    if ((count = read(fd, data, 0x222)) < 0x222)
    {
      printf("1\n");
      return (1);
    }

    // Get Bank type
    if ((data[0x32] != 4) && (data[0x32] != 23) && (data[0x32] != 30))
    {
      printf("2\n");
      return (1);
    }

    // OK
    printf("0\n");

    // Bank Type
    mode = data[0x32];
    printf("%d\n", mode);

    // Check Num of Blocks
    if (data[0x35] > 3)
    {
      effectFound = 1;
    }

    // Bank Name
    for (i = 0x208; i < 0x220; i = i + 2)
    {
      printf("%c", data[i]);
    }
    //Mask
    printf("\n%d\n", data[0x220]);
    instValid[7] = (data[0x220] >> 7) & 0x01;
    instValid[6] = (data[0x220] >> 6) & 0x01;
    instValid[5] = (data[0x220] >> 5) & 0x01;
    instValid[4] = (data[0x220] >> 4) & 0x01;
    instValid[3] = (data[0x220] >> 3) & 0x01;
    instValid[2] = (data[0x220] >> 2) & 0x01;
    instValid[1] = (data[0x220] >> 1) & 0x01;
    instValid[0] = data[0x220] & 0x01;

    for (j = 0; j < 9; j++)
    {

      //eps
      if (mode == MODE_EPS)
      {
        read(fd, data, 16);
      }
      else if (mode == MODE_E16)
      {
        read(fd, data, 16);
      }
      else
      {
        read(fd, data, 28);
      }

      // Valid instrument?
      if (j < 8)
      {

        if (!instValid[j])
        {
          printf("0\n");
          continue;
        }
      }
      else
      {
        // Valid song ?
        if (data[4] == 0)
        {
          printf("0\n");
          continue;
        }
      }

      // Inst/Song info
      if (data[0] > 0x7f)
      {
        //Song?
        if (j == 8)
        {
          printf("0\n");
        }
        else
        {
          // "Copy of" + Inst num
          printf("2,%d\n", (data[0] & 0x0f) + 1);
        }
      }
      else
      {

        //"Valid" + Path Depth
        printf("1,%d,", data[0]);
        // Device
        printf("%d,", data[2]);
        //Media Name (if any)
        if (mode == MODE_EPS)
        {
          printf("<NONE>,");
        }
        else
        {
          printf("%c%c%c%c%c%c%c,",
                 data[3],
                 data[5],
                 data[7],
                 data[9],
                 data[11],
                 data[13],
                 data[15]);
        }

        if (data[0] > 0)
        {
          for (k = 0; k < data[0]; k++)
          {
            printf("/%d", data[4 + 2 * k]);
          }
          printf(",%d\n", data[4 + 2 * k]);
        }
        else
        {
          printf("/,%d\n", data[4]);
        }
      }
    }
    if (effectFound)
    {
      lseek(fd, 0x800, 0);
      read(fd, data, 64);
      printf("1,%c%c%c%c%c%c%c%c%c%c%c%c\n",
             data[10],
             data[12],
             data[14],
             data[16],
             data[18],
             data[20],
             data[22],
             data[24],
             data[26],
             data[28],
             data[30],
             data[32]);
    }
    else
    {
      printf("0\n");
    }
  }
  else
  {
    // HUMAN READABLE

    if ((fd = open(filename, O_RDONLY | O_BINARY, 0)) <= 0)
    {
      printf("ERROR:File not found!\n");
      return (1);
    }

    if ((count = read(fd, data, 0x222)) < 0x222)
    {
      printf("ERROR:File read error!\n");
      return (1);
    }

    // Get Bank type
    switch (data[0x32])
    {
    case 4:
      mode = MODE_EPS;
      printf("Bank type:EPS\n");
      break;
    case 0x17:
      mode = MODE_E16;
      printf("Bank type:EPS16+\n");
      break;
    case 0x1e:
      mode = MODE_ASR;
      printf("Bank type:ASR\n");
      break;
    default:
      printf("ERROR: Not an Ensoniq Bank file!\n");
      exit(1);
    }

    // Check Num of Blocks
    if (data[0x35] > 3)
    {
      effectFound = 1;
    }

    printf("Bank Name:");
    for (i = 0x208; i < 0x220; i = i + 2)
    {
      printf("%c", data[i]);
    }
    printf("\n");
    printf("Inst Mask:0x%x (%d%d%d%d%d%d%d%d)\n", data[0x220],
           (data[0x220] >> 7) & 0x01,
           (data[0x220] >> 6) & 0x01,
           (data[0x220] >> 5) & 0x01,
           (data[0x220] >> 4) & 0x01,
           (data[0x220] >> 3) & 0x01,
           (data[0x220] >> 2) & 0x01,
           (data[0x220] >> 1) & 0x01,
           data[0x220] & 0x01);

    for (j = 0; j < 9; j++)
    {

      if (j == 8)
      {
        printf("\nSONG\n");
      }
      else
      {
        printf("\nINST %d\n", j + 1);
      }
      //eps
      if (mode == MODE_EPS)
      {
        read(fd, data, 16);
      }
      else if (mode == MODE_E16)
      {
        read(fd, data, 16);
      }
      else
      {
        read(fd, data, 28);
      }

      if (data[0] >= 0x80)
      {
        if (j == 8)
        {
          printf("<NONE>\n");
        }
        else
        {
          printf("Copy Of Inst: %d\n", (data[0] & 0x0f) + 1);
        }
      }
      else if (data[0] == 0x7f)
      {
        printf("Deleted\n");
      }
      else
      {
        printf("Path Depth: %d (0x%x)\n", data[0], data[0]);
        printf("0xFF: 0x%x\n", data[1]);
        printf("Media (0=floppy, SCSI+1): 0x%x\n", data[2]);
        printf("Disk Label: %c%c%c%c%c%c%c\n",
               data[3],
               data[5],
               data[7],
               data[9],
               data[11],
               data[13],
               data[15]);

        if (data[0] > 0)
        {
          printf("Path:");
          for (k = 0; k < data[0]; k++)
          {
            printf("/%d", data[4 + 2 * k]);
          }
          printf("\nInst Idx:%d\n", data[4 + 2 * k]);
        }
        else
        {
          printf("Inst Idx:%d\n", data[4]);
        }
      }
    }
    if (effectFound)
    {
      lseek(fd, 0x800, 0);
      read(fd, data, 64);
      printf("\nEFFECT: %c%c%c%c%c%c%c%c%c%c%c%c\n\n",
             data[10],
             data[12],
             data[14],
             data[16],
             data[18],
             data[20],
             data[22],
             data[24],
             data[26],
             data[28],
             data[30],
             data[32]);
    }
  }
  return (0);
}

/////////////////////////////////
// Get FAT entry - use fat-table
unsigned int GetFatEntry(char media_type, unsigned char *DiskFAT, int file, unsigned int block)
{
  unsigned int fatsect, fatpos, tmp;
  unsigned char FatEntry[3];

  fatsect = (int)block / 170;
  fatpos = block % 170;

  if (media_type == 'f')
  {
    // FILE ACCESS
#ifdef __CYGWIN__
    // To get /dev/scd work...
    {
      unsigned char tmp_buff[512];
      ReadBlocks(media_type, (FD_HANDLE)0, file, (FAT_START_BLOCK + fatsect), 1, tmp_buff);
      return ((tmp_buff[fatpos * 3] << 16) + (tmp_buff[fatpos * 3 + 1] << 8) + tmp_buff[fatpos * 3 + 2]);
    }
#else
    if (lseek(file, (FAT_START_BLOCK + fatsect) * BLOCK_SIZE + fatpos * 3, SEEK_SET) == -1)
    {
      printf("ERROR in seek\n");
    }
    if (read(file, FatEntry, 3) == 0)
    {
      printf("ERROR in read\n");
    }
    return ((FatEntry[0] << 16) + (FatEntry[1] << 8) + FatEntry[2]);
#endif
  }
  else
  {
    // DISK ACCESS - uses FAT-table
    tmp = (fatsect * BLOCK_SIZE) + fatpos * 3;
    return ((DiskFAT[tmp] << 16) + (DiskFAT[tmp + 1] << 8) + DiskFAT[tmp + 2]);
  }
}

/////////////////////
// PutFatEntry
int PutFatEntry(char media_type, unsigned char *DiskFAT, int file,
                unsigned int block, unsigned int fatval)
{
  unsigned int fatsect, fatpos, tmp;
  unsigned char FatEntry[3];

  fatsect = (int)block / 170;
  fatpos = block % 170;

  FatEntry[2] = fatval & 0x000000FF;
  FatEntry[1] = (fatval >> 8) & 0x000000FF;
  FatEntry[0] = (fatval >> 16) & 0x000000FF;

  if (media_type == 'f')
  {
    // file access
    lseek(file, (FAT_START_BLOCK + fatsect) * BLOCK_SIZE + fatpos * 3, SEEK_SET);
    write(file, FatEntry, 3);
    return (OK);
  }
  else
  {
    // disk access - uses FAT-table
    tmp = (fatsect * BLOCK_SIZE) + fatpos * 3;
    memcpy(DiskFAT + tmp, FatEntry, 3);
    return (OK);
  }
}

//////////////////////
// Convert MacFormat
// ===================
// - Convert really _stupid_ 'Mac'-format (ie. every '0x0a' is replaced by '0x0d0a')..
//   Why this even exists !?!?

int ConvertMacFormat(int *in, char *in_file)
{
  int out;
  //int c;
  unsigned int Hdr, i, prev;
  char temp_file[FILENAME_MAX];
  unsigned char *mem_pointer;
  struct stat stat_buf;

  lseek(*in, (long)0, SEEK_SET);
  read(*in, &Hdr, 4);
  lseek(*in, (long)0, SEEK_SET);

  Hdr = Hdr & 0x00FFFFFF;
  if (Hdr == 0x0a0d0d)
  {

    mkstemp(temp_file);
    if ((out = open(temp_file, O_RDWR | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", temp_file));
    }
    unlink(temp_file);

    if (stat(in_file, &stat_buf) != 0)
    {
      EEXIT((stderr, "ERROR: Can't get the filesize!!\n"));
    }

    mem_pointer = malloc(stat_buf.st_size);
    if (mem_pointer == NULL)
      EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

    read(*in, mem_pointer, stat_buf.st_size);

    // Find '0x0a' and copy fragments to file without proceeding '0x0d'

    prev = 0;
    for (i = 0; i < stat_buf.st_size; i++)
    {
      if (mem_pointer[i] == 0x0a)
      {
        write(out, mem_pointer + prev, i - prev - 1);
        prev = i;
      }
    }

    // if '0x0a' is last byte it has to write out separately..
    // Otherwise write whats left...
    if ((i - 1) != prev)
    {
      write(out, mem_pointer + prev, i - prev);
    }
    else
    {
      write(out, mem_pointer + prev, 1);
    }

    free(mem_pointer);

    // Close orginal and give pointer to converted file
    close(*in);
    *in = out;

    return (OK);
  }
  else
  {
    return (ERR);
  }
}

//////////////////////
// ConvertToImageFile
int ConvertToImage(char in_file[FILENAME_MAX], char out_file[FILENAME_MAX])
{
  int in, out;
  struct stat stat_buf;

  unsigned int i, j, skip_start, skip_size, num_of_tags, num_of_blks, img_length, img_offset;
  unsigned char buffer[BLOCK_SIZE], Data[BLOCK_SIZE], *mem_pointer, *SkipTable;
  char bits, *p;

  mem_pointer = NULL;

  if ((in = open(in_file, O_RDONLY | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", in_file));
  }
  if ((out = open(out_file, O_RDWR | O_CREAT | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", out_file));
  }

  if ((p = (char *)rindex(in_file, '.')) != NULL)
  {
    p++;
    //printf("Extension: %s\n",p);

    if (strcasecmp(p, "gkh") == 0)
    {
      //printf("GKH-file found\n");

      //// GKH ////
      if (stat(in_file, &stat_buf) != 0)
      {
        EEXIT((stderr, "ERROR: Can't get the filesize!!\n"));
      }

      lseek(in, (long)0, SEEK_SET);
      read(in, Data, 8);

      if (Data[4] != 'I')
      {
        EEXIT((stderr, "ERROR: GKH file in Motorola format is not supported!!\n"));
      }

      //num_of_tags = (Data[6] & 0x000000ff) + (Data[7] & 0x000000ff);
      num_of_tags = (unsigned int)(*((unsigned int *)(Data + 6)) & 0x0000ffff);

      //printf("num_of_tags= %d\n",num_of_tags);

      for (i = 0; i < num_of_tags; i++)
      {
        read(in, Data, 10);

        // DISKINFO-tag
        if (*Data == 0x0a)
        {
          num_of_blks = (unsigned int)((*((unsigned int *)(Data + 2)) & 0x0000ffff) *
                                       (*((unsigned int *)(Data + 4)) & 0x0000ffff) *
                                       (*((unsigned int *)(Data + 6)) & 0x0000ffff));
          //printf("num_of_blks = %d\n", num_of_blks);
        }

        // DISKINFO-tag
        if (*Data == 0x0b)
        {
          img_length = (unsigned int)*((unsigned int *)(Data + 2));
          img_offset = (unsigned int)*((unsigned int *)(Data + 6));
          //printf("img_length = %d\n", img_length);
          //printf("img_offset = %d\n", img_offset);
        }
      }

      // Skip the 58-byte header
      lseek(in, (long)img_offset, SEEK_SET);

      // Do copy...
      for (i = 0; i < num_of_blks; i++)
      {
        read(in, Data, BLOCK_SIZE);
        write(out, Data, BLOCK_SIZE);
      }

      // Get INFO after image
      // -- uncomment if INFO needed
      /*
	if(!feof(in)) {
	printf("\nInfo included in GKH file: \n");
	bits=fgetc(in); 
	while (feof(in) == 0) {
	printf("%c",bits);
	bits=fgetc(in);  
	}
	printf("\n");
	}
      */
    }
    else if ((strcasecmp(p, "ede") == 0) || (strcasecmp(p, "eda") == 0))
    {

      //// EDE / EDA /////

      if (strcasecmp(p, "ede") == 0)
      {
        skip_start = EDE_SKIP_START;
        skip_size = EDE_SKIP_SIZE;
      }
      else
      {
        skip_start = EDA_SKIP_START;
        skip_size = EDA_SKIP_SIZE;
      }

      // Write 'empty'-buffer
      for (i = 0; i < BLOCK_SIZE; i = i + 2)
      {
        buffer[i] = 0x6D;
        buffer[i + 1] = 0xB6;
      }

      // Check and convert if 'Mac'-format :-P is found
      if (ConvertMacFormat(&in, in_file) == OK)
      {
        printf("Warning: Macintosh generated EDx file found!\n");
      }

      mem_pointer = malloc(BLOCK_SIZE);
      if (mem_pointer == NULL)
        EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
      SkipTable = mem_pointer + skip_start;

      // Read Ede/Eda skip Table
      lseek(in, (long)0, SEEK_SET);
      read(in, mem_pointer, BLOCK_SIZE);

      lseek(in, BLOCK_SIZE, SEEK_SET);

      for (i = 0; i < skip_size; i++)
      {
        bits = SkipTable[i];

        for (j = 0; j < 8; j++)
        {
          if (bits < 0)
          {
            write(out, buffer, BLOCK_SIZE);
          }
          else
          {
            read(in, Data, BLOCK_SIZE);
            write(out, Data, BLOCK_SIZE);
          }
          bits = bits << 1;
        } // for j

      } // for i
    }
    else
    {
      // other extension - lets guess its raw-image
      //printf("No conversion!\n");
      free(mem_pointer);
      close(in);
      close(out);
      return (ERR);
    }
  }
  else
  {
    // Can't get file type
    //printf("No conversion!\n");
    free(mem_pointer);
    close(in);
    close(out);
    return (ERR);
  }

  //printf("conversion done!\n");
  free(mem_pointer);
  close(in);
  close(out);
  return (OK);
}

//////////////////////
// ConvertFromImage
int ConvertFromImage(char in_file[FILENAME_MAX], char out_file[FILENAME_MAX], char type)
{
  int in, out;
  unsigned char bits, *SkipTable, *mem_pointer, edx_id;
  char edx_label[12];
  unsigned int skip_size, skip_start, block, i, j;
  unsigned char Data[BLOCK_SIZE];

  edx_id = 0;
  skip_start = 0;
  skip_size = 0;

  switch (type)
  {
  case EDE_TYPE:
    skip_start = EDE_SKIP_START;
    skip_size = EDE_SKIP_SIZE;
    edx_id = EDE_ID;
    strcpy(edx_label, EDE_LABEL);
    break;

  case EDA_TYPE:

    skip_start = EDA_SKIP_START;
    skip_size = EDA_SKIP_SIZE;
    edx_id = EDA_ID;
    strcpy(edx_label, EDA_LABEL);
    break;

  case GKH_TYPE:
    EEXIT((stderr, "ERROR: GKH-output not supported yet!\n"));

  default:
    EEXIT((stderr, "ERROR: Unsupported conversion!\n"));
  }

  //printf("RAW -> EDx conversion!!\n");

  if ((in = open(in_file, O_RDONLY | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.", in_file));
  }
  if ((out = open(out_file, O_RDWR | O_CREAT | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.", out_file));
  }

  mem_pointer = malloc(BLOCK_SIZE);
  if (mem_pointer == NULL)
    EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

  SkipTable = mem_pointer + skip_start;

  // Construct EDx header

  // CR & LF
  mem_pointer[0] = 0x0D;
  mem_pointer[1] = 0x0A;

  // Spaces trough 0x02 - 0x4D
  for (i = 0x02; i < 0x4E; i++)
  {
    mem_pointer[i] = 0x20;
  }

  // Write EDE label
  strncpy((char *)(mem_pointer + 2), edx_label, 11);

  // CR & LF
  mem_pointer[0x4E] = 0x0D;
  mem_pointer[0x4F] = 0x0A;

  // Spaces trough 0x50 - skip_start
  for (i = 0x50; i < skip_start; i++)
  {
    mem_pointer[i] = 0x20;
  }

  // CR & LF
  mem_pointer[skip_start - 3] = 0x0D;
  mem_pointer[skip_start - 2] = 0x0A;
  mem_pointer[skip_start - 1] = 0x1A;

  // Disktype ID
  mem_pointer[BLOCK_SIZE - 1] = edx_id;
  // Write Header
  write(out, mem_pointer, BLOCK_SIZE);

  // Generate SkipTable
  block = 0;
  for (i = 0; i < skip_size; i++)
  {
    bits = 0;
    for (j = 0; j < 8; j++)
    {
      bits = bits << 1;
      if (GetFatEntry('f', NULL, in, block) == 0)
      {
        bits = bits | 0x01;
      }
      else
      {
        lseek(in, block * BLOCK_SIZE, SEEK_SET);
        read(in, Data, BLOCK_SIZE);
        write(out, Data, BLOCK_SIZE);
      }
      block++;
    }
    SkipTable[i] = bits;
  }
  // Write last EOF-marker
  //fputc(0x1A,out);
  Data[0] = 0x1A;
  write(out, Data, 1);

  lseek(out, skip_start, SEEK_SET);
  write(out, SkipTable, skip_size);
  return (OK);
}

///////////////
// IsEFE
int IsEFE(char in_file[FILENAME_MAX])
{
  char *p;

  if ((p = rindex(in_file, '.')) != NULL)
  {

    // Extension found!
    p++;
    //printf("Extension: %s\n",p);

    if ((strcasecmp(p, "efe") == 0) || (strcasecmp(p, "efa") == 0))
    {
      //printf("EFE-file found\n");
      return (OK);
    }
  }
  return (ERR);
}

///////////////////
// GetImageType
int GetImageType(char in_file[FILENAME_MAX], char *image_type)
{
  char *p;
  struct stat stat_buf;

  if ((p = rindex(in_file, '.')) != NULL)
  {

    // Extension found!
    p++;
    //printf("Extension: %s\n",p);

    if (strcasecmp(p, "gkh") == 0)
    {
      //printf("GKH-file found\n");
      *image_type = GKH_TYPE;
      return (OK);
    }
    else if (strcasecmp(p, "ede") == 0)
    {
      //printf("EDE-file found\n");
      *image_type = EDE_TYPE;
      return (OK);
    }
    else if (strcasecmp(p, "eda") == 0)
    {
      //printf("EDA-file found\n");
      *image_type = EDA_TYPE;
      return (OK);
    }
  }

  // No or unknown extension - check if known raw-image

  // Uncomment for header-check when raw-image expected
  /*
    if((in=fopen(in_file, "rb"))== NULL) {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.", in_file));

    }
  
    fseek(in, 512,0);
    fread(&Hdr,sizeof(unsigned long), 1,in);
  
    printf("hdr: %lx \n", Hdr);
  */

  if (stat(in_file, &stat_buf) != 0)
  {
    //fprintf(stderr,"\n(line: %d) ERROR: Can't get the filesize!!\n", __LINE__);
    return (ERR);
  }

  //printf("filesize: %ld\n",stat_buf.st_size);

  if (stat_buf.st_size == EPS_IMAGE_SIZE)
  {
    //printf("EPS-file found\n");
    *image_type = EPS_TYPE;
    return (OK);
  }
  else if (stat_buf.st_size == ASR_IMAGE_SIZE)
  {
    //printf("ASR-file found\n");
    *image_type = ASR_TYPE;
    return (OK);
  }
  else
  {
    //fprintf(stderr,"\nWarning! Not a known file format. Might be an HD-image or SCSI-device!  \n\n");
    *image_type = OTHER_TYPE;
    return (OK);
  }
}

////////////////////
// GetTHS
void GetTHS(unsigned int block, unsigned int num_of_sect,
            unsigned int *track, unsigned int *head, unsigned int *sector)
{
  *track = block / (2 * num_of_sect);
  *head = (block - (*track * 2 * num_of_sect)) / num_of_sect;
  *sector = block - (*track * 2 * num_of_sect) - (*head * num_of_sect);
}

////////////////////
// OpenFloppy
FD_HANDLE OpenFloppy(int drive)
{
  FD_HANDLE fd;

#ifdef __CYGWIN__
  DWORD dwRet;
  char szDevice[32];
  DWORD dwVersion = 0;

  HANDLE h = CreateFile("\\\\.\\fdrawcmd", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

  if (h != INVALID_HANDLE_VALUE)
  {
    DeviceIoControl(h, IOCTL_FDRAWCMD_GET_VERSION, NULL, 0, &dwVersion, sizeof(dwVersion), &dwRet, NULL);
    CloseHandle(h);
  }
  else
  {
    printf("fdrawcmd.sys is not installed, see: http://simonowen.com/fdrawcmd/\n");
    exit(ERR);
  }

  if (HIWORD(dwVersion) != HIWORD(FDRAWCMD_VERSION))
  {
    printf("The installed fdrawcmd.sys is not compatible with this utility.\n");
    exit(ERR);
  }

  wsprintf(szDevice, "\\\\.\\fdraw%u", drive);

  if ((fd = CreateFile(szDevice, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
  {
    printf("OpenFloppy - CreateFile: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

  if (DeviceIoControl(fd, IOCTL_FD_RESET, NULL, 0, NULL, 0, &dwRet, NULL) == 0)
  {
    printf("OpenFloppy - RESET: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

#else // Linux

  fd = open("/dev/fd0", O_ACCMODE | O_NDELAY);
  //fd = open( "/dev/fd0", O_ACCMODE | O_SYNC);
  if (fd < 0)
  {
    perror("open floppy");
    exit(ERR);
  }
#endif
  return fd;
}

////////////////////
// CloseFloppy
void CloseFloppy(FD_HANDLE fd)
{
#ifdef __CYGWIN__
  CloseHandle(fd);
#else // Linux
  close(fd);
#endif
}

//////////////////////////////////////
// Set FD geometry. 'a'=ASR, 'e'=EPS
void FD_SetGeometry(FD_HANDLE fd, char disk_type)
{
#ifdef __CYGWIN__
  DWORD dwRet;
  BYTE DataRate;

  if (disk_type == 'a')
  {
    // ASR (hd-disk)
    DataRate = 0;
  }
  else
  {
    // EPS (dd-disk)
    DataRate = 2;
  }

  if (DeviceIoControl(fd, IOCTL_FD_SET_DATA_RATE, &DataRate, sizeof(DataRate), NULL, 0, &dwRet, NULL) == 0)
  {
    printf("FD_SetGeometry - SET_DATA_RATE: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

#else // Linux

  struct floppy_struct g;

  if (disk_type == 'a')
  {
    // Geometry of ASR-Disk
    g.size = 3200;
    g.sect = 20;
  }
  else
  {
    // Geometry of EPS-Disk
    g.size = 1600;
    g.sect = 10;
  }

  // Common parameters
  g.head = 2;
  g.track = 80;
  g.stretch = 0;

  // Set Disk geometry
  if (ioctl(fd, FDSETPRM, &g) < 0)
  {
    perror("geometry ioctl");
    exit(ERR);
  }
#endif
}

// Calibrate FD
void FD_Calibrate(int fd)
{

#ifdef __CYGWIN__
#else // Linux

  struct floppy_raw_cmd raw_cmd;

  // Calibrate
  raw_cmd.flags = 8;
  raw_cmd.cmd_count = 2;
  raw_cmd.cmd[0] = 7;
  raw_cmd.cmd[1] = 0;

  if (ioctl(fd, FDRAWCMD, &raw_cmd) < 0)
  {
    perror("raw cmd");
    exit(ERR);
  }
#endif
}

///////////////////////
// Seek the FD track
void FD_Seek(FD_HANDLE fd, int track)
{

#ifdef __CYGWIN__
  DWORD dwRet;
  FD_SEEK_PARAMS sp;

  // details of seek location
  sp.cyl = track;
  sp.head = 0;

  // seek
  if (DeviceIoControl(fd, IOCTL_FDCMD_SEEK, &sp, sizeof(sp), NULL, 0, &dwRet, NULL) == 0)
  {
    printf("FD_Seek - SEEK: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

#else // Linux

  struct floppy_raw_cmd raw_cmd;

  // SEEK
  raw_cmd.flags = 8;
  raw_cmd.cmd_count = 3;
  raw_cmd.cmd[0] = 15;
  raw_cmd.cmd[1] = 0;
  raw_cmd.cmd[2] = track;

  if (ioctl(fd, FDRAWCMD, &raw_cmd) < 0)
  {
    perror("raw cmd");
    exit(ERR);
  }
#endif
}

////////////////////////////
// FD_Raw Read/Write Track
// (read: rw=1, write: rw=2)
int FD_RawRW_DiskTrack(FD_HANDLE fd, char disk_type, int track, int head, char *buffer, int rw)
{
#ifdef __CYGWIN__

  unsigned int datalen;
  DWORD dwRet;
  FD_READ_WRITE_PARAMS rwp;
  FD_SEEK_PARAMS sp;

  rwp.flags = FD_OPTION_MFM;
  rwp.phead = head;
  rwp.cyl = track;
  rwp.head = head;
  rwp.sector = 0;
  rwp.size = 2;
  rwp.gap = 0x0a;
  rwp.datalen = 0xff;

  // Seek
  sp.cyl = track;
  sp.head = head;

  if (disk_type == 'a')
  {
    // ASR (hd-disk)
    rwp.eot = 20;
    datalen = 20 * 512;
    FD_SetGeometry(fd, 'a');
  }
  else
  {
    // EPS (dd-disk)
    rwp.eot = 10;
    datalen = 10 * 512;
    FD_SetGeometry(fd, 'e');
  }

  if (DeviceIoControl(fd, IOCTL_FDCMD_SEEK, &sp, sizeof(sp), NULL, track, &dwRet, NULL) == 0)
  {
    printf("FD_RawRW_DiskTrack - SEEK: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

  if (rw == 1)
  {
    // READ TRACK
    if (DeviceIoControl(fd, IOCTL_FDCMD_READ_DATA, &rwp, sizeof(rwp), buffer, datalen, &dwRet, NULL) == 0)
    {
      //printf("FD_RawRW_DiskTrack - READ_DATA: (%ld) %s\n",GetLastError (), LastError());
      //exit(ERR);
      return (ERR);
    }
  }
  else
  {
    // WRITE TRACK
    if (DeviceIoControl(fd, IOCTL_FDCMD_WRITE_DATA, &rwp, sizeof(rwp), buffer, datalen, &dwRet, NULL) == 0)
    {
      printf("FD_RawRW_DiskTrack - WRITE_DATA: (%ld) %s\n", GetLastError(), LastError());
      exit(ERR);
    }
  }

#else // Linux

  struct floppy_raw_cmd raw_cmd;

  // RAW-COMMADS

  if (disk_type == 'a')
  {
    //ASR
    raw_cmd.length = 512 * 20;
    raw_cmd.rate = 0;
    raw_cmd.flags = 136 + rw; // read=137, write=138
    raw_cmd.cmd[6] = 20;      // Sectors (20)
  }
  else
  {
    //EPS
    raw_cmd.length = 512 * 10;
    raw_cmd.rate = 2;
    raw_cmd.flags = 136 + rw; // read=137, write=138
    raw_cmd.cmd[6] = 10;      // Sectors (10)
  }

  // Common parameters
  raw_cmd.data = buffer;
  raw_cmd.track = track;
  raw_cmd.cmd_count = 9;

  raw_cmd.cmd[0] = 230 - 33 * (rw - 1); // read=230, write=197
  raw_cmd.cmd[1] = head * 4;
  raw_cmd.cmd[2] = track;
  raw_cmd.cmd[3] = head;
  raw_cmd.cmd[4] = 0;
  raw_cmd.cmd[5] = 2;
  raw_cmd.cmd[7] = 27;
  raw_cmd.cmd[8] = 255;

  if (ioctl(fd, FDRAWCMD, &raw_cmd) < 0)
  {
    //perror("Disk access");
    EEXIT((stderr, "\nPlease insert Ensoniq disk in disk drive or define an image file!\n\n"));
  }
  if (raw_cmd.reply[0] >= 40)
  {
    return (ERR);
  }
  return (OK);
#endif
}

////////////////////////////
// FD_Raw Read/Write Blocks
// (read: rw=1, write: rw=2)
int FD_RawRW_DiskSectors(FD_HANDLE fd, char disk_type, int track, int head,
                         int sector, int num_of_sectors, char *buffer, int rw)
{

#ifdef __CYGWIN__

  unsigned int datalen;
  DWORD dwRet;
  FD_READ_WRITE_PARAMS rwp;
  FD_SEEK_PARAMS sp;

  rwp.flags = FD_OPTION_MFM;
  rwp.phead = head;
  rwp.cyl = track;
  rwp.head = head;
  rwp.sector = sector;
  rwp.size = 2;
  rwp.gap = 0x0a;
  rwp.datalen = 0xff;

  // Seek
  sp.cyl = track;
  sp.head = head;

  rwp.eot = sector + num_of_sectors;
  datalen = num_of_sectors * 512;

  if (DeviceIoControl(fd, IOCTL_FDCMD_SEEK, &sp, sizeof(sp), NULL, track, &dwRet, NULL) == 0)
  {
    printf("FD_RawRW_DiskTrack - SEEK: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

  if (rw == 1)
  {
    // READ BLOCKS
    if (DeviceIoControl(fd, IOCTL_FDCMD_READ_DATA, &rwp, sizeof(rwp), buffer, datalen, &dwRet, NULL) == 0)
    {
      printf("FD_RawRW_DiskSectors - READ_DATA: (%ld) %s\n", GetLastError(), LastError());
      exit(ERR);
    }
  }
  else
  {
    // WRITE BLOCKS
    if (DeviceIoControl(fd, IOCTL_FDCMD_WRITE_DATA, &rwp, sizeof(rwp), buffer, datalen, &dwRet, NULL) == 0)
    {
      printf("FD_RawRW_DiskSectors - WRITE_DATA: (%ld) %s\n", GetLastError(), LastError());
      exit(ERR);
    }
  }

#else // Linux

  struct floppy_raw_cmd raw_cmd;

  // RAW-COMMADS

  if (disk_type == 'a')
  {
    //ASR
    raw_cmd.length = 512 * num_of_sectors;
    raw_cmd.rate = 0;
    raw_cmd.flags = 136 + rw;
    raw_cmd.cmd[6] = 20; // Sectors (20)
  }
  else
  {
    //EPS
    raw_cmd.length = 512 * num_of_sectors;
    raw_cmd.rate = 2;
    raw_cmd.flags = 136 + rw;
    raw_cmd.cmd[6] = 10; // Sectors (10)
  }

  // Common parameters
  raw_cmd.data = buffer;
  raw_cmd.track = track;
  raw_cmd.cmd_count = 9;

  raw_cmd.cmd[0] = 230 - 33 * (rw - 1); // read=230, write=197
  raw_cmd.cmd[1] = head * 4;
  raw_cmd.cmd[2] = track;
  raw_cmd.cmd[3] = head;
  raw_cmd.cmd[4] = sector;
  raw_cmd.cmd[5] = 2;
  raw_cmd.cmd[7] = 27;
  raw_cmd.cmd[8] = 255;

  if (ioctl(fd, FDRAWCMD, &raw_cmd) < 0)
  {
    //perror("Disk access");
    EEXIT((stderr, "\nPlease insert Ensoniq disk in disk drive or define image file!\n\n"));
  }
  if (raw_cmd.reply[0] >= 40)
  {
    return (ERR);
  }
  return (OK);
#endif
}

#ifdef __CYGWIN__
#else // Linux
// Datatypes for Format
typedef struct
{
  char track;
  char head;
  char sector;
  char szcode;
} FormatHeader;

typedef FormatHeader FormatHeaders[MAX_DISK_SECT];
#endif

////////////////////
// Format DiskTrack
int FD_Format_DiskTrack(FD_HANDLE fd, int track, int head, int nsect, int rate, int skew)
{

#ifdef __CYGWIN__

  DWORD dwRet;
  BYTE abFormat[sizeof(FD_FORMAT_PARAMS) + sizeof(FD_ID_HEADER) * 20];

  BYTE s;
  PFD_FORMAT_PARAMS pfp = (PFD_FORMAT_PARAMS)abFormat;
  PFD_ID_HEADER ph = pfp->Header;
  pfp->flags = FD_OPTION_MFM;
  pfp->phead = head;
  pfp->size = 2;
  pfp->sectors = nsect;
  pfp->gap = 0x20;
  pfp->fill = 0x00;

  for (s = 0; s < (pfp->sectors); s++, ph++)
  {
    ph->cyl = track;
    ph->head = head;
    ph->sector = (s + skew) % nsect;
    ph->size = pfp->size;
  }

  if (DeviceIoControl(fd, IOCTL_FDCMD_SEEK, &track, sizeof(track), NULL, track, &dwRet, NULL) == 0)
  {
    printf("FD_RawRW_DiskTrack - SEEK: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

  if (DeviceIoControl(fd, IOCTL_FDCMD_FORMAT_TRACK, pfp, (ULONG)((PBYTE)ph - abFormat), NULL, 0, &dwRet, NULL) == 0)
  {
    printf("FD_Format_DiskTrack - FORMAT_TRACK: (%ld) %s\n", GetLastError(), LastError());
    exit(ERR);
  }

#else // Linux

  FormatHeaders headers;
  struct floppy_raw_cmd raw_cmd;
  int i, tmp;

  for (i = 0; i < nsect; i++)
  {

    headers[i].track = track;               // track
    headers[i].head = head;                 // head
    headers[i].sector = (i + skew) % nsect; // sector (with skew ie. sector shift)
    headers[i].szcode = 2;                  // sizecode
  }

  raw_cmd.track = track;
  raw_cmd.data = headers;
  raw_cmd.length = 4 * nsect;
  raw_cmd.rate = rate;
  raw_cmd.flags = 138;
  raw_cmd.cmd_count = 6;
  raw_cmd.cmd[0] = 77;
  raw_cmd.cmd[1] = head * 4;
  raw_cmd.cmd[2] = 2;
  raw_cmd.cmd[3] = nsect;
  raw_cmd.cmd[4] = 40;
  raw_cmd.cmd[5] = 0;

  tmp = ioctl(fd, FDRAWCMD, &raw_cmd);
  if (tmp < 0)
  {
    perror("raw cmd");
    exit(ERR);
  }
  if (raw_cmd.reply[0] >= 40)
  {
    //printf("\nERROR at: head=%d, track=%d !!\n\n",head,track);
    printf("\n\nDisk is WRITE PROTECTED !!\n\n");
    return (ERR);
  }
  return (OK);
#endif
}

////////////////
// MakeID_Block
void MakeID_Block(unsigned char *buffer, char *label, unsigned int tracks,
                  unsigned int nsect, unsigned int total_blks)
{
  int i;

  for (i = 0; i < BLOCK_SIZE; i++)
  {
    buffer[i] = 0;
  }

  // Device type
  //buffer[0] = 0x00;

  // Rem. Media device Type
  buffer[1] = 0x80;

  // Std. vers. #
  buffer[2] = 0x01;

  // SCSI
  //buffer[3] = 0x00;

  // Nsect
  buffer[4] = 0x00;
  buffer[5] = (unsigned char)nsect;

  // Heads
  buffer[6] = 0x00;
  buffer[7] = 0x02;

  // Tracks
  buffer[8] = 0x00;
  buffer[9] = (unsigned char)tracks;

  // Bytes per Block - 512
  buffer[10] = 0x00;
  buffer[11] = 0x00;
  buffer[12] = 0x02;
  buffer[13] = 0x00;

  // Total Blocks

  buffer[14] = (total_blks >> 24) & 0x000000ff; // MSB
  buffer[15] = (total_blks >> 16) & 0x000000ff;
  buffer[16] = (total_blks >> 8) & 0x000000ff;
  buffer[17] = total_blks & 0x000000ff; // LSB

  // SCSI Medium Type
  buffer[18] = 0x1e;

  // SCSI Density Code
  buffer[19] = 0x02;

  // reserved (20-29)

  // Disk Label
  if (label != NULL)
  {

    for (i = 0; i < 7; i++)
    {
      buffer[31 + i] = label[i];
    }
    buffer[30] = 0xff;
  }

  // ID
  buffer[38] = 'I';
  buffer[39] = 'D';
}

/////////////////
// MakeOS_Block
void MakeOS_Block(unsigned char *buffer, unsigned int free_blks)
{
  int i;

  for (i = 0; i < BLOCK_SIZE; i++)
  {
    buffer[i] = 0;
  }

  // Free Blocks
  buffer[0] = (free_blks >> 24) & 0x000000ff; // MSB
  buffer[1] = (free_blks >> 16) & 0x000000ff;
  buffer[2] = (free_blks >> 8) & 0x000000ff;
  buffer[3] = free_blks & 0x000000ff; // LSB

  // ID
  buffer[28] = 'O';
  buffer[29] = 'S';
}

/////////////////
// MakeDR_Block
void MakeDR_Block(unsigned char *buffer, int write_id)
{
  int i;

  for (i = 0; i < BLOCK_SIZE; i++)
  {
    buffer[i] = 0;
  }

  if (write_id == 1)
  {
    buffer[510] = 'D';
    buffer[511] = 'R';
  }
}

//////////////////////
// Format Disk
void FD_Format_Disk(char disk_type, int nsect, int rate, char *disk_label)
{

  FD_HANDLE fd;
  int track, head, free_blks, fat_blks, i, j, skew, head_skew, track_skew;
  unsigned char buffer[BLOCK_SIZE * 40];
  char str[81], tmp;

  // Init the skew factory for the disk
  if (disk_type == 'e')
  {

    track_skew = nsect - 2;
    head_skew = nsect - 1;
    skew = -track_skew;
  }
  else
  {

    track_skew = nsect - 3;
    head_skew = nsect - 2;
    skew = -track_skew;
  }

  // Calc disk parameters
  fat_blks = nsect * 2 * 80 / 170 + 1;
  free_blks = nsect * 2 * 80 - (5 + fat_blks);

  //Open FD
  fd = OpenFloppy(0);

  FD_SetGeometry(fd, disk_type);

  printf("1       10        20        30        40        50        60        70        80\n");
  sprintf(str, "|---+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|");

  for (track = 0; track < 80; track++)
  {

    //Shift sectors (track skew)
    skew = skew + track_skew;

    for (head = 0; head < 2; head++)
    {

      if (head == 0)
      {
        tmp = str[track];

        str[track] = '\\';
      }
      else
      {
        // Shift sectors (head skew)
        skew = skew + head_skew;

        str[track] = '/';
      }
      printf("\r%s", str);
      fflush(stdout);

      if ((FD_Format_DiskTrack(fd, track, head, nsect, rate, skew)) == ERR)
      {
        exit(ERR);
      }
    } //for head
    if (tmp == '|')
      str[track] = tmp;
    else
      str[track] = '#';
  } // for track

  printf("\r%s", str);
  fflush(stdout);
  printf("\n\nWriting System Block..\n");

  // TODO: Get the FD status so we don't have to wait

  sleep(1);

  // Write System-Blocks

  for (i = 0; i < BLOCK_SIZE * 40; i++)
  {
    buffer[i] = 0;
  }

  // ID-Block
  MakeID_Block(buffer + BLOCK_SIZE, disk_label, 80, nsect, 2 * 80 * nsect);

  // OS-Block
  MakeOS_Block(buffer + BLOCK_SIZE * 2, free_blks);

  // Write Dir-ID (to second Dir-Block)
  buffer[BLOCK_SIZE * 4 + 510] = 'D';
  buffer[BLOCK_SIZE * 4 + 511] = 'R';

  // FAT-Blocks
  for (i = 0; i < fat_blks; i++)
  {
    if (i == 0)
    {
      for (j = 0; j < (fat_blks + 5); j++)
      {
        buffer[BLOCK_SIZE * 5 + 3 * j + 2] = 1;
      }
    }
    buffer[BLOCK_SIZE * (5 + i) + 510] = 'F';
    buffer[BLOCK_SIZE * (5 + i) + 511] = 'B';
  }

  if (FD_RawRW_DiskTrack(fd, disk_type, 0, 0, buffer, WRITE) == ERR)
  {
    EEXIT((stderr, "ERROR: Couldn't write SystemData to disk!!\n\n"));
  }

  if (FD_RawRW_DiskTrack(fd, disk_type, 0, 1, buffer + (nsect * BLOCK_SIZE), WRITE) == ERR)
  {
    EEXIT((stderr, "ERROR: Couldn't write SystemData to disk!!\n\n"));
  }

  printf("Operation completed succesfully!\n\n");

  CloseFloppy(fd);
}

///////////////////////////////////////////////////////////
// Check which type of disk is inserted (EPS=DD or ASR=HD)
int FD_GetDiskType(char *disk_type, int *nsect, int *trk_size)
{

  FD_HANDLE fd;
  char buffer[20 * 512];

  *disk_type = 'n';

  //Open FD
  fd = OpenFloppy(0);

  // EPS-Disk !? (DD)
  FD_SetGeometry(fd, 'e');
  if ((FD_RawRW_DiskTrack(fd, 'e', 0, 0, buffer, READ)) == ERR)
  {

    //.. OR ASR-Disk !? (HD)
    FD_SetGeometry(fd, 'a');
    if ((FD_RawRW_DiskTrack(fd, 'a', 0, 0, buffer, READ)) == ERR)
    {

      // NEITHER :(
      return (ERR);
    }
    else
    { // ASR
      //printf("\nASR-Disk detected..\n");
      *disk_type = 'a';
      *nsect = 20;
      *trk_size = 20 * 512;
    }
  }
  else
  { // EPS
    //printf("\nEPS-Disk detected..\n");
    *disk_type = 'e';
    *nsect = 10;
    *trk_size = 10 * 512;
  }

  CloseFloppy(fd);

  return (OK);
}

//////////////////////////
// Read/Write the Disk
void FD_RW_Disk(char in_file[FILENAME_MAX], int rw_disk)
{
  int file;

  FD_HANDLE fd;
  int track, head, nsect, trk_size, convert, idx, errors;
  char disk_type, image_type;
  char buffer[512 * 20], str[81], tmp, filename[FILENAME_MAX];
  char mark[5] = {'\\', '/', '#', 'E', 'E'};
  char errors_text[20000];

  errors = 0;
  idx = 0;
  errors_text[0] = '\0';

  // Check image file
  if ((GetImageType(in_file, &image_type) == ERR) && (rw_disk == WRITE))
  {
    EEXIT((stderr, "ERROR: NOT a valid image!!\n\n"));
  }

  // Check disk
  if (FD_GetDiskType(&disk_type, &nsect, &trk_size) == ERR)
  {
    EEXIT((stderr, "ERROR: NOT an Ensoniq Disk.\n\n"));
  }

  // Check if disk and file types match
  if (disk_type == EPS_TYPE)
  {
    strcpy(str, "EPS-DISK");
    if (image_type == EDA_TYPE)
    {
      EEXIT((stderr, "ERROR: Don't use EPS disks with EDA files! Replace extension 'eda' to 'ede'.\n"));
    }
  }
  else
  {
    strcpy(str, "ASR-DISK");

    if (image_type == EDE_TYPE)
    {
      EEXIT((stderr, "ERROR: Don't use ASR disks with EDE files! Replace extension 'ede' to 'eda'.\n"));
    }
  }

  //Open FD
  fd = OpenFloppy(0);
  FD_SetGeometry(fd, disk_type);

  // If file type is other than image, conversion is needed

  if ((image_type == GKH_TYPE) && (rw_disk == READ))
  {
    EEXIT((stderr, "Sorry! Reading disk to GKH file is not yet supported!.\n"));
  }

  if ((image_type == EDE_TYPE) || (image_type == EDA_TYPE) || (image_type == GKH_TYPE))
  {
    // Generate tmp-file and bind the clean-up for it
    //tmpnam(tmp_file);
    if (mkstemp(tmp_file) == -1)
    {
      perror("mkstemp");
      exit(ERR);
    }

    atexit(CleanTmpFile);
    convert = 1;
  }
  else
  {
    convert = 0;
    strcpy(filename, in_file);
  }

  // Open image-file
  if (rw_disk == READ)
  {
    if (convert)
      strcpy(filename, tmp_file);
    if ((file = open(filename, O_RDWR | O_CREAT | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", filename));
    }
    printf("Reading %s to file '%s'..\n\n", str, in_file);
  }
  else
  {

    // Check if conversion is needed
    if (convert)
    {

      if (ConvertToImage(in_file, tmp_file) == OK)
      {
        //printf("CONVERSION DONE!!!\n");
        //repalace in_file to converted image
        strcpy(filename, tmp_file);
      }
      else
      {
        EEXIT((stderr, "ERROR: Something wrong with the image file!!\n"));
      }
    }

    file = open(filename, O_RDONLY | O_BINARY);
    printf("Writing %s from file '%s'..\n\n", str, in_file);
  }

  printf("1       10        20        30        40        50        60        70        80\n");
  sprintf(str, "|---+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|");

  for (track = 0; track < 80; track++)
  {
    //FD_Seek(fd,track);
    for (head = 0; head < 2; head++)
    {

      tmp = str[track];
      str[track] = mark[idx];

      printf("\r%s", str);
      fflush(stdout);

      if (rw_disk == WRITE)
      {
        read(file, buffer, trk_size);
      }

      {
        int retry_cnt = 0;

      retry:

        if (FD_RawRW_DiskTrack(fd, disk_type, track, head, buffer, rw_disk) == ERR)
        {
          // Error: 'E'
          idx = 2;
          //Show retry count
          str[track] = '0' + retry_cnt;
          printf("\r%s", str);
          fflush(stdout);
          retry_cnt++;
          if (retry_cnt <= 9)
            goto retry; // Yes, I know.. The Goto is a "bad programming"

          //Error text & count
          sprintf(errors_text + strlen(errors_text), "  ERROR: track=%d, head=%d\n", track, head);
          errors++;
        }
      }

      if (rw_disk == READ)
      {
        write(file, buffer, trk_size);
      }
      idx++;
    } //for head

    if ((mark[idx] != 'E') && ((((track + 1) % 10) == 0) || (track == 0)))
      str[track] = '|';
    else
      str[track] = mark[idx];
    idx = 0;
  } // for track

  printf("\r%s", str);
  fflush(stdout);

  if (convert && rw_disk == READ)
  {
    ConvertFromImage(filename, in_file, image_type);
  }

  if (errors > 0)
  {
    printf("\n\nWarning: %d error(s) found!\n", errors);
    printf("\n%s\n", errors_text);
  }
  else
  {
    printf("\n\nOperation completed succesfully!\n\n");
  }

  CloseFloppy(fd);
}

/////////////////////////////////////////////////////////////////
// ReadBlocks
// ----------
//   media_type  :  'file'-access('f') or disk-access('e','a')
//   fd          :  device (dev/fd0) file descriptor (depends on media_type if needed!)
//   file        :  file descriptor (depends on media_type if needed!)
//   start_block :  First block to read
//   length      :  How many blocks to read
//   buffer      :  Where to put the data

int ReadBlocks(char media_type, FD_HANDLE fd, int file, unsigned int start_block,
               unsigned int length, unsigned char *buffer)
{
  unsigned int sector, head, track;
  unsigned int end_sector, end_head, end_track;
  unsigned int end_block, cur_block, nsect = 0;
  unsigned char tmp_buffer[BLOCK_SIZE * 20];
#ifdef __CYGWIN__
  unsigned int i, count, num_of_blocks;
  unsigned int pre_blocks;
  unsigned int post_blocks;
  unsigned int cont_blocks;
  unsigned int offset;
  int skip;

  //char tmp_buffer[2048];
#endif

  switch (media_type)
  {
  case 'f':
#ifdef __CYGWIN__
  case 's':

    num_of_blocks = length;

    // Calc how many blocks to read in PRE_LOAD state
    pre_blocks = (4 - (start_block % 4)) % 4;
    // Not all blocks in the end of PRE_BLOCKS needed
    // when only 1 or 2 blocks are read in the 2nd or 3rd
    // block of the 2048..

    skip = pre_blocks - num_of_blocks;
    if (skip > 0)
      pre_blocks = pre_blocks - skip;

    cont_blocks = ((num_of_blocks - pre_blocks) / 4) * 4;
    post_blocks = (num_of_blocks - pre_blocks) % 4;
    offset = (start_block * 512) % 2048;
    /*
	printf("Start_block=%d\n",start_block);
	printf("SEEK: %d - Offset=%d\n",(start_block/ 4)*2048,offset);

	printf("Pre_blocks=%d\n",pre_blocks);
	printf("Cont_blocks=%d\n",cont_blocks);
	printf("Post_blocks=%d\n",post_blocks);
	printf("Num_of_blocks=%d <-> total_blocks=%d\n",num_of_blocks,pre_blocks+cont_blocks+post_blocks);
      */

    if ((lseek(file, (long)((start_block / 4) * 2048), SEEK_SET)) < 0)
    {
      perror("seek");
    }

    // PRE BLOCKS
    if (pre_blocks != 0)
    {
      //printf("PRE_LOAD  %d blocks\n",pre_blocks);

      if (read(file, tmp_buffer, 2048) < 2048)
      {
        printf("read error\n");
        exit(ERR);
      }
      memcpy(buffer, tmp_buffer + offset, pre_blocks * 512);
    }

    // CONT BLOCKS
    if ((cont_blocks != 0) && (skip < 0))
    {
      //printf("CONT_LOAD %d blocks\n",cont_blocks);
      if (read(file, buffer + (pre_blocks * 512), 512 * cont_blocks) < (512 * cont_blocks))
      {
        printf("read error\n");
        exit(ERR);
      }
    }

    // POST BLOCKS
    if ((post_blocks != 0) && (skip < 0))
    {
      //printf("POST_LOAD %d blocks\n",post_blocks);
      if ((read(file, buffer + (pre_blocks * 512) + (cont_blocks * 512), 512 * post_blocks)) < (512 * post_blocks))
      {
        printf("read error\n");
        exit(ERR);
      }
    }

#else // Linux
    if (lseek(file, start_block * BLOCK_SIZE, SEEK_SET) == -1)
    {
      printf("ERROR in seek\n");
      exit(ERR);
    }
    if (read(file, buffer, BLOCK_SIZE * length) == 0)
    {
      printf("ERROR in read\n");
      exit(ERR);
    }
#endif
    return (OK);

  case 'e':

    nsect = 10;
    break;

  case 'a':

    nsect = 20;
    break;
  }
  GetTHS(start_block, nsect, &track, &head, &sector);

  end_block = start_block + length - 1;

  GetTHS(end_block, nsect, &end_track, &end_head, &end_sector);

  FD_RawRW_DiskTrack(fd, media_type, track, head, tmp_buffer, READ);

  cur_block = start_block;

  while (cur_block <= end_block)
  {
    memcpy(buffer, tmp_buffer + (sector * BLOCK_SIZE), BLOCK_SIZE);

    buffer = buffer + BLOCK_SIZE;
    cur_block++;
    sector++;
    //printf("\r Reading from disk:  %-4d of %-4d (blocks) .. ", cur_block-start_block, end_block-start_block+1);
    //fflush(stdout);
    if (sector > (nsect - 1) && cur_block <= end_block)
    {
      //Load next track
      GetTHS(cur_block, nsect, &track, &head, &sector);
      FD_RawRW_DiskTrack(fd, media_type, track, head, tmp_buffer, READ);
      sector = 0;
    }
  }
  //printf("\r                                                   ");
  //fflush(stdout);
  return (OK);
}

/////////////////////////////////////////////////////////////////
// WriteBlocks
// ----------
//   media_type  :  'file'-access('f') or disk-access('e','a')
//   fd          :  device (dev/fd0) file descriptor (depends on media_type if needed!)
//   file        :  file descriptor (depends on media_type if needed!)
//   start_block :  First block to write
//   length      :  How many blocks to write
//   buffer      :  Where to get the data

int WriteBlocks(char media_type, FD_HANDLE fd, int file, unsigned int start_block,
                unsigned int length, unsigned char *buffer)
{
  int sector, head, track;
  int end_sector, end_head, end_track;
  unsigned int end_block, cur_block, nsect = 0, num_of_sectors, start_sector, tmp;
  unsigned char tmp_buffer[BLOCK_SIZE * 20];

  switch (media_type)
  {
  case 'f':
#ifdef __CYGWIN__
  case 's':
#endif
    lseek(file, start_block * BLOCK_SIZE, SEEK_SET);
    write(file, buffer, BLOCK_SIZE * length);
    return (OK);

  case 'e':
    nsect = 10;
    break;

  case 'a':
    nsect = 20;
    break;
  }

  GetTHS(start_block, nsect, &track, &head, &sector);
  end_block = start_block + length - 1;
  GetTHS(end_block, nsect, &end_track, &end_head, &end_sector);

  cur_block = start_block;
  start_sector = sector;

  do
  {
    memcpy(tmp_buffer + (sector * BLOCK_SIZE), buffer, BLOCK_SIZE);
    buffer = buffer + BLOCK_SIZE;

    if (sector >= (nsect - 1) || cur_block >= end_block)
    {
      //Write blocks to disk

      num_of_sectors = sector - start_sector + 1;
      GetTHS(cur_block, nsect, &track, &head, &tmp);

      FD_RawRW_DiskSectors(fd, media_type, track, head, start_sector, num_of_sectors,
                           tmp_buffer + (start_sector * BLOCK_SIZE), WRITE);

      sector = 0;
      start_sector = 0;
    }
    else
    {
      sector++;
    }
    cur_block++;
    printf("\r Writing to disk:  %-4d of %-4d (blocks) .. ", cur_block - start_block, end_block - start_block + 1);
    fflush(stdout);
  } while (cur_block <= end_block);
  printf("\r                                              ");
  fflush(stdout);
  return (OK);
}

//////////////////////////////////////////////
// Load Dir entry to Efe-array - use FAT-table
void LoadDirBlocks(char media_type, FD_HANDLE fd, unsigned char *DiskFAT, int file,
                   unsigned long start_blk, int cont, unsigned char Efe[][EFE_SIZE])
{

  unsigned char Dir[DIR_BLOCKS * BLOCK_SIZE];
  int i, j;

  // Get DirBlocks

  // First dir-block - Guess the block are contiguous, so read 2 blocks
  ReadBlocks(media_type, fd, file, start_blk, 2, Dir);

  //.. and second, if needed (ie. not contiguous..)
  if (cont != 2)
  {
    ReadBlocks(media_type, fd, file, GetFatEntry(media_type, DiskFAT, file, start_blk), 1, Dir + BLOCK_SIZE);
  }

  // Scan Directory
  for (i = 0; i < MAX_NUM_OF_DIR_ENTRIES; i++)
  {
    for (j = 0; j < EFE_SIZE; j++)
    {
      Efe[i][j] = Dir[i * EFE_SIZE + j];
    }
  }
}

////////////////////////////////////////////////////
// FormatMedia
// -----------
// -Format (and create if needed) imagefile/device
//
int FormatMedia(char **argv, int argc, int optind, char format_arg, char *disk_label)
{
  int file;

  long long i, j, k, image_size, total_blks, free_blks, fat_blks, fat_count;
  long long partial_blks, rest_blks;
  unsigned char buffer[BLOCK_SIZE], nsect, tracks;

  image_size = 0;

  switch (format_arg)
  {

  case 'a':
    // ASR Disk
    printf("\nFormatting ASR-DISK.. \n\n");
    FD_Format_Disk('a', 20, 0, disk_label);
    return (OK);

    // ASR Disk
  case 'e':
    printf("\nFormatting EPS-DISK.. \n\n");
    FD_Format_Disk('e', 10, 2, disk_label);
    return (OK);

  case 'i':

    // IMAGEFILE/DEVICE

    //Default  values for image/scsi (ie. "don't care")
    // Exception: for EPS/ASR _DISK_ images these values
    //            _has_ to be set correctly in the ID Block
    //            or else you have a disk that _instantly_
    //            reboots the keyboard when accessed!
    //            If You dare, test it yourself.. :-)
    nsect = 0;
    tracks = 0;

    // check if file argument exists
    if (argv[optind] == NULL)
    {
      ShowUsage();
      exit(ERR);
    }

    if ((file = open(argv[optind], O_RDWR | O_BINARY)) != -1)
    {

      //File exists.. get size

      if ((image_size = lseek(file, 0, SEEK_END)) == -1)
      {
        perror("lseek");
        exit(ERR);
      }

      if ((image_size % BLOCK_SIZE) != 0)
      {
        printf("Image size have to be multiple of %d!\n", BLOCK_SIZE);
        exit(ERR);
      }

      if (image_size == EPS_IMAGE_SIZE)
      {
        //Assume EPS Disk image - see above for reason..
        tracks = 80;
        nsect = 10;
      }
      if (image_size == ASR_IMAGE_SIZE)
      {
        //Assume ASR Disk image - see above for reason..
        tracks = 80;
        nsect = 20;
      }

      total_blks = image_size / BLOCK_SIZE;
    }
    else
    {

      // File DOESN'T exists..

      // Check that file size is defined
      if (argv[optind + 1] == NULL)
      {

        printf("No filesize defined!\n\n");
        exit(ERR);
      }
      else
      {

        // Check that given size is correctly formatted
        if ((strcasecmp(argv[optind + 1], "eps")) == 0)
        {
          //eps disk image

          // Set the correct values for nsect and tracks (see above)
          tracks = 80;
          nsect = 10;
          image_size = EPS_IMAGE_SIZE;
        }
        else if ((strcasecmp(argv[optind + 1], "asr")) == 0)
        {
          //asr disk image

          // Set the correct values for nsect and tracks (see above)
          tracks = 80;
          nsect = 20;
          image_size = ASR_IMAGE_SIZE;
        }
        else
        {
          // other
          image_size = atol(argv[optind + 1]);

          if (image_size == 0)
          {
            printf("No valid filesize!\n\n");
            exit(ERR);
          }

          // Check if 'M' or 'K' suffix used
          switch (argv[optind + 1][strlen(argv[optind + 1]) - 1])
          {
          case 'm':
          case 'M':
            image_size = image_size * 1024;
          case 'k':
          case 'K':
            image_size = image_size * 1024;
          }
        }
      }

      // Check the validity of the given filesize ie. that is multiple of block size

      if ((image_size % BLOCK_SIZE) != 0)
      {
        printf("Image size have to be multiple of %d!\n", BLOCK_SIZE);
        exit(ERR);
      }

      total_blks = image_size / BLOCK_SIZE;

      // create file
      file = open(argv[optind], O_RDWR | O_CREAT | O_BINARY, 0600);
      // Make empty file

      // 1st clear the buffer
      for (k = 0; k < BLOCK_SIZE; k++)
      {
        buffer[k] = 0;
      }

      //..create

      partial_blks = total_blks / 100;
      rest_blks = total_blks % 100;

      for (j = 0; j < 100; j++)
      {
        printf("\rCreating image file... %d%% completed", (int)j);
        fflush(stdout);
        for (i = 0; i < partial_blks; i++)
        {
          write(file, buffer, BLOCK_SIZE);
        }
      }
      for (i = 0; i < rest_blks; i++)
      {
        write(file, buffer, BLOCK_SIZE);
      }
      printf("\rCreating image file...  OK!                     \n");
      fflush(stdout);
    }

    // Calc num of needed FAT-Blocks
    printf("\rWriting filesystem....");
    fflush(stdout);

    fat_blks = total_blks / 170;

    if ((total_blks % 170) != 0)
      fat_blks++;

    free_blks = total_blks - (fat_blks + 5);

    //Write "not used" (first) block with added info
    lseek(file, 0, SEEK_SET);
    for (i = 0; i < BLOCK_SIZE; i++)
      buffer[i] = 0;
    sprintf(buffer, "m%s", FIRST_BLOCK_MESSAGE);
    write(file, buffer, BLOCK_SIZE);

    //Write ID-Block
    lseek(file, BLOCK_SIZE * ID_BLOCK, SEEK_SET);
    MakeID_Block(buffer, disk_label, tracks, nsect, total_blks);
    write(file, buffer, BLOCK_SIZE);

    //Write OS-Block
    lseek(file, BLOCK_SIZE * OS_BLOCK, SEEK_SET);
    MakeOS_Block(buffer, free_blks);
    write(file, buffer, BLOCK_SIZE);

    //Write Dir-Blocks
    lseek(file, BLOCK_SIZE * DIR_START_BLOCK, SEEK_SET);
    MakeDR_Block(buffer, 0);
    write(file, buffer, BLOCK_SIZE);

    lseek(file, BLOCK_SIZE * DIR_END_BLOCK, SEEK_SET);
    MakeDR_Block(buffer, 1);
    write(file, buffer, BLOCK_SIZE);

    //Write FAT-Blocks
    fat_count = fat_blks + 5;

    //for(i=0; i<BLOCK_SIZE;i++) {
    //  buffer[i]=0;
    //}
    buffer[510] = 'F';
    buffer[511] = 'B';

    for (i = 0; i < fat_blks; i++)
    {

      // Clear buffer needed: once at start and once after all "used block" marked
      if (fat_count >= 0)
      {
        for (k = 0; k < BLOCK_SIZE - 2; k++)
        {
          buffer[k] = 0;
        }
        if (fat_count == 0)
          fat_count--;
      }

      if (fat_count > 0)
      {
        for (j = 0; j < 510; j = j + 3)
        {
          buffer[j] = 0;
          buffer[j + 1] = 0;
          buffer[j + 2] = 1;
          fat_count--;
          if (fat_count == 0)
          {
            break;
          }
        }
      }

      lseek(file, BLOCK_SIZE * (i + 5), SEEK_SET);
      if (write(file, buffer, BLOCK_SIZE) <= 0)
        perror("write:");
    }
    printf("\rWriting filesystem....  OK!\n\n");
    fflush(stdout);
    close(file);
  } // case
  return (OK);
}

////////////////////////////////////////////////
// Save Dir entry from Efe-array - use FAT-table
void SaveDirBlocks(char media_type, FD_HANDLE fd, unsigned char *DiskFAT, int file,
                   unsigned long start_blk, int cont, unsigned char Efe[][EFE_SIZE])
{

  unsigned char Dir[DIR_BLOCKS * BLOCK_SIZE];
  int i, j;

  // Put Directory
  for (i = 0; i < MAX_NUM_OF_DIR_ENTRIES; i++)
  {
    for (j = 0; j < EFE_SIZE; j++)
    {
      Dir[i * EFE_SIZE + j] = Efe[i][j];
    }
  }
  // .. and tail (ie. '00000000' and 'DR')

  for (i = 10; i < 2; i--)
  {
    Dir[DIR_BLOCKS * BLOCK_SIZE - i] = 0;
  }
  Dir[DIR_BLOCKS * BLOCK_SIZE - 2] = 'D';
  Dir[DIR_BLOCKS * BLOCK_SIZE - 1] = 'R';

  // Put DirBlocks

  if (cont == 2)
  {
    // If contiguous, write all in one
    WriteBlocks(media_type, fd, file, start_blk, 2, Dir);
  }
  else
  {
    // .. and if not, do it separately
    WriteBlocks(media_type, fd, file, start_blk, 1, Dir);
    WriteBlocks(media_type, fd, file, GetFatEntry(media_type, DiskFAT, file, start_blk), 1, Dir + BLOCK_SIZE);
  }
}

/////////////////////////////
// GetEFEs
int GetEFEs(char media_type, FD_HANDLE fd, int in, unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE],
            char *process_efe, unsigned char *DiskFAT)
{
  int out;
  unsigned int i, j, k, size, cont, start, fatval, bp;
  unsigned char type, Header[BLOCK_SIZE], *mem_pointer;
  char name[13], dosname[64], tmp_name[64];
  unsigned char Data[BLOCK_SIZE];
  char type_text[8];
#ifdef __CYGWIN__
  unsigned int tmp_fatval[32768];
  unsigned int total_blks, fat_blks, cont_blks, total_cont_blks;
#endif

  // Process Efe's list
  for (j = 0; j < MAX_NUM_OF_DIR_ENTRIES; j++)
  {

    // Check if current efe has to be processed
    if (process_efe[j] == 0)
      continue;

    // Get info
    if ((type = Efe[j][1]) == 0)
      continue;

    size = (unsigned int)((Efe[j][14] << 8) + Efe[j][15]);
    cont = (unsigned int)((Efe[j][16] << 8) + Efe[j][17]);
    start = (unsigned long)((Efe[j][18] << 24) + (Efe[j][19] << 16) + (Efe[j][20] << 8) + Efe[j][21]);

    //Name
    for (k = 0; k < 12; k++)
    {
      name[k] = Efe[j][k + 2];
    }
    name[12] = 0;

    // Name to DosName
    DosName(dosname, name);

    //Add type (and multifile) prefix to filename
    if (Efe[j][22] != 0)
    {
      strcpy(type_text, EpsTypes[type]);
      type_text[5] = '\0';
      sprintf(type_text, "%s%2d", type_text, Efe[j][22]);
      sprintf(tmp_name, "[%s] %s", type_text, dosname);
    }
    else
    {
      sprintf(tmp_name, "[%s] %s", EpsTypes[type], dosname);
    }
    sprintf(dosname, "%s", tmp_name);

    //Add index prefix to filename
    sprintf(tmp_name, "[%02d]%s", j, dosname);
    sprintf(dosname, "%s", tmp_name);

    // Write EFE-Header

    //Construct Header
    Header[0] = 0x0D;
    Header[1] = 0x0A;
    strcpy(&Header[2], "Eps File:       ");
    strcpy(&Header[18], name);
    strcpy(&Header[30], EpsTypes[type]);
    strcpy(&Header[37], "          ");

    // CR, LF, EOF
    Header[47] = 0x0D;
    Header[48] = 0x0A;
    Header[49] = 0x1A;

    Header[50] = Efe[j][1]; // Instrument
    Header[51] = 0;
    Header[52] = Efe[j][14];
    Header[53] = Efe[j][15];
    Header[54] = Efe[j][16];
    Header[55] = Efe[j][17];
    Header[56] = Efe[j][20];
    Header[57] = Efe[j][21];
    Header[58] = Efe[j][22]; // MultiFile index

    for (i = 59; i < BLOCK_SIZE; i++)
      Header[i] = 0;

    if ((out = open(dosname, O_RDWR | O_CREAT | O_BINARY, S_IRUSR | S_IWUSR)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", dosname));
    }

    write(out, Header, BLOCK_SIZE);

    printf("\rProcessing [%s]...", name);
    fflush(stdout);

    // Copy contiguous blocks
#ifdef __CYGWIN__

    // DISK - ACCESS - Can be copied in one big chunck (disk size is limited!)
    mem_pointer = malloc(BLOCK_SIZE * cont);
    if (mem_pointer == NULL)
      EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
    ReadBlocks(media_type, fd, in, start, cont, mem_pointer);
    write(out, mem_pointer, BLOCK_SIZE * cont);
    free(mem_pointer);

#else // Linux

    if (media_type == 'f')
    {
      // FILE - ACCESS
      for (i = 0; i < cont; i++)
      {
        ReadBlocks(media_type, fd, in, start + i, 1, Data);
        write(out, Data, BLOCK_SIZE);
      }
    }
    else
    {
      // DISK - ACCESS - Can be copied in one big chunck (disk size is limited!)
      mem_pointer = malloc(BLOCK_SIZE * cont);
      if (mem_pointer == NULL)
        EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
      ReadBlocks(media_type, fd, in, start, cont, mem_pointer);
      write(out, mem_pointer, BLOCK_SIZE * cont);
      free(mem_pointer);
    }
#endif

    //Get rest of the blocks
    bp = start + cont - 1;

    fatval = GetFatEntry(media_type, DiskFAT, in, bp);

    if (fatval != 1)
    {

      if (media_type == 'f')
      {

#ifdef __CYGWIN__

        ReadBlocks(media_type, fd, in, ID_BLOCK, 1, Data);
        total_blks = ((Data[14] << 24) +
                      (Data[15] << 16) +
                      (Data[16] << 8) +
                      (Data[17] & 0x000000ff));

        fat_blks = total_blks / 170 + 1;

        mem_pointer = malloc((5 + (fat_blks)) * BLOCK_SIZE);
        if (mem_pointer == NULL)
          EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

        ReadBlocks(media_type, fd, in, 0, 5 + (fat_blks), mem_pointer);
        DiskFAT = mem_pointer + FAT_START_BLOCK * BLOCK_SIZE;
        k = 0;

        while (fatval != 1)
        {

          bp = fatval;
          tmp_fatval[k] = fatval = GetFatEntry('c', DiskFAT, in, bp);
          k++;
        }

        free(mem_pointer);

        k = 0;
        cont_blks = 1;
        total_cont_blks = 0;

        while (tmp_fatval[k] != 1)
        {
          if (tmp_fatval[k + 1] == tmp_fatval[k] + 1)
          {
            cont_blks++;
          }
          else
          {
            cont_blks++;
            mem_pointer = malloc(total_blks * BLOCK_SIZE);
            if (mem_pointer == NULL)
              EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
            ReadBlocks(media_type, fd, in, tmp_fatval[k], cont_blks, mem_pointer);
            write(out, mem_pointer, cont_blks * BLOCK_SIZE);
            free(mem_pointer);
            total_cont_blks = total_cont_blks + cont_blks;
            cont_blks = 1;
          }
          k++;
        }
#else // Linux
        // FILE ACCESS
        while (fatval != 1)
        {
          ReadBlocks(media_type, fd, in, fatval, 1, Data);
          write(out, Data, BLOCK_SIZE);
          bp = fatval;
          fatval = GetFatEntry(media_type, DiskFAT, in, bp);
        }
#endif
      }
      else
      {

        // DISK ACCESS
        while (fatval != 1)
        {

          // Find contigous blocks from FAT-table
          cont = 1;
          start = fatval;
          bp = fatval;
          fatval = GetFatEntry(media_type, DiskFAT, in, start);

          while (fatval == bp + 1)
          {
            bp = fatval;
            fatval = GetFatEntry(media_type, DiskFAT, in, bp);
            cont++;
          }

          // Copy contigous blocks
          mem_pointer = malloc(BLOCK_SIZE * cont);
          if (mem_pointer == NULL)
            EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
          ReadBlocks(media_type, fd, in, start, cont, mem_pointer);
          write(out, mem_pointer, BLOCK_SIZE * cont);
          free(mem_pointer);
        } // while
      }   // else
    }     // if
    printf("\r                                                     ");
    close(out);
  }
  return (OK);
}

//////////////////////////////////////////////////////////////
// PutEFE
// ------
// - This function can be used for writing EFEs to disk/image
//   from EFE-file or from memory. If memory is used, the
//   pointer for efe header (MemDataHdr) and efe data (MemData)
//   are needed!!

int PutEFE(
    char *process_efe,
    unsigned char start_idx,
    unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE],
    char media_type,
    char image_type,
    char *image_file,
    char **efe_files,
    int optind,
    char *orig_image_name,
    unsigned int dir_start,
    unsigned int dir_cont,
    unsigned int total_blks,
    unsigned int *free_blks,
    unsigned int fat_blks,
    FD_HANDLE fd,
    unsigned char *DiskFAT,
    unsigned char *DiskHdr,
    unsigned char *MemDataHdr,
    unsigned char *MemData)
{

  int in, out;
  char in_file[FILENAME_MAX];
  unsigned int idx, i, j, blks, start;
  unsigned char efe_name[13], EfeData[EFE_SIZE], buffer[4], efe_type, Data[BLOCK_SIZE];
  unsigned char *mem_pointer;
  unsigned int efe_start_block, efe_blks, first_free_block, first_cont_blks, prev_block;
  unsigned int free_start, free_cnt, OS;

#ifdef __CYGWIN__
  if (((media_type == 'f') || (media_type == 's')) && (MemData == NULL))
  {
#else // Linux
  if ((media_type == 'f') && (MemData == NULL))
  {
#endif
    // FILE ACCESS

    if ((out = open(image_file, O_RDWR | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open image file '%s'.\n", image_file));
    }
    // Skip image_file
    optind++;
  }

  // PROCESS ALL EFEs

  // KLUDGE!!!
  while ((MemData != NULL) || (efe_files[optind] != NULL))
  {

    // Starting at pos 'start_idx' !!
    // (It's quite a unusual to put other that OS or SubDir to idx 0)
    for (idx = start_idx; idx < MAX_NUM_OF_DIR_ENTRIES; idx++)
    {
      if (Efe[idx][1] == 0)
        break;
    }

    process_efe[idx] = 1;

    if (idx == MAX_NUM_OF_DIR_ENTRIES)
    {
      printf("\r                                         \r");
      fflush(stdout);

      // Write SystemBlocks to disk and free mem
      // so that efes so far will be saved..
      WriteBlocks(media_type, fd, out, 0, 5 + fat_blks, DiskHdr);
      free(DiskHdr);
      printf("ERROR: Directory full!\n");
      //EEXIT((stderr,"ERROR: Directory full!\n"));
      return (ERR);
    }

    if (MemData == NULL)
    {
      //EFE from file
      //=============

      // Efe-file
      if ((in = open(efe_files[optind], O_RDONLY | O_BINARY)) < 0)
      {
        EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", efe_files[optind]));
      }
      strcpy(in_file, efe_files[optind]);

      // Check and convert if 'Mac'-format :-P is found
      if (ConvertMacFormat(&in, in_file) == OK)
      {
        printf("Warning: Macintosh generated EFx file found!\n");
      }

      lseek(in, (long)0x32, SEEK_SET);
      read(in, EfeData, EFE_SIZE);

      lseek(in, (long)0x12, SEEK_SET);
      read(in, efe_name, 12);
      efe_name[12] = '\0';

      efe_blks = (EfeData[2] << 8) + EfeData[3];
      efe_type = EfeData[0];

      // name
      strcpy(&Efe[idx][2], efe_name);
      // zero
      Efe[idx][0] = 0;
      // type
      Efe[idx][1] = efe_type;
      // size
      Efe[idx][14] = EfeData[2];
      Efe[idx][15] = EfeData[3];
      // MultiFile index
      Efe[idx][22] = EfeData[8];
      // empty
      Efe[idx][23] = 0;
      Efe[idx][24] = 0;
      Efe[idx][25] = 0;

      // If efe is OS, get OS version
      switch (efe_type)
      {
      case 1: // EPS OS
        lseek(in, EPS_OS_POS, SEEK_SET);
        read(in, &OS, 4);
        break;
      case 27: // E16 OS
        lseek(in, E16_OS_POS, SEEK_SET);
        read(in, &OS, 4);
        break;
      case 32: // ASR OS
        lseek(in, ASR_OS_POS, SEEK_SET);
        read(in, &OS, 4);
        break;
      default:
        OS = 0;
      }

      //Print progress info..
      printf("\rProcessing [%s]...", efe_name);
      fflush(stdout);

      // Set reading point to start of the data
      lseek(in, BLOCK_SIZE, SEEK_SET);
    }
    else
    {
      // EFE from memory (ie. for ex. mkdir)
      //====================================

#ifdef __CYGWIN__
      if ((media_type == 'f') || (media_type == 's'))
      {
#else // Linux
      if (media_type == 'f')
      {
#endif
        // FILE ACCESS
        // Image-file
        if ((out = open(image_file, O_RDWR | O_BINARY)) < 0)
        {
          EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", image_file));
        }
      }

      // Copy header to Efe (ie. make dir entry)
      memcpy(Efe[idx], MemDataHdr, EFE_SIZE);

      efe_type = MemDataHdr[1];

      if (efe_type == 2)
      {
        // set dir size temporary to 2
        efe_blks = 2;
        // Root dir has info about parent's index in 'cont'
        MemData[16] = 0;
        MemData[17] = idx;
      }
      else
      {
        efe_blks = (MemDataHdr[14] << 8) + MemDataHdr[15];
      }
    }

    if (efe_blks > *free_blks)
    {
      EEXIT((stderr, "Not enough free space! %d needed, %d available.\n", efe_blks, *free_blks));
    }

    first_free_block = 0;
    free_start = 0;
    free_cnt = 0;

    // First PASS - Check if enough contiguos blocks
    for (i = FAT_START_BLOCK + fat_blks; i < total_blks; i++)
    {
      if (GetFatEntry(media_type, DiskFAT, out, i) == 0)
      {
        //Free block found
        if (free_start == 0)
        {
          // Save the num. of the first free block found
          if (first_free_block == 0)
          {
            first_free_block = i;
          }

          free_start = i;
        }
        free_cnt++;

        if (free_cnt == efe_blks)
        {
          // Contiguos blocks found - write whole efe from start_block

          if (media_type == 'f')
          {
            // FILE ACCESS

            // Copy Blocks
            lseek(out, BLOCK_SIZE * free_start, SEEK_SET);
            for (j = 0; j < efe_blks; j++)
            {

              if (MemData == NULL)
              {
                read(in, Data, BLOCK_SIZE);
              }
              else
              {
                memcpy(Data, MemData, BLOCK_SIZE);
                MemData = MemData + BLOCK_SIZE;
              }

              write(out, Data, BLOCK_SIZE);
            }
          }
          else
          {
            // DISK ACCESS
            mem_pointer = malloc(BLOCK_SIZE * efe_blks);
            if (mem_pointer == NULL)
              EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

            if (MemData == NULL)
            {
              read(in, mem_pointer, BLOCK_SIZE * efe_blks);
            }
            else
            {
              memcpy(mem_pointer, MemData, BLOCK_SIZE * efe_blks);
            }

            WriteBlocks(media_type, fd, out, free_start, efe_blks, mem_pointer);
            free(mem_pointer);
          }

          // Write FAT
          for (j = free_start; j < free_start + efe_blks - 1; j++)
          {
            PutFatEntry(media_type, DiskFAT, out, j, j + 1);
          }

          // Mark the end-of-efe
          PutFatEntry(media_type, DiskFAT, out, j, 1);

          efe_start_block = free_start;
          first_cont_blks = efe_blks;
          break;
        }
      }
      else
      {
        free_start = 0;
        free_cnt = 0;
      }
    }

    // Second PASS (if needed)
    // If there was no enough contiguos blocks, the efe must be
    // saved using the space fragments available.. :-(

    if (i == total_blks)
    {
      efe_start_block = first_free_block;
      prev_block = 0;
      free_cnt = 0;
      first_cont_blks = 0;
      blks = 0;
      start = 0;

      // Spilt n' write the efe
      for (i = first_free_block; (i < total_blks) || (free_cnt == efe_blks); i++)
      {

        if (free_cnt == efe_blks)
          break;

        // Try to find free space
        if (GetFatEntry(media_type, DiskFAT, out, i) == 0)
        {

          if (start == 0)
            start = i;

          if (prev_block != 0)
          {
            PutFatEntry(media_type, DiskFAT, out, prev_block, i);
          }
          prev_block = i;
          free_cnt++;
          blks++;
        }
        else
        {
          // Calc cont blocks (first entry);
          if (first_cont_blks == 0)
          {
            first_cont_blks = i - first_free_block;
          }

          // process data in chunks (after found free space)
          if (blks != 0)
          {

            // Allocate mem
            mem_pointer = malloc(BLOCK_SIZE * blks);
            if (mem_pointer == NULL)
              EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

            // Read data
            if (MemData == NULL)
            {
              read(in, mem_pointer, BLOCK_SIZE * blks);
            }
            else
            {
              memcpy(mem_pointer, MemData, BLOCK_SIZE * blks);
              MemData = MemData + BLOCK_SIZE * blks;
            }

            // Write data
            WriteBlocks(media_type, fd, out, start, blks, mem_pointer);

            free(mem_pointer);
            start = 0;
            blks = 0;
          }
        }
      }

      // Allocate mem
      mem_pointer = malloc(BLOCK_SIZE * blks);
      if (mem_pointer == NULL)
        EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

      // Read data
      if (MemData == NULL)
      {
        read(in, mem_pointer, BLOCK_SIZE * blks);
      }
      else
      {
        memcpy(mem_pointer, MemData, BLOCK_SIZE * blks);
        MemData = MemData + BLOCK_SIZE * blks;
      }

      // Write data
      WriteBlocks(media_type, fd, out, start, blks, mem_pointer);

      free(mem_pointer);

      if (i == total_blks)
      {
        EEXIT((stderr, "ERROR: Image/Disk has a corrupted fat!!\n"));
      }

      // Mark the end-of-efe
      PutFatEntry(media_type, DiskFAT, out, i - 1, 1);
    }

    // Update disk-free field
    *free_blks = (*free_blks) - efe_blks;
    buffer[0] = ((*free_blks) >> 24) & 0x000000ff; // MSB
    buffer[1] = ((*free_blks) >> 16) & 0x000000ff;
    buffer[2] = ((*free_blks) >> 8) & 0x000000ff;
    buffer[3] = (*free_blks) & 0x000000ff; // LSB

    if (media_type == 'f')
    {
      // FILE ACCESS
      lseek(out, OS_BLOCK * BLOCK_SIZE, SEEK_SET);
      write(out, buffer, 4);

      // If OS, update OS-version
      if (OS != 0)
      {
        write(out, &OS, 4);
      }
    }
    else
    {
      // DISK ACCESS
      memcpy(DiskHdr + OS_BLOCK * BLOCK_SIZE, buffer, 4);

      // If OS, update OS-version
      if (OS != 0)
      {
        memcpy(DiskHdr + OS_BLOCK * BLOCK_SIZE + 4, &OS, 4);
      }
    }

    // Complete DirEntry

    // size in blocks
    if (efe_type == 2)
    {
      // dir's size = num of files ( = 0 when created)
      Efe[idx][14] = 0;
      Efe[idx][15] = 0;
    }
    else
    {
      Efe[idx][14] = (unsigned char)(efe_blks >> 8) & 0xFF;
      Efe[idx][15] = (unsigned char)(efe_blks & 0xFF);
    }

    // contiguos blocks
    Efe[idx][16] = (unsigned char)(first_cont_blks >> 8);
    Efe[idx][17] = (unsigned char)first_cont_blks & 0x00FF;

    // start block
    Efe[idx][18] = efe_start_block >> 24;
    Efe[idx][19] = efe_start_block >> 16;
    Efe[idx][20] = efe_start_block >> 8;
    Efe[idx][21] = efe_start_block & 0x000000FF;

    // Go to Dir entry

    // Write System Blocks
    if (media_type == 'f')
    {
      // FILE ACCESS
      lseek(out, dir_start * BLOCK_SIZE + EFE_SIZE * idx, SEEK_SET);
      write(out, Efe[idx], EFE_SIZE);
    }
    else
    {
      // DISK ACCESS

      // !!!!!
      if (dir_start == DIR_START_BLOCK)
      {
        memcpy(DiskHdr + dir_start * BLOCK_SIZE + EFE_SIZE * idx, Efe[idx], EFE_SIZE);
      }

      //WriteBlocks(media_type,fd,NULL,0,5+fat_blks,DiskHdr);
    }

    // If Not in Main Dir, update Dir Entries & num. of  files in dir
    if (dir_start != DIR_START_BLOCK)
    {

      unsigned char ParentEfe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE];
      unsigned int parent_dir_idx, parent_dir_files;

      // Save changes in curent dir
      SaveDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, Efe);

      // Get info about parent dir
      parent_dir_idx = (unsigned int)((Efe[0][16] << 8) + Efe[0][17]);
      dir_start = (unsigned long)((Efe[0][18] << 24) + (Efe[0][19] << 16) + (Efe[0][20] << 8) + Efe[0][21]);

      // Don't know how many cont blocks so suppose worst case
      dir_cont = 1;

      // Load parent dir
      LoadDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, ParentEfe);

      // Update parent dir 'num. of files'
      parent_dir_files = (unsigned int)((ParentEfe[parent_dir_idx][14] << 8) +
                                        ParentEfe[parent_dir_idx][15]);

      parent_dir_files++;
      ParentEfe[parent_dir_idx][14] = (unsigned char)(parent_dir_files >> 8) & 0xFF;
      ParentEfe[parent_dir_idx][15] = (unsigned char)(parent_dir_files & 0xFF);

      // Save changes in parent dir
      SaveDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, ParentEfe);
    }

    optind++;

    // KLUDGE!!!
    if (MemData != NULL)
      break;

  } // while(efe_files...)

  // Free memory used for DiskFat etc. cache
  if (media_type != 'f')
  {
    // Write SystemBlocks to disk and free mem
    WriteBlocks(media_type, fd, out, 0, 5 + fat_blks, DiskHdr);
    free(DiskHdr);
  }
  else
  {

    // Convert to back to orginal format if not raw-image
    if ((image_type != EPS_TYPE) && (image_type != ASR_TYPE) && (image_type != OTHER_TYPE))
    {

      if (MemData != NULL)
      {
        close(in);
      }

      close(out);
      ConvertFromImage(image_file, orig_image_name, image_type);
    }
  }

  printf("\r                                         \r");
  fflush(stdout);
}

/////////////////////////////
// EraseEFEs
int EraseEFEs(char media_type, char image_type, FD_HANDLE fd,
              unsigned char *DiskFAT, unsigned char *DiskHdr,
              char *in_file, char *orig_image_name,
              unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE], char *process_efe,
              unsigned int fat_blks, unsigned int *free_blks,
              unsigned int dir_start, unsigned int dir_cont)
{
  int out;
  unsigned int size, cont, start, type, i, j, counter;
  unsigned int OS = 1;
  unsigned char buffer[4];

  counter = 0;

  // FILE ACCESS
#ifdef __CYGWIN__
  if ((media_type == 'f') || (media_type == 's'))
  {
#else //Linux
  if (media_type == 'f')
  {
#endif
    // Image-file
    if ((out = open(in_file, O_RDWR | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", in_file));
    }
  }

  // Process Efe's list
  for (j = 0; j < MAX_NUM_OF_DIR_ENTRIES; j++)
  {
    // Check if current efe has to be processed
    if (process_efe[j] == 0)
      continue;

    // Get info
    type = Efe[j][1];

    switch (type)
    {
    case 0: // Empty
      continue;

    case 2: // Sub dir
      if (Efe[j][15] > 0)
      {
        fprintf(stderr, "\nIDX %-2d: Can't erase directory! Directory not empty.\n\n", j);
        continue;
      }
      size = 2;
      break;

    case 8: // Pointer to 'root'
      fprintf(stderr, "\nIDX %-2d: Can't erase link to parent dir!\n\n", j);
      continue;

      // If erasing os, clear version field
    case 1:
    case 27:
    case 32:
      OS = 0;

    default:
      size = (unsigned int)((Efe[j][14] << 8) + Efe[j][15]);
    }

    cont = (unsigned int)((Efe[j][16] << 8) + Efe[j][17]);
    start = (unsigned long)((Efe[j][18] << 24) + (Efe[j][19] << 16) + (Efe[j][20] << 8) + Efe[j][21]);

    // Calculate 'disk-free'
    *free_blks = (*free_blks) + size;

    // Clear FAT entries

    while ((i = GetFatEntry(media_type, DiskFAT, out, start)) != 1)
    {
      PutFatEntry(media_type, DiskFAT, out, start, 0);
      start = i;
    }
    // .. and last entry (ie. stopmark '1')
    PutFatEntry(media_type, DiskFAT, out, start, 0);

    // Clear Dir entry
    for (i = 0; i < 26; i++)
    {
      Efe[j][i] = 0;
    }

    // Count erased files for 'num of files' in parent dir
    counter++;

  } // For 'process_efes'

  // Update disk-free field
  buffer[0] = ((*free_blks) >> 24) & 0x000000ff; // MSB
  buffer[1] = ((*free_blks) >> 16) & 0x000000ff;
  buffer[2] = ((*free_blks) >> 8) & 0x000000ff;
  buffer[3] = (*free_blks) & 0x000000ff; // LSB

  if (media_type == 'f')
  {
    // FILE ACCESS
    lseek(out, OS_BLOCK * BLOCK_SIZE, SEEK_SET);
    write(out, buffer, 4);
    // If erasing OS, clear OS-field
    if (!OS)
    {
      write(out, &OS, 4);
    }
  }
  else
  {
    // DISK ACCESS
    memcpy(DiskHdr + OS_BLOCK * BLOCK_SIZE, buffer, 4);
    // If erasing OS, clear OS-field
    if (!OS)
    {
      memcpy(DiskHdr + OS_BLOCK * BLOCK_SIZE + 4, &OS, 4);
    }
  }

  // Write System Blocks
  if (media_type != 'f')
  {
    // DISK ACCESS
    WriteBlocks(media_type, fd, out, 0, 5 + fat_blks, DiskHdr);
  }

  // Update Dir Entries
  SaveDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, Efe);

  // If Not in Main Dir, update Dir Entries & num. of  files in dir
  if (dir_start != DIR_START_BLOCK)
  {

    unsigned char ParentEfe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE];
    unsigned int parent_dir_idx, parent_dir_files;

    // Get info about parent dir
    parent_dir_idx = (unsigned int)((Efe[0][16] << 8) + Efe[0][17]);
    dir_start = (unsigned long)((Efe[0][18] << 24) + (Efe[0][19] << 16) + (Efe[0][20] << 8) + Efe[0][21]);

    // Don't know how many cont blocks so suppose worst case
    dir_cont = 1;

    // Load parent dir
    LoadDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, ParentEfe);

    // Update parent dir 'num of files'
    parent_dir_files = (unsigned int)((ParentEfe[parent_dir_idx][14] << 8) +
                                      ParentEfe[parent_dir_idx][15]);

    // Decrement 'num. of files' in parent dir
    parent_dir_files = parent_dir_files - counter;

    // if everything is OK, this _should_ NOT happen...
    if (parent_dir_files < 0)
      parent_dir_files = 0;

    ParentEfe[parent_dir_idx][14] = (unsigned char)(parent_dir_files >> 8) & 0xFF;
    ParentEfe[parent_dir_idx][15] = (unsigned char)(parent_dir_files & 0xFF);

    // Save changes in parent dir
    SaveDirBlocks(media_type, fd, DiskFAT, out, dir_start, dir_cont, ParentEfe);
  }

  // Free memory used for DiskFat etc. cache
  if (media_type != 'f')
  {
    free(DiskHdr);
  }
  else
  {

    // Convert to back to orginal format if not raw-image
    if ((image_type != EPS_TYPE) && (image_type != ASR_TYPE) && (image_type != OTHER_TYPE))
    {
      close(out);
      ConvertFromImage(in_file, orig_image_name, image_type);
    }
  }

  return (OK);
}

//////////////////
// MkDir
int MkDir(
    char *process_efe,
    unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE],
    char media_type,
    char image_type,
    char *image_file,
    char *orig_image_name,
    unsigned int dir_start,
    unsigned int dir_cont,
    char *dir_name,
    unsigned int total_blks,
    unsigned int *free_blks,
    unsigned int fat_blks,
    FD_HANDLE fd,
    unsigned char *DiskFAT,
    unsigned char *DiskHdr,
    char DirName[12])
{
  unsigned char MemDataHdr[EFE_SIZE], *MemData;
  int i;

  for (i = 0; i < EFE_SIZE; i++)
  {
    MemDataHdr[i] = 0;
  }

  // Convert name
  for (i = 0; i < 12; i++)
  {
    DirName[i] = (unsigned char)toupper(DirName[i]);
  }

  // Make header

  // Type  (2)
  MemDataHdr[1] = 2;
  // Name
  for (i = 0; i < 12; i++)
  {
    if (DirName[i] == 0)
    {
      MemDataHdr[2 + i] = 0x20;
    }
    else
    {
      MemDataHdr[2 + i] = DirName[i];
    }
  }
  // Size (2)
  MemDataHdr[15] = 2;

  // Make data

  //  2 x Blocks
  MemData = calloc(2 * BLOCK_SIZE, sizeof(unsigned char));
  if (MemData == NULL)
    EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

  // First entry is root-dir

  // Type (8)
  MemData[1] = 8;
  // Name of parent dir
  strncpy(MemData + 2, dir_name, 12);
  // Cont ( < 255!!)
  MemData[17] = (unsigned char)dir_cont;
  // Root Dir start block
  MemData[18] = (dir_start >> 24) & 0x000000FF;
  MemData[19] = (dir_start >> 16) & 0x000000FF;
  MemData[20] = (dir_start >> 8) & 0x000000FF;
  MemData[21] = dir_start & 0x000000FF;

  // 'DR'
  MemData[2 * BLOCK_SIZE - 2] = 0x44;
  MemData[2 * BLOCK_SIZE - 1] = 0x52;

  // Write dir

  PutEFE(process_efe, 1, Efe, media_type, image_type,
         image_file, NULL, 0, orig_image_name,
         dir_start, dir_cont, total_blks, free_blks, fat_blks,
         fd, DiskFAT, DiskHdr, MemDataHdr, MemData);

  return (OK);
}

/////////////////////////////
// SplitEfe
void SplitEfe(char *in_file, char *slice_type, int argc)
{
  int in, out;
  struct stat stat_buf;
  unsigned int i, slice_size, slice_count, data_len;
  unsigned char Data[BLOCK_SIZE];
  unsigned char Hdr[BLOCK_SIZE];
  char out_file[FILENAME_MAX], out_file_head[12], out_file_tail[FILENAME_MAX];

  // Check arguments
  if ((argc != 4) || ((strcasecmp(slice_type, "eps") != 0) && (strcasecmp(slice_type, "asr") != 0)))
  {
    fprintf(stderr, "ERROR: Invalid or wrong number of arguments\n");
    ShowUsage();
    exit(ERR);
  }

  if (strcasecmp(slice_type, "eps") == 0)
  {
    slice_size = 1585 * BLOCK_SIZE;
  }
  else
  {
    slice_size = 3176 * BLOCK_SIZE;
  }

  // Open input Efe file
  if ((in = open(in_file, O_RDONLY | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", in_file));
  }

  // Get filesize
  if (stat(in_file, &stat_buf) != 0)
  {
    EEXIT((stderr, "\n(line: %d) ERROR: Can't get the filesize!!\n", __LINE__));
  }

  data_len = stat_buf.st_size - BLOCK_SIZE;

  // Check the validity of the given filesize ie. that is multiple of block size
  if ((data_len % BLOCK_SIZE) != 0)
  {
    EEXIT((stderr, "Image size have to be multiple of %d!\n", BLOCK_SIZE));
  }

  // Check if split is needed..
  if (data_len <= slice_size)
  {
    printf("No split needed. EFE will fit in one disk!\n");
    exit(OK);
  }

  // ASR max mem 16M!
  if (data_len > 16777216)
  {
    EEXIT((stderr, "EFE is too big! Maybe corrupted !?\n"));
  }

  // Read Efe Header
  //fseek(in, 512,0);
  read(in, &Hdr, BLOCK_SIZE);

  // Check Efe type
  if (Hdr[0x32] != 3)
  {
    EEXIT((stderr, "ERROR: File '%s' is not an INSTRUMENT type.\n", in_file));
  }

  // Check filesize and Efe header blocksize match..
  if (((data_len / BLOCK_SIZE) != (((Hdr[0x34] << 8) + Hdr[0x35]))))
  {
    EEXIT((stderr, "ERROR: File size and block size in Efe header doesn't match!\n"));
  }

  // If Efe is named using EpsLin "format"
  if (strncmp("[Instr  ]", in_file + 4, 9) == 0)
  {
    in_file[10] = '\0';
    strcpy(out_file_head, in_file);
    strcpy(out_file_tail, in_file + 12);
  }
  else
  {
    strcpy(out_file_head, "Part_");
    sprintf(out_file_tail, "_%s", in_file);
  }

  slice_count = 0;

  printf("\nSplitting in progress:\n");

  // Split
  while (data_len > 0)
  {

    // Seek imputfile
    lseek(in, BLOCK_SIZE + slice_count * slice_size, 0);

    // Construct output filename
    slice_count++;
    sprintf(out_file, "%s%02d%s", out_file_head, slice_count, out_file_tail);

    if ((out = open(out_file, O_RDWR | O_CREAT | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", out_file));
    }

    printf("\rWriting MultiFile EFE #%d", slice_count);

    // If last slice...
    if (data_len < slice_size)
      slice_size = data_len;

    // Generate EFE header

    Hdr[0x34] = ((slice_size / BLOCK_SIZE) & 0xFF00) >> 8; // Blocks
    Hdr[0x35] = ((slice_size / BLOCK_SIZE) & 0x00FF);
    Hdr[0x3a] = slice_count; // Multifile

    // Write header
    write(out, Hdr, BLOCK_SIZE);

    // Copy data
    for (i = 0; i < slice_size; i = i + BLOCK_SIZE)
    {
      read(in, Data, BLOCK_SIZE);
      write(out, Data, BLOCK_SIZE);
    }

    data_len = data_len - slice_size;

  } // while

  printf("\rEFE slpitting OK!            \n");

  close(in);
  close(out);
}

/////////////////////////////
// JoinEfe
void JoinEfes(int argc, char **argv)
{
  int in, out;
  unsigned char Data[BLOCK_SIZE];
  unsigned char Hdr[BLOCK_SIZE];
  char out_file[FILENAME_MAX];
  char in_file[FILENAME_MAX];
  struct stat stat_buf;
  unsigned int i, slice_idx, data_len, data_total;

  // Check arguments
  if (argc < 5)
  {
    fprintf(stderr, "ERROR: Invalid or wrong number of arguments\n");
    ShowUsage();
    exit(ERR);
  }

  sprintf(out_file, argv[2]);
  // Open Output file
  if ((out = open(out_file, O_RDWR | O_CREAT | O_BINARY)) < 0)
  {
    EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", out_file));
  }

  slice_idx = 0;
  data_total = 0;

  while (slice_idx < (argc - 3))
  {
    strcpy(in_file, argv[slice_idx + 3]);

    printf("Processin Efe part #%d: %s\n", slice_idx + 1, in_file);

    // Open input Efe file
    if ((in = open(in_file, O_RDONLY | O_BINARY)) < 0)
    {
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", in_file));
    }

    // Get filesize
    if (stat(in_file, &stat_buf) != 0)
    {
      EEXIT((stderr, "\n(line: %d) ERROR: Can't get the filesize!!\n", __LINE__));
    }

    data_len = stat_buf.st_size - BLOCK_SIZE;

    // Check the validity of the given filesize ie. that is multiple of block size
    if ((data_len % BLOCK_SIZE) != 0)
    {
      EEXIT((stderr, "Image size have to be multiple of %d!\n", BLOCK_SIZE));
    }

    // Read Efe Header
    read(in, &Hdr, BLOCK_SIZE);

    // Check Efe type
    if (Hdr[0x32] != 3)
    {
      EEXIT((stderr, "ERROR: File '%s' is not an INSTRUMENT type.\n", in_file));
    }

    // Check filesize and Efe header blocksize match..
    if (((data_len / BLOCK_SIZE) != (((Hdr[0x34] << 8) + Hdr[0x35]))))
    {
      EEXIT((stderr, "ERROR: File size and block size in Efe header doesn't match!\n"));
    }

    // Copy header before first slice..
    if (slice_idx == 0)
    {
      write(out, Hdr, BLOCK_SIZE);
    }

    // Copy data
    for (i = 0; i < data_len; i = i + BLOCK_SIZE)
    {
      read(in, Data, BLOCK_SIZE);
      write(out, Data, BLOCK_SIZE);
    }

    data_total = data_total + data_len;
    slice_idx++;
  } // while

  //Fix the final header of the output file
  // Generate EFE header

  Hdr[0x34] = ((data_total / BLOCK_SIZE) & 0xFF00) >> 8; // Blocks
  Hdr[0x35] = ((data_total / BLOCK_SIZE) & 0x00FF);
  Hdr[0x3a] = 0; // Singlefile

  // Overwrite header
  lseek(out, 0L, SEEK_SET);
  write(out, Hdr, BLOCK_SIZE);
  printf("Join operation OK! Total size of the joined EFE is %d blocks.\n\n", data_total / BLOCK_SIZE);
  close(in);
  close(out);
}

/////////////////////////////
// DoConversion
void DoConversion(char in_file[FILENAME_MAX], char out_file[FILENAME_MAX], int argc)
{
  struct stat stat_buf;
  char image_type;

  if (argc != 4)
  {
    fprintf(stderr, "ERROR: Wrong number of arguments\n");
    ShowUsage();
    exit(ERR);
  }

  if (ConvertToImage(in_file, out_file) == OK)
  {
    //printf("CONVERSION DONE!!!\n");
    exit(OK);
  }

  // No conversion - suppose img-file
  // Check if ASR or EPS image

  if (stat(in_file, &stat_buf) != 0)
  {
    EEXIT((stderr, "ERROR: Can't get the filesize!!\n"));
  }

  if (stat_buf.st_size == EPS_IMAGE_SIZE)
  {
    image_type = EDE_TYPE;
  }
  else if (stat_buf.st_size == ASR_IMAGE_SIZE)
  {
    image_type = EDA_TYPE;
  }
  else
  {
    EEXIT((stderr, "ERROR: Unsupported image type!\n\n"));
  }

  ConvertFromImage(in_file, out_file, image_type);

  //printf("Conversion succesfully!\n");
}

/////////////////////////////////////////
// GetMedia
// ------------
// - Determines if disk or image is used,
//   and does nessecery opening and conversions

void GetMedia(char *arg, int argc, char *media_type, char *image_type,
              unsigned int *nsect, unsigned int *trk_size, FD_HANDLE *fd,
              char *in_file, int *in)
{
  unsigned int tmp;

  // Determines is DISK or FILE access

  // TODO: THIS IS EXPERIMENTAL
  if ((arg == NULL) || ((IsEFE(arg) == OK) && (argc >= 3)))
  {
    // DISK ACCESS
    if (FD_GetDiskType(media_type, nsect, trk_size) == ERR)
    {
      EEXIT((stderr, "ERROR: NOT an Ensoniq Disk.\n       Please format the disk (-fe or -fa) and try again!!\n\n"));
    }

    //Open FD
    *fd = OpenFloppy(0);
  }
  else
  {
    // FILE ACCESS
    *media_type = 'f';

    // Get the image filename
    strcpy(in_file, arg);

    GetImageType(in_file, image_type);

    // Check if conversion is needed!
    if ((*image_type != EPS_TYPE) && (*image_type != ASR_TYPE) && (*image_type != OTHER_TYPE))
    {

      // Generate tmp-file and bind the clean-up for it
      //tmpnam(tmp_file);
      mkstemp(tmp_file);
      atexit(CleanTmpFile);

      if (ConvertToImage(in_file, tmp_file) == OK)
      {
        //repalace in_file to converted image
        strcpy(in_file, tmp_file);
      }
    }

    if ((*in = open(in_file, O_RDONLY | O_BINARY)) < 0)
    {
      perror("open:");
      EEXIT((stderr, "ERROR: Couldn't open file '%s'.\n", in_file));
    }

    if (IsEFE(in_file) != OK)
    {
      // Check that EPS/ASR image is valid! (ie. do the 'ID-check')
#ifdef __CYGWIN__
      // To get /dev/scd work...
      {
        unsigned char tmp_buff[512];
        ReadBlocks(*media_type, *fd, *in, 1, 1, tmp_buff);
        tmp = *(((unsigned int *)tmp_buff) + 9);
      }
#else
      lseek(*in, (long)0x224, SEEK_SET);
      read(*in, &tmp, 4);
#endif

      if ((tmp & 0xffff0000) != 0x44490000)
      {
        EEXIT((stderr, "ERROR: Not a valid image file!\n"));
      }
#ifdef __CYGWIN__
      // Find out if Iomega Zip device ie. writing only possible
      // in blocks.. !!
      {
        int out;
        unsigned char mark = 'I';
        struct stat buf;
        if (lstat(in_file, &buf) < 0)
        {
          perror("lstat");
          EEXIT((stderr, "Couldn't get info about file!"));
        }
        // If block device
        if (S_ISBLK(buf.st_mode))
        {
          // If CD-ROM
          if (strncmp("/dev/scd", in_file, 8) == 0)
          {
            *media_type = 's';
          }
          else
          {
            // Other block device...
            if ((out = open(in_file, O_RDWR | O_BINARY)) < 0)
            {
              perror("open:");
              EEXIT((stderr, "ERROR: Couldn't open file for write '%s'.\n", in_file));
            }

            // If can't read single byte, set media type to 's' = special
            // ie. Zip-dirve..
            if (lseek(out, 0x226, SEEK_SET) < 0)
            {
              perror("seek");
              EEXIT((stderr, "ERROR: file is not seekable!"));
            }
            if (write(out, &mark, 1) <= 0)
              *media_type = 's';
            close(out);
          }
        }
      }
#endif
    }
  }
}

////////////////////////////////////////////////////
// GetInfo
// -------
// -Get info about used media and loads the selected
//  directory structure.
//

void GetInfo(char *media_type, FD_HANDLE fd, unsigned char **DiskFAT, unsigned char **DiskHdr,
             int in,
             unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE],
             unsigned int *fat_blks, unsigned int *free_blks, unsigned int *total_blks,
             unsigned int *dir_start, unsigned int *dir_cont,
             char *parent_dir_name, int subdir_cnt,
             unsigned int *DirPath, char *DiskLabel)
{

  unsigned char *mem_pointer;
  unsigned int tmp, i, j;

  if (*media_type == 'f')
  {
    // FILE ACCESS
    mem_pointer = malloc(5 * BLOCK_SIZE);
    if (mem_pointer == NULL)
      EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

    ReadBlocks(*media_type, fd, in, 0, 5, mem_pointer);

#ifdef __CYGWIN__
  }
  else if (*media_type == 's')
  {
    unsigned char Data[BLOCK_SIZE];
    unsigned char *ptr;
    ReadBlocks(*media_type, fd, in, ID_BLOCK, 1, Data);
    *total_blks = ((Data[14] << 24) +
                   (Data[15] << 16) +
                   (Data[16] << 8) +
                   (Data[17] & 0x000000ff));

    *fat_blks = *total_blks / 170 + 1;
#ifdef DEBUG
    printf("total_blks=%d\n", *total_blks);
#endif
    mem_pointer = malloc((5 + (*fat_blks)) * BLOCK_SIZE);
    if (mem_pointer == NULL)
      EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));
#ifdef DEBUG
    printf("Getinfo - read (%d blocks) start..", 5 + *fat_blks);
#endif
    ReadBlocks(*media_type, fd, in, 0, 5 + (*fat_blks), mem_pointer);
#ifdef DEBUG
    printf("Getinfo - read stop..\n");
#endif
    *DiskHdr = mem_pointer;
    *DiskFAT = mem_pointer + FAT_START_BLOCK * BLOCK_SIZE;
#endif
  }
  else
  {
    // DISK ACCESS
    if (*media_type == 'e')
      *fat_blks = 10;
    else
      *fat_blks = 19;

    mem_pointer = malloc((5 + (*fat_blks)) * BLOCK_SIZE);
    if (mem_pointer == NULL)
      EEXIT((stderr, "ERROR: Couldn't allocate memory !!!!\n"));

    ReadBlocks(*media_type, fd, in, 0, 5 + (*fat_blks), mem_pointer);
    *DiskHdr = mem_pointer;
    *DiskFAT = mem_pointer + FAT_START_BLOCK * BLOCK_SIZE;
  }

  //  Get 'TotalBlocks' and 'DiskLabel' from ID_BLOCK

  tmp = ID_BLOCK * BLOCK_SIZE;
  *total_blks = (unsigned int)((mem_pointer[tmp + 14] << 24) +
                               (mem_pointer[tmp + 15] << 16) +
                               (mem_pointer[tmp + 16] << 8) +
                               (mem_pointer[tmp + 17] & 0x000000ff));

  strncpy(DiskLabel, mem_pointer + tmp + 31, 7);
  DiskLabel[7] = '\0';

  if (strlen(DiskLabel) == 0)
    strcpy(DiskLabel, "<NONE>");
  *fat_blks = (*total_blks / 170) + 1;

  //  Get 'FreeBlocks' from OS_BLOCK

  tmp = OS_BLOCK * BLOCK_SIZE;
  *free_blks = (unsigned int)((mem_pointer[tmp] << 24) +
                              (mem_pointer[tmp + 1] << 16) +
                              (mem_pointer[tmp + 2] << 8) +
                              (mem_pointer[tmp + 3] & 0x000000ff));

  // Load main directory
  *dir_start = DIR_START_BLOCK;
  *dir_cont = 2;

  // Scan Main Directory
  for (i = 0; i < MAX_NUM_OF_DIR_ENTRIES; i++)
  {
    for (j = 0; j < EFE_SIZE; j++)
    {
      Efe[i][j] = mem_pointer[DIR_START_BLOCK * BLOCK_SIZE + i * EFE_SIZE + j];
    }
  }

  //LoadDirBlocks(media_type,fd,DiskFAT,in,dir_start,dir_cont,Efe);

  // SUB-DIRS - Use the 'path' to 'change dir'...
  if (subdir_cnt > 0)
  {
    for (i = 0; i < subdir_cnt; i++)
    {
      j = DirPath[i];
      if (Efe[j][1] != 2)
      {
        EEXIT((stderr, "ERROR: Index '%d' is not a directory!\n\n", j));
      }

      strncpy(parent_dir_name, Efe[j] + 2, 12);
      *dir_cont = (unsigned int)((Efe[j][16] << 8) + Efe[j][17]);
      *dir_start = (unsigned long)((Efe[j][18] << 24) + (Efe[j][19] << 16) + (Efe[j][20] << 8) + Efe[j][21]);

      LoadDirBlocks(*media_type, fd, *DiskFAT, in, *dir_start, *dir_cont, Efe);
    }
  }
  else
  {
    // if parent is root dir, set parent_name to 'ROOT'
    strncpy(parent_dir_name, "ROOT        ", 12);
  }
}

////////////////////////////////////////////////////
// CheckMedia
// ----------
// -Get info about used media and check the filesystem
//  structure

void CheckMedia(char media_type, FD_HANDLE fd, int file, int check_level)
{
  unsigned long i, j, count, used_blks;
  unsigned char buffer[BLOCK_SIZE], *root_dir;
  unsigned long total_blks, free_blks;
  unsigned long file_size;

  //Get File/Device info
#ifdef __CYGWIN__
  if ((media_type == 'f') || (media_type == 's'))
  {
#else // Linux
  if (media_type == 'f')
  {
#endif
    // FILE ACCESS
    if ((file_size = lseek(file, 0, SEEK_END)) == -1)
    {
      perror("lseek");
      exit(ERR);
    }
  }
  else
  {
    // DISK ACCESS
#ifdef __CYGWIN__
    // How to do this in Windows!?

#else // Linux
    if ((file_size = lseek(fd, 0, SEEK_END)) == -1)
    {
      perror("lseek");
      exit(ERR);
    }
#endif
  }

  printf("\nFile/Device size: %ld bytes (%ld blocks)\n\n", file_size, file_size / BLOCK_SIZE);

  //ID-Block

  ReadBlocks(media_type, fd, file, ID_BLOCK, 1, buffer);

  printf("\nID-BLOCK\n");
  printf("========\n\n");

  printf("Device type            : %d(%x)\n", buffer[0], buffer[0]);
  printf("Rem. Media Device type : %d(%x)\n", buffer[1], buffer[1]);
  printf("Std.Version#           : %d(%x)\n", buffer[2], buffer[2]);
  printf("SCSI (?)               : %d(%x)\n", buffer[3], buffer[3]);
  printf("Number of Sectors      : %d(%x%x)\n", (buffer[4] * 256 + buffer[5]), buffer[4], buffer[5]);
  printf("Number of Heads        : %d(%x%x)\n", (buffer[6] * 256 + buffer[7]), buffer[6], buffer[7]);
  printf("Number of Tracks       : %d(%x%x)\n", (buffer[8] * 256 + buffer[9]), buffer[8], buffer[9]);
  printf("Bytes per Block (512)  : %d(%x%x)\n", (buffer[12] * 256 + buffer[13]), buffer[12], buffer[13]);

  total_blks = ((buffer[14] << 24) +
                (buffer[15] << 16) +
                (buffer[16] << 8) +
                (buffer[17] & 0x000000ff));
  printf("Total Blocks           : %ld(%x%x%x%x) (%ld Bytes = %ld KBytes = %ld MBytes)\n",
         total_blks,
         buffer[14], buffer[15], buffer[16], buffer[17],
         total_blks * 512, total_blks * 512 / 1024, total_blks * 512 / 1024 / 1024);

  printf("SCSI Medium type       : %d(%x)\n", buffer[18], buffer[18]);
  printf("SCSI Density Code      : %d(%x)\n", buffer[19], buffer[19]);

  printf("DISK LABEL             : \"%c%c%c%c%c%c%c\" \n", buffer[31], buffer[32], buffer[33],
         buffer[34], buffer[35], buffer[36], buffer[37]);

  printf("End of ID-Block(\"ID\")  : \"%c%c\"\n", buffer[38], buffer[39]);

  //OS-Block

  ReadBlocks(media_type, fd, file, OS_BLOCK, 1, buffer);

  printf("\nOS-BLOCK\n");
  printf("========\n\n");

  free_blks = ((buffer[0] << 24) +
               (buffer[1] << 16) +
               (buffer[2] << 8) +
               (buffer[3] & 0x000000ff));

  printf("Free Blocks            : %ld(%x%x%x%x)\n", free_blks,
         buffer[0], buffer[1], buffer[2], buffer[3]);

  printf("Disk OS Version        : %d.%d(%x.%x)\n", buffer[4], buffer[5], buffer[4], buffer[5]);
  printf("ROM  OS Version        : %d.%d(%x.%x)\n", buffer[6], buffer[7], buffer[6], buffer[7]);

  printf("End of OS-Block(\"OS\")  : \"%c%c\"\n", buffer[28], buffer[29]);

  printf("\nOther Blocks\n");
  printf("============\n\n");

  count = 0;
  used_blks = 0;

  ReadBlocks(media_type, fd, file, DIR_END_BLOCK, 1, buffer);

  printf("Root DirBlocks    : ");
  if (buffer[510] == 'D' && buffer[511] == 'R')
    printf("OK\n");
  else
    printf("NOT OK - DISK/FILE is corrupted!\n");

  for (i = FAT_START_BLOCK;; i++)
  {

    ReadBlocks(media_type, fd, file, i, 1, buffer);

    //Check if valid FAT BLOCK
    if (buffer[510] != 'F' || buffer[511] != 'B')
      break;

    //Count used blocks
    for (j = 2; j < 510; j = j + 3)
    {

      if (!((buffer[j] == 0) && (buffer[j - 1] == 0) && (buffer[j - 2] == 0)))
        used_blks++;
    }
    count++;
  }

  printf("Num. of FatBlocks : %ld\n", count);
  printf("FAT entries       : %ld  => ", 170 * count);
  if ((170 * count) >= total_blks)
    printf("OK\n");
  else
    printf("FAT corrupted!! Not enough FAT tables!\n");

  printf("Used Blocks in FAT: %ld  => ", used_blks);
  if ((total_blks - (used_blks + free_blks)) == 0)
    printf("OK\n");
  else
    printf("FAT corrupted!! Used blocks should be %ld\n", total_blks - free_blks);

  printf("\n");

  // If higher check_level, display also root dir structure
  if (check_level > 0)
  {
    // Directory structure info
    printf("\nROOT DIR:\n");
    printf("=========\n");

    root_dir = (unsigned char *)malloc(BLOCK_SIZE * 2);

    ReadBlocks(media_type, fd, file, DIR_START_BLOCK, 2, root_dir);

    // Scan Directory
    for (i = 0; i < MAX_NUM_OF_DIR_ENTRIES; i++)
    {
      char name[13];
      printf("%3ld:", i);
      for (j = 0; j < EFE_SIZE; j++)
      {
        printf("%02x ", root_dir[i * EFE_SIZE + j]);
      }
      printf("\n");
      strncpy(name, &root_dir[i * EFE_SIZE + 2], 12);
      name[12] = '\0';

      printf("    Type:%2d, Name:%12s, Size:%ld, Cont:%ld, Start:%ld \n\n",
             root_dir[i * EFE_SIZE + 1],
             name,
             (long)root_dir[i * EFE_SIZE + 14] * 256 + root_dir[i * EFE_SIZE + 15],
             (long)root_dir[i * EFE_SIZE + 16] * 256 + root_dir[i * EFE_SIZE + 17],
             (long)root_dir[i * EFE_SIZE + 18] * 256 * 256 * 256 + root_dir[i * EFE_SIZE + 19] * 256 * 256 +
                 root_dir[i * EFE_SIZE + 20] * 256 + root_dir[i * EFE_SIZE + 21]);
    }
    free(root_dir);
  }
}

////////////////////////////////////////////////////
// Confirm
// -------
// -Asks if user wants to proceed

void Confirm()
{
  char answer;

  printf("\nThis operation will DESTROY everything in the TARGET medium!\n\n");
  printf("Do You want to proceed (y/n) ? ");
  scanf("%c", &answer);

  if ((answer == 'y') || (answer == 'Y'))
    printf("\n");
  else
  {
    printf("Operation aborted!\n\n");
    exit(OK);
  }
}

///////////////////////
// ImageCopy
// ----------
// -Copy image file to another one. Use to copy
// for ex. CD-ROM to HD etc.

int ImageCopy(char *source_file_name, char *target_file_name)
{
  //FILE *source, *target;
  unsigned int i = 0;
  unsigned long source_file_size, target_file_size;
  unsigned char buffer[IMAGE_COPY_BUFFER_BLOCKS * BLOCK_SIZE];

  int source, target;
  unsigned int read_bytes, write_bytes;

  printf("\n");

  // Open source file and get file size
  if ((source = open(source_file_name, O_RDONLY | O_BINARY)) < 0)
  {
    perror("open:");
    EEXIT((stderr, "ERROR: Couldn't open source file '%s'.\n", source_file_name));
  }

  source_file_size = lseek(source, 0, SEEK_END);

  // Check validity of source file
  if (source_file_size == 0)
    EEXIT((stderr, "ERROR: Source file '%s' is empty.\n", source_file_name));
  if ((source_file_size % BLOCK_SIZE) != 0)
  {
    EEXIT((stderr, "Source size have to be multiple of block size %d!\n", BLOCK_SIZE));
    exit(ERR);
  }

  if ((target = open(target_file_name, O_RDWR | O_BINARY)) < 0)
  {
    if ((target = open(target_file_name, O_RDWR | O_CREAT | O_BINARY, S_IRWXU)) < 0)
    {
      perror("open:");
      EEXIT((stderr, "ERROR: Couldn't open target file '%s'.\n", target_file_name));
    }
    //printf("Creating new file '%s'\n", target_file_name);
  }

  target_file_size = lseek(target, 0, SEEK_END);

  if (target_file_size == 0)
  {
    printf("Creating target file %s\n", target_file_name);
  }
  else
  {
    if (source_file_size > target_file_size)
    {
      EEXIT((stderr, "ERROR: Couldn't fit!\nSource '%s' is BIGGER that target '%s'.\n\nSource size: %ld Bytes, target size %ld Bytes\n\n",
             source_file_name, target_file_name, source_file_size, target_file_size));
    }
  }

  //printf("s:%ld,t:%ld\n",source_file_size,target_file_size);

  if (lseek(source, 0, SEEK_SET) < 0)
  {
    perror("seek");
    return (ERR);
  }

  if (lseek(target, 0, SEEK_SET) < 0)
  {
    perror("seek");
    return (ERR);
  }

  // Do copy in big chunks...
  do
  {
    read_bytes = read(source, buffer, IMAGE_COPY_BUFFER_BLOCKS * BLOCK_SIZE);
    if (read_bytes < 0)
    {
      perror("read");
      return (ERR);
    }
    // if last read was "full" read, but get all the data left, next read is 0
    if (read_bytes == 0)
      break;

    printf("\rCopying image file... %d%% completed", ((100 * i++) / ((source_file_size / BLOCK_SIZE) / IMAGE_COPY_BUFFER_BLOCKS)));

    write_bytes = write(target, buffer, read_bytes);
    if (write_bytes < 0)
    {
      perror("write");
      return (ERR);
    }
  } while (read_bytes == (IMAGE_COPY_BUFFER_BLOCKS * BLOCK_SIZE));

  printf("\rImage copy from '%s' to '%s' done! Total %ld Bytes copied. \n", source_file_name, target_file_name, source_file_size);
  fflush(stdout);

  close(source);
  close(target);
  return (OK);
}

//################################################################
// MAIN PROGRAM   ###############################################
//##############################################################

int main(int argc, char **argv)
{
  int in;

  unsigned char Efe[MAX_NUM_OF_DIR_ENTRIES][EFE_SIZE];
  unsigned char *DiskFAT, *DiskHdr, *mem_pointer;
  char DiskLabel[DISK_LABEL_SIZE];

  unsigned int DirPath[MAX_DIR_DEPTH], subdir_cnt;
  unsigned int total_blks, free_blks, fat_blks;
  unsigned int dir_start, dir_cont, start_idx;

  unsigned long i, j;

  char c, media_type, image_type, process_efe[MAX_NUM_OF_DIR_ENTRIES], in_file[FILENAME_MAX];
  char mkdir_name[12], parent_dir_name[12];
  char format_arg;
  FD_HANDLE fd;
  int mode, trk_size, nsect, printmode;
  int check_level, confirm_operation;

  // Initialize variables
  mode = NONE;
  subdir_cnt = 0;
  j = 0;
  image_type = -1;
  printmode = HUMAN_READABLE;
  trk_size = 0;
  media_type = 0;
  fat_blks = 0;
  in = 0;
  DiskFAT = NULL;
  mem_pointer = NULL;
  start_idx = 1;
  confirm_operation = 0;
  strncpy(DiskLabel, DEFAULT_DISK_LABEL, DISK_LABEL_SIZE);
  fd = (FD_HANDLE)NULL;

  for (i = 0; i < MAX_NUM_OF_DIR_ENTRIES; i++)
    process_efe[i] = 0;

  //printf("\n");

  // Get options and parameters
  while (1)
  {

    c = getopt(argc, argv, "JPj:b:srwf:g:p::e:d:m:itc:C::l:qI");
    if (c == -1 || c == 255)
      break;

    switch (c)
    {
    case 'b': // ** Bank Info **
      PrintBankInfo(optarg, printmode);
      printf("%d,%s\n", printmode, optarg);
      exit(OK);
      break;
    case 'P': // ** COPUTER READABLE DIRLIST **
      printmode = COMPUTER_READABLE;
      break;
    case 'J': // ** JSON DIRLIST **
      printmode = JSON;
      break;
    case 's': // ** SPLIT EFE **
      SplitEfe(argv[2], argv[3], argc);
      exit(OK);
      break;
    case 'j': // ** JOIN EFEs **
      JoinEfes(argc, argv);
      exit(OK);
      break;
    case 't': // ** TEST **
      //EEXIT((stderr,"ERROR: No test-modes!\n"));
      mode = TEST;
      break;

    case 'C': // ** CHECK MEDIA **
      //printf("argv[optind=%d]=%s,argc=%d\n",optind,argv[optind],argc);
      GetMedia(argv[optind], argc, &media_type, &image_type, &nsect, &trk_size, &fd, in_file, &in);
      if (optarg == NULL)
        check_level = 0;
      else
        check_level = *optarg - '0';
      CheckMedia(media_type, fd, in, check_level);
      exit(OK);
      break;

    case 'c': // ** CONVERT **
      DoConversion(argv[2], argv[3], argc);
      exit(OK);
      break;

    case 'i': // ** INFO **
      if (FD_GetDiskType(&media_type, &nsect, &trk_size) == ERR)
      {
        EEXIT((stderr, "ERROR: NOT an Ensoniq Disk.\n       Please format the disk (-fe or -fa) and try again!!\n\n"));
      }
      exit(OK);
      break;

    case 'I': // ** IMAGE COPY ***
      if (confirm_operation == 0)
        Confirm();
      //printf("source=%s,target=%s\n",argv[optind],argv[optind+1]);
      ImageCopy(argv[optind], argv[optind + 1]);
      exit(OK);
      break;

    case 'r': // ** READ DISK to IMAGE **
      mode = READ;
      confirm_operation++;
      break;

    case 'w': // ** WRITE DISK from IMAGE **
      mode = WRITE;
      confirm_operation++;
      break;

    case 'f': // ** FORMAT DISK/IMAGE **
      mode = FORMAT;
      confirm_operation++;
      format_arg = optarg[0];
      break;

    case 'l':
      strncpy(DiskLabel, optarg, DISK_LABEL_SIZE);
      break;

    case 'g': // ** Get EFEs **
      ParseEntry(optarg, process_efe);
      mode = GET;
      break;

    case 'p': // ** Put EFEs **
      if (optarg != NULL)
      {
        start_idx = atoi(optarg);
      }
      mode = PUT;
      break;

    case 'e': // ** Erase EFEs **
      ParseEntry(optarg, process_efe);
      mode = ERASE;
      break;

    case 'd': // ** Directory **
      ParseDir(optarg, DirPath, &subdir_cnt);
      //printf("DirPath:%s\n",optarg);
      break;

    case 'm': // ** Make directory **
      mode = MKDIR;
      strncpy(mkdir_name, optarg, 12);
      break;

    case 'q': // Quiet mode //
      confirm_operation--;
      break;

    case '?':
      ShowUsage();
      return (ERR);
      break;

    default:
      printf("DEFAULT\n");
      printf("\n");
      //goto outside;
    }
  }

  //outside:

  if (mode == FORMAT)
  {
    if (confirm_operation)
      Confirm();
    FormatMedia(argv, argc, optind, format_arg, DiskLabel);
  }

  // If other than disk read/write/format, get info etc..

  //if((mode != WRITE) && (mode != READ) && (mode != FORMAT)) {

  if ((mode != WRITE) && (mode != READ))
  {

    // Check if DISK or FILE access and get media parameters
#ifdef DEBUG
    printf("GETMEDIA\n");
    fflush(stdout);
#endif
    GetMedia(argv[optind], argc, &media_type, &image_type,
             &nsect, &trk_size, &fd, in_file, &in);

#ifdef DEBUG
    printf("media_type=%c\n", media_type);
#endif

    // GET DIR & MISC INFO
#ifdef DEBUG
    printf("GETINFO\n");
    fflush(stdout);
#endif
    GetInfo(&media_type, fd, &DiskFAT, &DiskHdr, in, Efe,
            &fat_blks, &free_blks, &total_blks, &dir_start, &dir_cont,
            parent_dir_name, subdir_cnt, DirPath, DiskLabel);
  }

  // Select operation mode (if any)
  // ==============================
  switch (mode)
  {
  case READ: // Read/Wite DISK
  case WRITE:
    if (confirm_operation)
      Confirm();
    FD_RW_Disk(argv[optind], mode);
    exit(OK);

  case GET: // Get EFEs
    GetEFEs(media_type, fd, in, Efe, process_efe, DiskFAT);
    break;

  case PUT: // Put EFEs
#ifdef DEBUG
    printf("PUTEFE\n");
    fflush(stdout);
#endif
    PutEFE(process_efe, start_idx, Efe, media_type, image_type,
           in_file, argv, optind, argv[optind],
           dir_start, dir_cont, total_blks, &free_blks, fat_blks,
           fd, DiskFAT, DiskHdr, NULL, NULL);
    break;

  case ERASE: // Erase EFEs
    EraseEFEs(media_type, image_type, fd, DiskFAT, DiskHdr, in_file, argv[optind],
              Efe, process_efe,
              fat_blks, &free_blks, dir_start, dir_cont);
    break;

  case MKDIR: // Make Dir
    MkDir(process_efe, Efe, media_type, image_type,
          in_file, argv[optind],
          dir_start, dir_cont, parent_dir_name, total_blks,
          &free_blks, fat_blks,
          fd, DiskFAT, DiskHdr, mkdir_name);
    break;

  case TEST:
    printf("\nFAT:\n");
    for (i = 0; i < total_blks; i++)
    {
      switch (GetFatEntry(media_type, DiskFAT, in, i))
      {
      case 0:
        printf(".");
        break;
      case 1:
        printf("E");
        break;
      default:
        printf("#");
      }
    }
    printf("\n");
    exit(0);
  }

  // Close file-pointers if needed
  if (in != 0)
    close(in);

  if (fd != (FD_HANDLE)NULL)
  {
    CloseFloppy(fd);
  }

#ifdef DEBUG
  printf("PRINTDIR\n");
  fflush(stdout);
#endif
  // Print the DirectoryList - Uses 'original' filename (not tmp :-)
  PrintDir(Efe, mode, process_efe, argv[optind], media_type, DiskLabel,
           free_blks, (total_blks - free_blks - fat_blks - 5), printmode);

  exit(OK);
}
