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
  int header_found, footer_found, footer_offset;
  unsigned long header_blk;
  unsigned long footer_blk;
  char dotpart[4];
};
struct node {
	unsigned long headerblk;
	unsigned long footerblk;
	int footer_offset;
	struct node* next;
	char dotpart[4];  
	};

struct node *head=NULL;

static struct argp_option options[] = {
  {"grepblk", 'd', 0, 0, "[TODO] Dump a block that matches <magic-string>."},
  {"incrblk", 'i', 0, 0,
   "recover files with increase the file/block limit (default 48KB if block size=4KB)."},
  {"recover", 'g', 0, 0, "recover files with default settings."},
  {"salvage", 's', 0, 0, "[TODO] Salvage/Recover partial data too. "},
  {"type", 't', 0, 0, "Recover file by type."},
  {"fileimage", 'f', 0, 0, "Recover from file image."},
  {"analyze", 'a', 0, 0, "Analyze the disk - dont recover."},
  {0}
};
char FILE_TYPE[5];
int use_file_fmt;
int EXT2_BLOCK_SIZE;
int DIRECT_BLKS = 11;
int analyze_mode=0;

const char *argp_program_version = "extcarve 1.2 (12-Oct-2011) ";
const char *argp_program_bug_address =
  "<http://groups.google.com/group/giis-users>";


/* Functions involved. */
static error_t parse_opt (int, char *, struct argp_state *);
void do_dump_unused ();
void do_check_fs (char *);

static char args_doc[] = "";
static char doc[] =
  "extcarve - An undelete tool for ext2/ext3/ext4FS.(http://www.giis.co.in)";
static struct argp argp = { options, parse_opt, args_doc, doc };

int
extcarve_search4footer (unsigned char buf[EXT2_BLOCK_SIZE],
			struct extcarve_meta *needle, unsigned long blk);
int
extcarve_search4header (unsigned char buf[EXT2_BLOCK_SIZE],
			struct extcarve_meta *needle, unsigned long blk);

