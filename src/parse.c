#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
  if (addstring == NULL) {
    printf("add_employee called with NULL addstring\n");
    return STATUS_ERROR;
  }

  if (dbhdr == NULL || employees == NULL) {
    printf("add_employee called with NULL dbhdr or employees\n");
    return STATUS_ERROR;
  }

  if (dbhdr->count == 0) {
    printf("add_employee called with count of 0\n");
    return STATUS_ERROR;
  }

  printf("Did we get the string?: %s\n", addstring);

  /* Work on a local copy so we don't modify the caller's buffer */
  char *input_copy = strdup(addstring);
  if (input_copy == NULL) {
    printf("Failed to allocate memory for input copy\n");
    return STATUS_ERROR;
  }

  char *name = strtok(input_copy, ",");
  char *addr = strtok(NULL, ",");
  char *hours = strtok(NULL, ",");

  if (name == NULL || addr == NULL || hours == NULL) {
    printf("Invalid employee string, expected 3 comma-separated fields: '%s'\n", addstring);
    free(input_copy);
    return STATUS_ERROR;
  }

  printf("Verifying name, addr, hours: %s %s %s\n", name, addr, hours);

  snprintf(employees[dbhdr->count-1].name, sizeof(employees[dbhdr->count-1].name), "%s", name);
  snprintf(employees[dbhdr->count-1].address, sizeof(employees[dbhdr->count-1].address), "%s", addr);
  employees[dbhdr->count-1].hours = atoi(hours);

  free(input_copy);

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("calloc failed to create the db header\n");
    return STATUS_ERROR;
  }

    read(fd, employees, count*sizeof(struct employee_t));

    int i = 0;
    for  (; i < count; i++) {
      employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int realcount = dbhdr->count;

  // Work with a copy to avoid modifying the caller's structure
  struct dbheader_t header_copy = *dbhdr;
  header_copy.magic = htonl(header_copy.magic);
  header_copy.filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
  header_copy.count = htons(header_copy.count);
  header_copy.version = htons(header_copy.version);

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("lseek");
    return STATUS_ERROR;
  }

  if (write(fd, &header_copy, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("write");
    return STATUS_ERROR;
  }

  int i = 0;
  for (; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  *headerOut = NULL;

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("calloc failed to create the db header\n");
    return STATUS_ERROR;
  }

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("lseek");
    free(header);
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
   perror("read");
   free(header);
   return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic value\n");
    free(header);
    return STATUS_ERROR;
  } 
   
  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};
  if (fstat(fd, &dbstat) == -1) {
    perror("fstat");
    free(header);
    return STATUS_ERROR;
  }
  if (header->filesize != dbstat.st_size) {
    printf("Corrupted database!\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;
  return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("calloc failed to create the db header\n");
    return STATUS_ERROR;
  }

  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
