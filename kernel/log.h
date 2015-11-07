/*
 * log.h
 *
 * log should have no dependencies within the project
 */

typedef void (*log_logger)(char *);

// Logging Functions
void klog(char *m);
void kwarn(char *m);
void kerror(char *m);

// Telling functions (command output)
void ktell(char *m);

// Setting functions
void klog_set(log_logger t);
void kwarn_set(log_logger t);
void kerror_set(log_logger t);
void ktell_set(log_logger t);

// Throwing functions (unrecoverable errors)
void kthrow(char *m);
