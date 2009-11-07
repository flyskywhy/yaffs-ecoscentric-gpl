/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */




#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "yaffsfs.h"

#include "nor_stress.h"
#include "yaffs_fsx.h"

void (*ext_fatal)(void);


int random_seed;
int simulate_power_failure = 0;


int do_fsx;
int init_test;
int do_upgrade;
int n_cycles = -1;

extern int ops_multiplier;

char mount_point[200];

void BadUsage(void)
{
	printf("Usage: yaffs_test [options] mountpoint\n");
	printf("options\n");
	printf(" f: do fsx testing\n");
	printf(" i: initialise for upgrade testing\n");
	printf(" l: multiply number of operations by 5\n");
	printf(" n nnn: number of cycles\n");
	printf(" p: simulate power fail testing\n");
	printf(" s sss: set seed\n");
	printf(" u: do upgrade test\n");
	exit(2);
}

void test_fatal(void)
{
	printf("fatal yaffs test pid %d sleeping\n",getpid());
	while(1)
		sleep(1);
	
}

void bad_ptr_handler(int sig)
{
	printf("signal %d received\n",sig);
	test_fatal();
}
int main(int argc, char **argv)
{
	int ch;
	
	ext_fatal = test_fatal;
	signal(SIGSEGV,bad_ptr_handler);
	signal(SIGBUS,bad_ptr_handler);
	signal(SIGABRT,bad_ptr_handler);
	
	while ((ch = getopt(argc,argv, "filn:ps:u"))
	       != EOF)
		switch (ch) {
		case 's':
			random_seed = atoi(optarg);
			break;
		case 'p':
			simulate_power_failure =1;
			break;
		case 'i':
			init_test = 1;
			break;
		case 'f':
			do_fsx = 1;
			break;
		case 'l':
			ops_multiplier *= 5;
			break;
		case 'u':
			do_upgrade = 1;
			break;
		case 'n':
			n_cycles = atoi(optarg);
			break;
		default:
			BadUsage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	
	if(argc == 1) {
		strcpy(mount_point,argv[0]);
		
		if(simulate_power_failure)
			n_cycles = -1;
		printf("Running test %s %s %s %s seed %d cycles %d\n",
			do_upgrade ? "fw_upgrade" : "",
			init_test ? "initialise":"",
			do_fsx ? "fsx" :"",
			simulate_power_failure ? "power_fail" : "",
			random_seed, n_cycles);

		yaffs_StartUp();
		yaffs_mount(mount_point);
		printf("Mount complete\n");
			
		if(do_upgrade && init_test){
			simulate_power_failure = 0;
			NorStressTestInitialise(mount_point);
		} else if(do_upgrade){
			printf("Running stress on %s with seed %d\n",mount_point,random_seed);
			NorStressTestRun(mount_point,n_cycles,do_fsx);
		} else if(do_fsx){
			yaffs_fsx_main(mount_point,n_cycles);
		}else {
			printf("No test to run!\n");
			BadUsage();
		}
		yaffs_unmount(mount_point);
		
		printf("Test run completed!\n");
	}
	else
		BadUsage();
	return 0;
}


