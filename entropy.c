/*
 * entropy.c
 *
 * Copyright 2012, Stefan Beller, stefanbeller@googlemail.com
 *
 * This file is part of the entropy utility.
 *
 * The output of different lines are piped to some special file descriptors,
 * if given (i.e. if they can be opened).
 * This piping lets you get each line into a bash variable for instance, so using
 * this program within a script might be easier.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define ERRORMESSAGE(output) printf("%s failed in function %s in file %s line %d with:\n%s\n", \
	output,__FUNCTION__,__FILE__,__LINE__, strerror(errno));

#define CATCHERROR(condition, output) { if (condition) { ERRORMESSAGE(output) exit(1);} }

#define BUFFER_SIZE 4096

int readByte[256];
int allReadBytes;

char * buffer;
char * fname;

char optionRecurse = 0;
int symbolsize = 8;

long long unsigned int outputNorm = 8;

void init()
{
	memset( &readByte, 0, 256);
	allReadBytes = 0;

	buffer = (char*) malloc (sizeof(char) * BUFFER_SIZE);
	CATCHERROR ( !buffer, "malloc");
}

void inspectFile(char *fname)
{
	FILE * pFile;
	long lSize;

	size_t result;

	pFile = fopen ( fname , "rb" );
	CATCHERROR( !pFile, "open");

	while (!feof(pFile)) {
		result = fread (buffer, 1, BUFFER_SIZE, pFile);
		CATCHERROR ( ferror(pFile), "fread");

		allReadBytes += result;
		while (result > 0) {
			readByte[(unsigned char)(buffer[result])]++;
			result--;
		}
	}
	// terminate
	fclose (pFile);
}

void displayResult()
{
	int i = 0;
	int count = 0;
	double plogp = 0;
	double entropy = 0;
	double all = allReadBytes;

	for (i = 0; i < 256 ; i++) {
		if (readByte[i]) {
			double p = readByte[i] / all;
			plogp -= p * log(p);
		}
		count+=readByte[i];
	}
	CATCHERROR( count!= allReadBytes, "internal error");

	plogp /= log(2);
	entropy = allReadBytes * plogp; // measured in bit
	printf("%f\n", entropy / outputNorm);
}

void printusage(void)
{
	printf("entropy <filename>\n");
	printf("<filename>\tfile to inspect\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int c;
    static struct option long_options[] = {
        {"output", 1, 0, 'o'},
        {NULL, 0, NULL, 0}
    };
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "ro:s:",
			long_options, &option_index)) != -1) {
        int this_option_optind = optind ? optind : 1;
        switch (c) {
        case 'o':
            if (!strcmp(optarg, "b")) {
				outputNorm = 1;
			} else if (!strcmp(optarg, "B")) {
				outputNorm = (long) 8;
			} else if (!strcmp(optarg, "k")) {
				outputNorm = (long) 8 * 1024;
			} else if (!strcmp(optarg, "m")) {
				outputNorm = (long) 8 * 1024 * 1024;
			} else if (!strcmp(optarg, "g")) {
				outputNorm = (long) 8 * 1024 * 1024 * 1024;
			} else if (!strcmp(optarg, "t")) {
				outputNorm = (long) 8 * 1024 * 1024 * 1024 * 1024;
			} else if (!strcmp(optarg, "K")) {
				outputNorm = (long) 8 * 1000;
			} else if (!strcmp(optarg, "M")) {
				outputNorm = (long) 8 * 1000 * 1000;
			} else if (!strcmp(optarg, "G")) {
				outputNorm = (long) 8 * 1000 * 1000 * 1000;
			} else if (!strcmp(optarg, "T")) {
				outputNorm = (long) 8 * 1000 * 1000 * 1000 * 1000;
			} else {
				printf("no valid output size: %s\n", optarg);
				exit(1);
			}
            break;

        case 's':
			symbolsize = atoi(optarg);

        case 'r':
			optionRecurse = 1;
			break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

    init();

    if (optind == argc)
		printusage();

	while (optind < argc)
		inspectFile(argv[optind++]);

	displayResult();

	free (buffer);
}