void push(struct node**,struct extcarve_meta*);
void printlist(struct node*);
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
    case 't':
      arguments->flag = 5;
      break;
    case 'f':
      arguments->flag = 6;
      break;
    case 'a':
      arguments->flag = 7;
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
  if (!(arguments.flag > 0 && arguments.flag < 8))
    o_O ("For usage type : extcarve --help");
  if (arguments.flag == 2 || arguments.flag == 4)
    {
      o_O ("TODO : will be implemented in next version");
    }
  if(arguments.flag==7){
	analyze_mode=1;
	
	}

  printf ("\nPlease enter the device name:");
  scanf ("%s", device);
  printf
    ("\nPlease enter the output directory - (You must provide  separate partition/drive\'s directory) :");
  scanf ("%s", restore_device_dir);

  if (arguments.flag == 6 ){
    
	printf ("Enter the File System block size :");
	scanf("%d",&EXT2_BLOCK_SIZE);

        printf("\n Get by file format? Press 1 else 0:");
	scanf("%d",&use_file_fmt);
	if(use_file_fmt){
        printf ("Please enter the file type :(gif/jpg/pdf/tex/txt/tgz/zip/htm/cpp/php):");
        scanf ("%s", FILE_TYPE);
	}


	do_check_fs(device);
        return 1;	
	}

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

  if (arguments.flag == 1 )
    {
      printf ("Please enter increased block limit:(default is 12) :");
      scanf ("%d", &DIRECT_BLKS);
      printf ("\nIncreased blk size from 12 to %d", DIRECT_BLKS);
      do_dump_unused (current_fs);
    }
  if (arguments.flag == 3  || arguments.flag == 7)
    {
      do_dump_unused (current_fs);
    }
  if (arguments.flag == 5)
    {
      use_file_fmt=1;
      printf ("Please enter the file type :(gif/jpg/pdf/tex/txt/tgz/zip/htm/cpp/php):");
      scanf ("%s", FILE_TYPE);
      printf ("\n Searching for file type %s", FILE_TYPE);
      do_dump_unused (current_fs);
    }

  ext2fs_close (current_fs);
  if(analyze_mode)
	printlist(head); //display the list
  printf ("\n extcarve is completed");
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
  unsigned long blk, last_searched_blk, blk_cnt = 1;
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

      blk_cnt++;
      //cleanup buf before read
      memset (buf, '\0', EXT2_BLOCK_SIZE);
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
	("\rSearching non-zero unused block : %16lu  Analyzed: %16lu of %16lu",
	 blk, blk_cnt, current_fs->super->s_free_blocks_count);

      if ((blk - last_searched_blk) != 1)
	{
	  //printf("\nsome blks are skipped b'cause they are in-use or empty.so reset.");
	  memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	}

      retval = extcarve_search4header (buf, &needle, blk);
      last_searched_blk = blk;

	//When recover by file format , If found dotpart is different than FILE_TYPE ,then reset.
	if (use_file_fmt && (strcmp (needle.dotpart, FILE_TYPE) != 0)){
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
		retval=0;
	} 

      if (retval == 1 || needle.header_found == 1)
	{			//fresh header
	  f_retval = extcarve_search4footer (buf, &needle, blk);
	  if (f_retval == -1)
	    {
	      //printf("\nfooter mismatch.reset.");
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else if (f_retval == 1)
	    {
	      //printf ("\nperfect match.recover from %u to %u",needle.header_blk, needle.footer_blk);
	      //carve_this
	      extcarve_write_to_fd (current_fs, &needle);
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else;			//printf ("\nNo footer at all");

	  if ((blk - needle.header_blk) >= DIRECT_BLKS)
	    {
	      // printf("\nunable to find footer in DIRECT_BLKS blks so reset");
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }

	  //for (i=0; i < current_fs->blocksize; i++)
	  //      fputc(buf[i], stdout);
	}
      else if (retval == -1)
	{
	  //printf("\nheader found again. reset.");
	  memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	}
      else  ; //printf("no header at all");
		
	
    }

  printf ("\n Scanning completed");
}

