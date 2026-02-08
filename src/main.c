#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-n] -f <database_file> [-a <employee_data>]\n", argv[0]);
  printf("\t -n - create new database file\n");
  printf("\t -f - (required) path to database file\n");
  printf("\t -l - list the employees\n");
  printf("\t -a - (optional) append an employee in format: name,address,hours\n");
  printf("\t      when used with -n, the employee is added to the newly created database;\n");
  printf("\t      otherwise, the employee is appended to the existing database\n");
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  char *addstring = NULL;

  bool newfile = false;
  bool list = false;

  int c;

  int dbfd = -1;

  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  int ret = EXIT_SUCCESS;

  while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
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
      case 'a':
        addstring = optarg;
        break;
      case 'l':
        list = true;
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
       ret = EXIT_FAILURE;
       goto cleanup;
     }

     if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
       printf("Failed to create database header\n");
       ret = EXIT_FAILURE;
       goto cleanup;
     }
  } else {
    // Open read-only if we're only listing (no mutations)
    if (list && !addstring) {
      dbfd = open_db_file_readonly(filepath);
    } else {
      dbfd = open_db_file(filepath);
    }
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      ret = EXIT_FAILURE;
      goto cleanup;
    }

    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      ret = EXIT_FAILURE;
      goto cleanup;
    }
  }

  printf("Newfile: %d\n", newfile);
  printf("Filepath: %s\n", filepath);

  if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
    printf("Failed to read employees\n");
    ret = EXIT_FAILURE;
    goto cleanup;
  }

  if (addstring) {
    size_t new_count = dbhdr->count + 1;
    struct employee_t *tmp = realloc(employees, new_count * sizeof(struct employee_t));
    if (tmp == NULL) {
      printf("Failed to allocate memory for new employee\n");
      ret = EXIT_FAILURE;
      goto cleanup;
    }
    employees = tmp;
    // dbhdr->count is the next available index (0-based) for the new employee
    if (add_employee(dbhdr, employees, addstring, dbhdr->count) == STATUS_ERROR) {
      printf("Failed to add employee\n");
      ret = EXIT_FAILURE;
      goto cleanup;
    }
    dbhdr->count = new_count;
  }

  if (list) {
    list_employees(dbhdr, employees);
  }

  // Only write to file if we created a new file or added an employee
  if (newfile || addstring) {
    if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR) {
      printf("Failed to write database header\n");
      ret = EXIT_FAILURE;
      goto cleanup;
    }
  }

  printf("File size from dbhdr->count: %d\n", dbhdr->count);

cleanup:
  if (dbfd != -1) {
    close(dbfd);
  }
  if (dbhdr != NULL) {
    free(dbhdr);
  }
  if (employees != NULL) {
    free(employees);
  }
  return ret;
}
