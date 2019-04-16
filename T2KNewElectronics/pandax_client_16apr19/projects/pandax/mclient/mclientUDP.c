/*******************************************************************************

 File:        mclientUDP.c

 Description: A simple UDP client application for testing data acquisition
 of Feminos cards over Fast/Gigabit Ethernet.


 Author:      D. Calvet,        denis.calvetATcea.fr
              

 History:
   May 2011 : copied from T2K version

  April 2013: added -c option to specify the IP address of the local interface
  to be used for communication with the Feminos.

  October 2018: added -arc option to specify that the target is an ARC rather
  than a TDCM.

*******************************************************************************/
#include "platform_spec.h"
#include "os_al.h"
#include "sock_util.h"
#include "timerlib.h"
#include "cmdfetcher.h"
#include "femarray.h"
#include "bufpool.h"
#include "evbuilder.h"

#include <stdio.h>
#include <time.h>

/*******************************************************************************
 Constants types and global variables
*******************************************************************************/
char res_file[80] = {"\0"};
int  save_res     = 0;
int verbose       = 4;
int format_ver    = 2;
int pending_event = 0;
int comp          = 0;
int echo_cmd      = 1;
//int is_arc        = 0;

int cur_fem;


CmdFetcher   cmdfetcher;
FemArray     femarray;
BufPool      bufpool;
EventBuilder eventbuilder;

/*******************************************************************************
 help() to display usage
*******************************************************************************/
void help()
{
	printf("pclient <options>\n");
	printf("   -h               : print this message help\n");
	printf("   -arc             : server is an ARC (by default: TDCM)\n");
	printf("   -s <xx.yy.zz.ww> : base IP address of remote server(s) in dotted decimal\n");
	printf("   -p <port>        : remote UDP target port\n");
	printf("   -S <0xServer_set>: hexadecimal pattern to tell which server(s) to connect to\n");
	printf("   -c <xx.yy.zz.ww> : IP address of the local interface in dotted decimal\n");
	printf("   -f <file>        : read commands from file specified\n");
	printf("   -o <file>        : save results in file specified\n");
	printf("   -v <level>       : verbose\n");
}

