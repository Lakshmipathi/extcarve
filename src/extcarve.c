/*
* /extcarve/extcarve.c-ext2/ext3/ext4 Undelete Tool.
*
* Copyright (C) 2007,2008,2009,2010,2011 Lakshmipathi.G <lakshmipathi.g@gmail.com>
* Visit www.giis.co.in for manuals or docs.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <ext2fs/ext2fs.h>
#include <ext2fs/ext2_io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <assert.h>		/* assert */
#include <libgen.h>
/* argp */
#include <argp.h>


#define o_O(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define BUF_SIZE 1024
#define TRUE 1
#define FALSE 0


struct arguments
{
  int flag;			/* 1-install 2-update 3-recover 4-uninstall */
};

struct extcarve_meta
{
//indicates whether header/footer is found.
  int header_found, footer_found;
  unsigned long header_blk;
  unsigned long footer_blk;
  char dotpart[4];
};

static struct argp_option options[] = {
  {"grepblk", 'd', 0, 0, "[TODO] Dump a block that matches <string>"},
  {"incrblk", 'i', 0, 0,
   "recover files with increase the file/block limit (default 48KB if block size=4KB)"},
  {"recover", 'g', 0, 0, "recover files with default settings."},
  {"salvage", 's', 0, 0, "[TODO] Salvage/Recover partial data too. "},
  {0}
};

int EXT2_BLOCK_SIZE;
int DIRECT_BLKS;
const char *argp_program_version = "extcarve 0.3 (10-Jun-2011) ";
const char *argp_program_bug_address =
  "<http://groups.google.com/group/giis-users>";


/* Functions involved. */
static error_t parse_opt (int, char *, struct argp_state *);
void do_dump_unused ();

static char args_doc[] = "";
static char doc[] =
  "extcarve - An undelete tool for ext2/ext3/ext4FS.(http://www.giis.co.in)";
static struct argp argp = { options, parse_opt, args_doc, doc };


static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'i':
      arguments->flag = 1;
      break;
    case 'd':
      arguments->flag = 2;
      break;
    case 'g':
      arguments->flag = 3;
      break;
    case 's':
      arguments->flag = 4;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

char restore_device_dir[100];

int
main (int argc, char *argv[])
{
  ext2_filsys current_fs = NULL;
  char device[75];
  int open_flags = EXT2_FLAG_SOFTSUPP_FEATURES;	//| EXT2_FLAG_RW;
  blk_t superblock = 0;
  blk_t blocksize = 0;
  int retval = 0;
  int i = 0;
  int ans = 0;
  struct arguments arguments;
  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  if (!(arguments.flag > 0 && arguments.flag < 6))
    o_O ("For usage type : extcarve --help");
	if(arguments.flag == 2 || arguments.flag == 4 ){
		o_O("TODO : will be implemented in next version");
	}

  printf ("\nPlease enter the device name:");
  scanf ("%s", device);
  printf
    ("\n Please enter the output directory - (You must provide  separate partition/drive\'s directory :");
  scanf ("%s", restore_device_dir);

  retval =
    ext2fs_open (device, open_flags, superblock, blocksize, unix_io_manager,
		 &current_fs);
  if (retval)
    {
      current_fs = NULL;
      o_O ("Error while opening filesystem.");
    }

  retval = ext2fs_read_inode_bitmap (current_fs);
  if (retval)
    {
      o_O ("Error while reading inode bitmap");
    }

  retval = ext2fs_read_block_bitmap (current_fs);
  if (retval)
    {
      o_O ("Error while reading block bitmap");
    }


  EXT2_BLOCK_SIZE = current_fs->blocksize;

  if (arguments.flag == 1)
    {
      printf ("Please enter increased block limit:(default is 12) :");
      scanf ("%d", &DIRECT_BLKS);
      printf ("\nIncreased blk size from 12 to %d", DIRECT_BLKS);
      do_dump_unused (current_fs);
    }
  if (arguments.flag == 3)
    {
      do_dump_unused (current_fs);
    }
	
  ext2fs_close (current_fs);
  return 1;
}


/*
 * unused.c --- quick and dirty unused space dumper
 *
 * Copyright (C) 1997 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 */