int
extcarve_write_to_fd (ext2_filsys current_fs, struct extcarve_meta *needle)
{
  unsigned long blk,f_size=0;
  unsigned char buf[EXT2_BLOCK_SIZE];
  unsigned int i;
  errcode_t retval, f_retval;

  char fname[] = "extcarveXXXXXX";
  char tmp_dname[100];
  int temfd;
  if(!analyze_mode){
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
	}
	
      if(blk!=needle->footer_blk)
	f_size+=EXT2_BLOCK_SIZE;
      write (temfd, buf, EXT2_BLOCK_SIZE);
    }
  close (temfd);

  //Now truncate file to its size,for _those_ format where we got footer-
   if(needle->footer_offset!=-1)
   if(truncate(tmp_dname,f_size+needle->footer_offset)!=0)
	o_O("Error while fixing size");
  }
  //push details into the list
  push(&head,needle);

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
       && buf[4] == 0x2D))
    {
      //printf ("pdf header found!!!%lu",blk);
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
  //Latex
  if ((strcasestr (buf, "\\documentclass") != NULL)
      || (strcasestr (buf, "\\input") != NULL)
      || (strcasestr (buf, "\\section") != NULL)
      || (strcasestr (buf, "\\setlenth") != NULL)
      || (strcasestr (buf, "\\relax") != NULL)
      || (strcasestr (buf, "\\contentsline") != NULL))
    {

      //printf ("Latex  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".tex");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  //MATLAB .fig 
  if (memcmp (buf, "MATLAB", 6) == 0)
    {
      //printf ("fig  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".fig");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  //gzip .tgz file
  if ((buf[0] == 0x1f && buf[1] == 0x8b))
    {
      //printf ("tgz  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".tgz");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  //zip
  if ((buf[0] == 0x50 && buf[1] == 0x4B && buf[2] == 0x03 && buf[3] == 0x04 ))
    {
      //printf ("zip header found!!!%lu",blk);
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".zip");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

  //bzip2 .bz2 file
  if ((buf[0] == 0x42 && buf[1] == 0x5a && buf[2] == 0x68 && buf[3] == 0x39 && buf[4] == 0x31 && buf[5] == 0x41 && buf[6] == 0x59 && buf[7] == 0x26))
    {
      //printf ("bz2  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".bz2");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }

 //rpm file
  if ((buf[0] == 0xed && buf[1] == 0xab && buf[2] == 0xee && buf[3] == 0xdb ))
    {
      //printf ("rpm  header found!!!");
      if (needle->header_found != 1)
	{
	  needle->header_found = 1;
	  needle->header_blk = blk;
	  strcpy (needle->dotpart, ".rpm");
	  return 1;
	}
      needle->header_found = -1;
      return -1;
    }
  //Please add new file type header here


  //No header matched - check whether its ascii file
	  
	if ( extcarve_is_empty(buf) > 0 && needle->header_found != 1 && extcarve_is_ascii(buf)>0){
		  needle->header_found = 1;
		  needle->header_blk = blk;
		  strcpy (needle->dotpart, ".txt");
		  return 1;
	}

  return 0;
}

int
extcarve_search4footer (unsigned char buf[EXT2_BLOCK_SIZE],
			struct extcarve_meta *needle, unsigned long blk)
{
  int i = 0;
	//If empty block found -return 
	if ( (strcmp (needle->dotpart, ".txt") == 0) && extcarve_is_empty(buf) < 0 ){
	  needle->footer_blk = blk;
	  needle->footer_found = 1;
	  needle->footer_offset=-1;
	  return 1;
	}
// MATLAB like files - which has no footer - Search till EOF . If EOF reached return 1
  if ((strcmp (needle->dotpart, ".fig") == 0) || (strcmp (needle->dotpart, ".tgz") == 0) || (strcmp (needle->dotpart, ".txt") == 0) || (strcmp (needle->dotpart, ".bz2") == 0) || (strcmp (needle->dotpart, ".rpm") == 0))
    {

      if ((extcarve_is_EOF (-8, buf) == 0)
	  || (extcarve_is_EOF (256, buf) == 0)
	  || (extcarve_is_EOF (512, buf) == 0)
	  || (extcarve_is_EOF (1024, buf) == 0)
	  || (extcarve_is_EOF (2048, buf) == 0)
	  || (extcarve_is_EOF (3072, buf) == 0)
	  || (extcarve_is_EOF (3136, buf) == 0)
	  || (extcarve_is_EOF (3200, buf) == 0)
	  || (extcarve_is_EOF (3328, buf) == 0)
	  || (extcarve_is_EOF (3586, buf) == 0)
	  || (extcarve_is_EOF (3840, buf) == 0))
	{
	  needle->footer_blk = blk;
	  needle->footer_found = 1;
	  needle->footer_offset=-1;
	  return 1;
	}
      return 0;
    }

  while (i <= EXT2_BLOCK_SIZE - 1)
    {
      if (buf[i] == 0x00 && buf[i + 1] == 0x3b && buf[i + 2] == '\0')
	{
	  if (extcarve_is_EOF (i, buf) == 0)
	    {
	      if (strcmp (needle->dotpart, ".gif") == 0)
		{
		  needle->footer_blk = blk;
		  needle->footer_found = 1;
		  needle->footer_offset=i+2;
		  return 1;
		}
	      else
		return -1;
	    }
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
	      needle->footer_offset=i+10;
	      return 1;
	    }
	  else
	    return -1;
	}
      //jpg
      if (buf[i] == 0xFF && buf[i + 1] == 0xD9)
	{
	  if (extcarve_is_EOF (i, buf) == 0)
	    {
	      if (strcmp (needle->dotpart, ".jpg") == 0)
		{
		  needle->footer_blk = blk;
		  needle->footer_found = 1;
		  needle->footer_offset=i+1;
		  return 1;
		}
	      else
		return -1;

	    }
	}
      //pdf 
      if (buf[i] == 0x25 && buf[i + 1] == 0x25 && buf[i + 2] == 0x45
	  && buf[i + 3] == 0x4F && buf[i + 4] == 0x46)
	{
	  if (extcarve_is_EOF (i, buf) == 0)
	    {
	      if (strcmp (needle->dotpart, ".pdf") == 0)
		{
		  needle->footer_blk = blk;
		  needle->footer_found = 1;
		  needle->footer_offset=i+4;
		  return 1;
		}
	      else
		return -1;
	    }
	}
      //C/C++ 
      if ((memcmp (&buf[i], "return", 6) == 0))
	{
	  if (strcmp (needle->dotpart, ".cpp") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      needle->footer_offset=i+6;
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
	      needle->footer_offset=i+6;
	      return 1;
	    }
	  else
	    return -1;
	}
      //tex 
      if ((strcasestr (buf, "\\end") != NULL))
	{
	  if (strcmp (needle->dotpart, ".tex") == 0)
	    {
	      needle->footer_blk = blk;
	      needle->footer_found = 1;
	      needle->footer_offset=-1;
	      return 1;
	    }
	  else
	    return -1;
	}

      //zip 
      if (buf[i] == 0x4b && buf[i + 1] == 0x05 && buf[i + 2] == 0x06
	  && buf[i + 3] == 0x00)
	{
	  if (extcarve_is_EOF (i, buf) == 0)
	    {
	      if (strcmp (needle->dotpart, ".zip") == 0)
		{
		  needle->footer_blk = blk;
		  needle->footer_found = 1;
		  needle->footer_offset=i+4;
		  return 1;
		}
	      else
		return -1;
	    }
	}

      //Please add new file type footer here
      i++;
    }
  return 0;
}

int
extcarve_is_EOF (int i, char buf[EXT2_BLOCK_SIZE])
{
  char newbuf[EXT2_BLOCK_SIZE];
  char tmpbuf[EXT2_BLOCK_SIZE];
  int n = 0, m = 0, j = 0, k = 0, ret = -1;

  memset (&newbuf, '\0', EXT2_BLOCK_SIZE);
  memset (&tmpbuf, '\0', EXT2_BLOCK_SIZE);

  k = i;
  m = k + 8;			//m position
  n = EXT2_BLOCK_SIZE - 4 - m;			//n chars 

  j = 0;
//      printf("From %d to %d",n,m-1);
  for (k = m - 1; j < n; k++)
    newbuf[j++] = buf[k];

  newbuf[j] = '\0';

  ret = memcmp (tmpbuf, newbuf, n);
  if (ret == 0)
    return 0;
  else
    return 1;
}

int extcarve_is_ascii(char buf[EXT2_BLOCK_SIZE])
{
int i=0;
while (i<EXT2_BLOCK_SIZE){
if(isascii(buf[i++])>0)
continue;
else
return -1;
}
return 1;
}

//check whether given block is empty or not
int extcarve_is_empty(char buf[EXT2_BLOCK_SIZE])
{
int i=0,ret=0;

      for (i = 0; i < EXT2_BLOCK_SIZE; i++)
	if (buf[i]==0x0 || isblank(buf[i])==0 || iscntrl(buf[i])==0 )
	  continue;
	else
	   break;

        if (i < EXT2_BLOCK_SIZE){
  		   return 1;
                    }

    return -1;
}
//modified to scan all file images
void
do_check_fs(char *device)
{
  unsigned long blk, last_searched_blk, blk_cnt = 0;
  unsigned char buf[EXT2_BLOCK_SIZE];
  unsigned int i;
  errcode_t retval, f_retval;
  struct extcarve_meta needle = { 0 };
  int fp=0,read_bytes=0;

  fp=open(device,0);
  if(fp < 0 ){
	o_O("Unable to open");
   }
  //cleanup buf before read
  memset (buf, '\0', EXT2_BLOCK_SIZE);

  read_bytes=read(fp,&buf,EXT2_BLOCK_SIZE);
  while (read_bytes > 0){
      blk_cnt++;
      blk=blk_cnt;
      retval = extcarve_search4header (buf, &needle, blk);
	//When recover by file format , If found dotpart is different than FILE_TYPE ,then reset.
	if (use_file_fmt && (strcmp (needle.dotpart, FILE_TYPE) != 0)){
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
		retval=0;
	} 

      if (retval == 1 || needle.header_found == 1)
	{			//fresh header
	  f_retval = extcarve_search4footer (buf, &needle, blk);
	  if (f_retval == -1)
	    {
	      //printf("\nfooter mismatch.reset.");
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else if (f_retval == 1)
	    {
	//      printf ("\nperfect match.recover from %u to %u",needle.header_blk, needle.footer_blk);    //carve_this
	      extcarve_write_to_fd2 (fp,&needle);
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	  else ;		//printf ("\nNo footer at all");
	  if ((blk - needle.header_blk) >= DIRECT_BLKS)
	    {
	      // printf("\nunable to find footer in DIRECT_BLKS blks so reset");
	      memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	    }
	}
      else if (retval == -1)
	{
	  //printf("\nheader found again. reset.");
	  memset ((void *) &needle, '\0', sizeof (struct extcarve_meta));
	}
      else  ; //printf("no header at all");
      memset (buf, '\0', EXT2_BLOCK_SIZE); //cleanup buf before read
      read_bytes=read(fp,&buf,EXT2_BLOCK_SIZE);
     }
  close(fp);
  printf ("\n Scanning completed");
}


int
extcarve_write_to_fd2 (int fp, struct extcarve_meta *needle)
{
  unsigned long blk,f_size=0;
  unsigned char buf[EXT2_BLOCK_SIZE];
  unsigned int i,count=0;
  errcode_t retval, f_retval;

  char fname[] = "extcarveXXXXXX";
  char tmp_dname[100];
  int temfd;

  if(!analyze_mode){
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

  printf("\nRestoring files dir : %s %u %u",restore_device_dir,needle->header_blk,needle->footer_blk);

  count = (needle->footer_blk - needle->header_blk)+1; //no.of blocks to read

	//lseek
  if (lseek64(fp,EXT2_BLOCK_SIZE*needle->header_blk,0)<0){
	  o_O ("Error during lseek.");
	}

	while(count > 0){
	//read from offset
 	retval=read(fp,buf,EXT2_BLOCK_SIZE);
	      if (retval < 0)
		{
		  o_O ("Error while reading block");
		}
	
      		if(count > 1)
		f_size+=EXT2_BLOCK_SIZE;

      		write (temfd, buf, EXT2_BLOCK_SIZE);
		count--;
    	}

  close (temfd);
  //Now truncate file to its size,for _those_ format where we got footer-
   if(needle->footer_offset!=-1)
   if(truncate(tmp_dname,f_size+needle->footer_offset)!=0)
	o_O("Error while fixing size");
	}
  //push details into the list
  push(&head,needle);
}

//push data into the list

void push(struct node** headref,struct extcarve_meta* needle){

		struct node* newnode=malloc(sizeof(struct node));

		newnode->headerblk=needle->header_blk;
		newnode->footerblk=needle->footer_blk;
		newnode->footer_offset=needle->footer_offset;
		strcpy(newnode->dotpart,needle->dotpart);

		newnode->next=*headref;
	
		*headref=newnode;
}
//print the list
void printlist(struct node *current){
char tmp_dname[100];
int fd;

	  strcpy (tmp_dname, restore_device_dir);
	  strcat (tmp_dname, "/");
	  strcat (tmp_dname, "extcarve_analyze.txt");
	  fd=fopen(tmp_dname,"w+");
		
	while(current!=NULL){
	fprintf(fd,"\n Header blk :%u",current->headerblk);
	fprintf(fd,"\n Footer blk :%u",current->footerblk);
	fprintf(fd,"\n Footer offset:%d",current->footer_offset);
	fprintf(fd,"\n File Type: %s\n",current->dotpart);
	current=current->next;
	}
}
