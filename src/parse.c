#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  int i = 0;
  for (; i < dbhdr->count; i++) {
    printf("Employee ID: %d\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours: %u\n", employees[i].hours);
    printf("\n");
  }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring, unsigned int index) {
  if (addstring == NULL) {
    printf("add_employee called with NULL addstring\n");
    return STATUS_ERROR;
  }

  if (dbhdr == NULL || employees == NULL) {
    printf("add_employee called with NULL dbhdr or employees\n");
    return STATUS_ERROR;
  }

  /* Work on a local copy so we don't modify the caller's buffer */
  char *input_copy = strdup(addstring);
  if (input_copy == NULL) {
    printf("Failed to allocate memory for input copy\n");
    return STATUS_ERROR;
  }

  // strtok: extract tokens from strings
  char *name = strtok(input_copy, ",");
  char *addr = strtok(NULL, ",");
  char *hours = strtok(NULL, ",");

  if (name == NULL || addr == NULL || hours == NULL) {
    printf("Invalid employee string, expected 3 comma-separated fields: '%s'\n", addstring);
    free(input_copy);
    return STATUS_ERROR;
  }

  printf("[Verifying add_employee] name, addr, hours: %s | %s | %s\n", name, addr, hours);

  // snprintf() writes at most `size` bytes (including the terminating null byte ('\0')) to str.
  snprintf(employees[index].name, sizeof(employees[index].name), "%s", name);
  snprintf(employees[index].address, sizeof(employees[index].address), "%s", addr);
  
  /* Use strtoul for better error handling and validation */
  // strtoul(): convert a string to an unsigned long integer
  char *endptr;
  unsigned long hours_val = strtoul(hours, &endptr, 10);
  
  /* Check for conversion errors */
  if (endptr == hours || *endptr != '\0') {
    printf("Invalid hours value: '%s' is not a valid number\n", hours);
    free(input_copy);
    return STATUS_ERROR;
  }
  
  /* Check for overflow (hours_val should fit in unsigned int) */
  if (hours_val > UINT_MAX) {
    printf("Hours value %lu exceeds maximum allowed value\n", hours_val);
    free(input_copy);
    return STATUS_ERROR;
  }
  
  employees[index].hours = (unsigned int)hours_val;

  free(input_copy);

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  /* Reject negative counts and counts that would overflow a size_t multiplication */
  if (count < 0 || (size_t)count > SIZE_MAX / sizeof(struct employee_t)) {
    printf("Employee count too large or invalid, would cause integer overflow\n");
    return STATUS_ERROR;
  }

  /* Handle zero employees explicitly: no allocation needed */
  if (count == 0) {
    *employeesOut = NULL;
    return STATUS_SUCCESS;
  }

  struct employee_t *employees = calloc((size_t)count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("Failed to allocate memory for employee records\n");
    return STATUS_ERROR;
  }
  /* Validate that we read exactly the expected number of bytes */
  size_t total_bytes = (size_t)count * sizeof(struct employee_t);
  ssize_t bytes_read = read(fd, employees, total_bytes);
  if (bytes_read != (ssize_t)total_bytes) {
    if (bytes_read < 0) {
      perror("read");
    } else {
      printf("Incomplete read: expected %zu bytes, got %zd bytes\n", total_bytes, bytes_read);
    }
    free(employees);
    return STATUS_ERROR;
  }

  int i = 0;
  for (; i < count; i++) {
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
    /* Work on a temporary copy so we don't modify the caller's employee array */
    struct employee_t emp_net = employees[i];
    emp_net.hours = htonl(emp_net.hours);
    
    ssize_t bytes_written = write(fd, &emp_net, sizeof(struct employee_t));
    if (bytes_written != (ssize_t)sizeof(struct employee_t)) {
      perror("write");
      return STATUS_ERROR;
    }
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