//void do_dump_unused(int argc EXT2FS_ATTR((unused)), char **argv)
void
do_dump_unused (ext2_filsys current_fs)
{
  unsigned long blk, last_searched_blk;
  unsigned char buf[EXT2_BLOCK_SIZE];
  unsigned int i;
  errcode_t retval, f_retval;
  struct extcarve_meta needle = { 0 };

/*	if (common_args_process(argc, argv, 1, 1,
				"dump_unused", "", 0))
		return;*/

  for (blk = current_fs->super->s_first_data_block;
       blk < current_fs->super->s_blocks_count; blk++)
    {
      if (ext2fs_test_block_bitmap (current_fs->block_map, blk))
	continue;
      retval = io_channel_read_blk (current_fs->io, blk, 1, buf);
      if (retval)
	{
	  //      com_err(argv[0], retval, "While reading block\n");
	  o_O ("Error while reading block");
	  return;
	}
      for (i = 0; i < current_fs->blocksize; i++)
	if (buf[i])
	  break;
      //      if (i >= current_fs->blocksize){
      //              printf("\nUnused block %lu contains zero data:\n\n",blk);
      //              continue;
      //              }

      printf
	("\nSearching Unused block %lu which contains non-zero data:\n\n",
	 blk);

      if ((blk - last_searched_blk) != 1)
	{
	  //some blks are skipped b'cause they are in-use or empty.so reset.
	  memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	}

      retval = extcarve_search4header (buf, &needle, blk);
      last_searched_blk = blk;

      if (retval == 1 || needle.header_found == 1)
	{			//fresh header
	  f_retval = extcarve_search4footer (buf, &needle, blk);
	  if (f_retval == -1)
	    {
	      //footer mismatch.reset.
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else if (f_retval == 1)
	    {
	      printf ("perfect match.recover from %u to %u",
		      needle.header_blk, needle.footer_blk);
	      //carve_this
	      extcarve_write_to_fd (current_fs, &needle);
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else
	    printf ("No footer at all");

	  if ((blk - needle.header_blk) >= DIRECT_BLKS)
	    {
	      //unable to find footer in DIRECT_BLKS blks so reset
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }

	  //for (i=0; i < current_fs->blocksize; i++)
	  //      fputc(buf[i], stdout);
	}
      else if (retval == -1)
	{
	  //header found again. reset.
	  memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	}
      else
	printf ("no header found.at all");
    }
}

int
extcarve_write_to_fd (ext2_filsys current_fs, struct extcarve_meta *needle)
{
  unsigned long blk;
  unsigned char buf[EXT2_BLOCK_SIZE];
  unsigned int i;
  errcode_t retval, f_retval;

  char fname[] = "extcarveXXXXXX";
  char tmp_dname[100];
  int temfd;

  /* The trick to get a unique name using mkstemp and unlink the temp.file :-)and then use it. */
  temfd = mkstemp (fname);
  close (temfd);
  unlink (fname);
  strcpy (tmp_dname, restore_device_dir);

  strcat (tmp_dname, "/");
  strcat (tmp_dname, fname);
  strcat (tmp_dname, needle->dotpart);

  temfd = creat (tmp_dname, 0700);
  if (temfd < 0)
    {
      printf
	("Please check permission for destination directory %s.Something wrong",
	 restore_device_dir);
      exit (0);
    }

  //printf("\nRestoring files dir : %s %u %u",restore_device_dir,needle->header_blk,needle->footer_blk);

  for (blk = needle->header_blk; blk <= needle->footer_blk; blk++)
    {
      retval = io_channel_read_blk (current_fs->io, blk, 1, buf);
      if (retval)
	{
	  //      com_err(argv[0], retval, "While reading block\n");
	  o_O ("Error while reading block");
	  return;
	}
      write (temfd, buf, EXT2_BLOCK_SIZE);
    }
  close (temfd);
  return 1;
}

int
extcarve_search4header (unsigned char buf[EXT2_BLOCK_SIZE],
			struct extcarve_meta *needle, unsigned long blk)
{
  //read magic file signature and determine file type of given block
  //gif 
  if (buf[0] == 0x47 && buf[1] == 0x49 && buf[2] == 0x46 && buf[3] == 0x38)
    {
      //printf ("gif header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".gif");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  //png 
  if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47
      && buf[4] == 0x0D && buf[5] == 0x0A && buf[6] == 0x1A && buf[7] == 0x0A)
    {
      //printf ("png header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".png");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }
  //jpg -
  if ((buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF && buf[3] == 0xE1)
      || (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF
	  && buf[3] == 0xE0))
    {
      //printf ("jpg header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".jpg");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }
  //pdf 
  if ((buf[0] == 0x25 && buf[1] == 0x50 && buf[2] == 0x44 && buf[3] == 0x46
       && buf[4] == 0x2D && buf[5] == 0x31 && buf[6] == 0x2E))
    {
      //printf ("pdf header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".pdf");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }
  //C/CPP programs
  if (memcmp (buf, "#include", 8) == 0)
    {
      //printf ("C/C++  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".cpp");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }
  //php
  if (memcmp (buf, "<?php>", 6) == 0)
    {
      //printf ("php  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".php");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  return 0;
}

int
extcarve_search4footer (unsigned char buf[EXT2_BLOCK_SIZE],
			struct extcarve_meta *needle, unsigned long blk)
{
  int i = 0;
  while (i <= EXT2_BLOCK_SIZE - 1)
    {
      if (buf[i] == 0x00 && buf[i + 1] == 0x3b)
	{
	  if (strcmp (needle->dotpart, ".gif") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}
      //png

      if (buf[i] == 0x00 && buf[i + 1] == 0x00 && buf[i + 2] == 0x49
	  && buf[i + 3] == 0x45 && buf[i + 4] == 0x4e && buf[i + 5] == 0x44
	  && buf[i + 6] == 0xae && buf[i + 7] == 0x42 && buf[i + 8] == 0x60
	  && buf[i + 9] == 0x82)
	{
	  if (strcmp (needle->dotpart, ".png") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}
      //jpg
      if (buf[i] == 0xFF && buf[i + 1] == 0xD9)
	{
	  if (strcmp (needle->dotpart, ".jpg") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}
      //pdf 
      if (buf[i] == 0x25 && buf[i + 1] == 0x25 && buf[i + 2] == 0x45
	  && buf[i + 3] == 0x4F && buf[i + 4] == 0x46 && buf[i + 5] == 0x0A)
	{
	  if (strcmp (needle->dotpart, ".pdf") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}
      //C/C++ 
      if ((memcmp (&buf[i], "return", 6) == 0))
	{
	  if (strcmp (needle->dotpart, ".cpp") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}
      //php 
      if ((memcmp (&buf[i], "</php>", 6) == 0))
	{
	  if (strcmp (needle->dotpart, ".php") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      return 1;
	    }
	  else
	    return -1;
	}

      i++;
    }
  return 0;
}
