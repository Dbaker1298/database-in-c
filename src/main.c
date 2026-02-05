#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

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

  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
      case 'n':
        newfile = true;
        break;
      case 'f':
        filepath = optarg;
        if (filepath[0] == '\0') {
          printf("Filepath cannot be empty\n");
          print_usage(argv);
          return 1;
        }
        break;
      case '?':
        printf("Unknown option -%c\n", optopt);
        print_usage(argv);
        return -1;
      default:
        return -1;
    }
  }

  if (filepath == NULL) {
    printf("Filepath is a required argument\n");
    print_usage(argv);

    return 1;
  }

  printf("Newfile: %d\n", newfile);
  printf("Filepath: %s\n", filepath);

  return 0;
}