/*******************************************************************************
 parse_cmd_args() to parse command line arguments
*******************************************************************************/
int parse_cmd_args(int argc, char **argv)
{
	int i;
	int match;
	int err = 0;

	for (i=1;i<argc; i++)
	{
		match = 0;
		// remote host IP address
		if (strncmp(argv[i], "-s", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				if (sscanf(argv[i],"%d.%d.%d.%d", &(femarray.rem_ip_beg)[0], &(femarray.rem_ip_beg[1]), &(femarray.rem_ip_beg[2]), &(femarray.rem_ip_beg[3])) == 4)
				{

				}
				else
				{
					printf("illegal argument %s\n", argv[i]);
					return (-1);
				}
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// remote port number
		else if (strncmp(argv[i], "-p", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				if (sscanf(argv[i],"%d", &femarray.rem_port) == 1)
				{

				}
				else
				{
					printf("illegal argument %s\n", argv[i]);
					return (-1);
				}
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// Server pattern
		else if (strncmp(argv[i], "-S", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				if (sscanf(argv[i],"0x%x", &femarray.fem_proxy_set) == 1)
				{
					femarray.fem_proxy_set &= 0xFFFFFFFF;
				}
				else
				{
					printf("illegal argument %s\n", argv[i]);
					return (-1);
				}
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// local IP address of the interface
		if (strncmp(argv[i], "-c", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				if (sscanf(argv[i],"%d.%d.%d.%d", &(femarray.loc_ip[0]), &(femarray.loc_ip[1]), &(femarray.loc_ip[2]), &(femarray.loc_ip[3])) == 4)
				{

				}
				else
				{
					printf("illegal argument %s\n", argv[i]);
					return (-1);
				}
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// get file name for commands 
		else if (strncmp(argv[i], "-f", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				strcpy(&(cmdfetcher.cmd_file[0]), argv[i]);
				cmdfetcher.use_stdin = 0;
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// result file name
		else if (strncmp(argv[i], "-o", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				i++;
				strcpy(res_file, argv[i]);
				save_res = 1;
			}
			else
			{
				printf("missing argument after %s\n", argv[i]);
				return (-1);
			}
		}
		// format for saving data
		else if (strncmp(argv[i], "-F", 2) == 0)
		{
			match    = 1;
			if ((i+1) < argc)
			{
				if (sscanf(argv[i+1],"%d", &format_ver) == 1)
				{
					i++;
				}
				else
				{
					format_ver = 2;
				}
			}
			else
			{
				format_ver = 2;
			}
		}
		// verbose
		else if (strncmp(argv[i], "-v", 2) == 0)
		{
			match = 1;
			if ((i+1) < argc)
			{
				if (sscanf(argv[i+1],"%d", &(cmdfetcher.verbose)) == 1)
				{
					i++;
				}
				else
				{
					cmdfetcher.verbose = 1;
				}
			}
			else
			{
				cmdfetcher.verbose = 1;
			}
			femarray.verbose = cmdfetcher.verbose;
			
		}
		// ARC switch
		else if (strncmp(argv[i], "-arc", 4) == 0)
		{
			match  = 1;
			femarray.is_arc = 1;
		}
		// help
		else if (strncmp(argv[i], "-h", 2) == 0)
		{
			match = 1;
			help();
			return (-1); // force an error to exit
		}
		// unmatched options
		if (match == 0)
		{
			printf("Warning: unsupported option %s\n", argv[i]);
		}
	}
	return (0);

}

/*******************************************************************************
 Main
*******************************************************************************/
int main(int argc, char **argv)
{
	int err;

	// Initialize Command Fetcher
	CmdFetcher_Init(&cmdfetcher);
	cmdfetcher.verbose = verbose;

	// Initialize FEM array
	FemArray_Clear(&femarray);

	// Initialize Event Builder
	EventBuilder_Clear(&eventbuilder);

	// Initialize socket library
	if ((err = socket_init()) < 0)
	{
		printf("sock_init failed: %d\n", err);
		return (err);
	}

	// Parse command line arguments
	if (parse_cmd_args(argc, argv) < 0)
	{
		printf("parse_cmd_args failed: %d\n", err);
		return (-1);	
	}

	// Initialize Buffer Pool
	BufPool_Init(&bufpool);

	// Open the array of FEM
	if ((err = FemArray_Open(&femarray)) < 0)
	{
		printf("FemArray_Open failed: %d\n", err);
		goto cleanup;
	}

	// Open the Event Builder
	if ((err = EventBuilder_Open(&eventbuilder)) < 0)
	{
		printf("EventBuilder_Open failed: %d\n", err);
		goto cleanup;
	}
	
	// Pass a pointer to the fem array to the command catcher
	cmdfetcher.fa = (void *) &femarray;
	
	// Pass a pointer to buffer pool and event builder to the fem array
	femarray.bp = (void *) &bufpool;
	femarray.eb = (void *) &eventbuilder;

	// Pass a pointer to the fem array to the event builder
	eventbuilder.fa = (void *) &femarray;

	// Create FEM Array thread
	femarray.thread.routine = (void *) FemArray_ReceiveLoop;
	femarray.thread.param   = (void *) &femarray;
	femarray.state          = 1;
	if( (err = Thread_Create( &femarray.thread )) < 0 )
	{
		printf( "Thread_Create failed %d\n", err );
		goto cleanup;
	}
	//printf("femarray Thread_Create done\n" );

	// Create Event Builder thread
	eventbuilder.thread.routine = (void *) EventBuilder_Loop;
	eventbuilder.thread.param   = (void *) &eventbuilder;
	eventbuilder.state          = 1;
	if( (err = Thread_Create( &eventbuilder.thread )) < 0 )
	{
		printf( "Thread_Create failed %d\n", err );
		goto cleanup;
	}
	//printf("eventbuilder Thread_Create done\n" );

	// Run the main loop of the command interpreter
	CmdFetcher_Main(&cmdfetcher);

	/* wait until FEM array thread stops */
	if(femarray.thread.thread_id >= 0)
	{
		if(Thread_Join(&femarray.thread) < 0)
		{
			printf("femarray: Thread_Join failed.\n");
		}
		else
		{
			printf("femarray: Thread_Join done.\n");
		}
	}

	/* wait until event builder thread stops */
	if(eventbuilder.thread.thread_id >= 0)
	{
		if(Thread_Join(&eventbuilder.thread) < 0)
		{
			printf("eventbuilder: Thread_Join failed.\n");
		}
		else
		{
			printf("eventbuilder: Thread_Join done.\n");
		}
	}

cleanup:

	socket_cleanup();
	return (err);
}


