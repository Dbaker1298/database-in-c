#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database_file>\n", argv[0]);
  printf("\t -n - create new database file\n");
  printf("\t -f - (required) path to database file\n");
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  bool newfile = false;

  int c;

  int dbfd = -1;

  struct dbheader_t *dbhdr = NULL;

  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
      case 'n':
        newfile = true;
        break;
      case 'f':
        filepath = optarg;
        if (filepath == NULL || filepath[0] == '\0') {
          printf("Filepath cannot be empty\n");
          print_usage(argv);
          return EXIT_FAILURE;
        }
        break;
      case '?':
        printf("Unknown option -%c\n", optopt);
        print_usage(argv);
        return EXIT_FAILURE;
      default:
        return EXIT_FAILURE;
    }
  }

  if (filepath == NULL) {
    printf("Filepath is a required argument\n");
    print_usage(argv);

    return EXIT_FAILURE;
  }

  if (newfile) {
     dbfd = create_db_file(filepath);
     if (dbfd == STATUS_ERROR) {
       printf("Unable to create database file\n");
       return EXIT_FAILURE;
     }

     if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
       printf("Failed to create database header\n");
       return EXIT_FAILURE;
     }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return EXIT_FAILURE;
    }

    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return EXIT_FAILURE;
    }
  }

  printf("Newfile: %d\n", newfile);
  printf("Filepath: %s\n", filepath);

  if (output_file(dbfd, dbhdr) == STATUS_ERROR) {
    printf("Failed to write database header\n");
    close(dbfd);
    return EXIT_FAILURE;
  }

  close(dbfd);
  return 0;
}
