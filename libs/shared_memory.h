#ifndef SHM_SYNC_COMMON_H
#define SHM_SYNC_COMMON_H


#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_CLIENT 0
#define SHM_SERVER 1
#define MAX_MUTEXES 4
#define SEGMENT_KEY_STR "/tmp/wshm"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Arguments {
    int size;
    int count;
    char segment_key_path[240];

} Arguments;

struct Mutex {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int count;
};

struct Sync {
    int segment_id;
    uint8_t *raw_shared_memory;
    uint8_t *shared_memory[MAX_MUTEXES];
    struct Mutex *client_mutex[MAX_MUTEXES];
    struct Mutex *server_mutex[MAX_MUTEXES];
    int mutex_created;
};

extern struct Sync local_sync;

void cleanup(struct Sync *sync);

void init_sync(struct Sync *sync);

void destroy_sync(struct Sync *sync);

uint8_t sync_wait(struct Mutex *sync);

void sync_notify(struct Mutex *sync);

void create_sync(struct Sync *sync, struct Arguments *args);

void shm_timeout(uint16_t timeout);

void shm_init(uint8_t _is_server, uint32_t shm_size, const char *segment_key_string);

void shm_notify(uint16_t mutex_num);

uint8_t shm_wait(uint16_t mutex_num);

void shm_clean();

/******************** DEFINITIONS ********************/

struct timeval;

/******************** INTERFACE ********************/

/**
 * Calls perror() and exits the program.
 *
 * Use this function when the error *is* the result of a syscall failure. Then
 * it will print the specified message along with the implementation defined
 * error message that is appended in perror(). Do not append a newline.
 *
 * \param message The message to print.
 *
 * \see terminate()
 */
void throw_custom(const char *message) __attribute__((noreturn));

/**
 * Prints a message to stderr and exits the program.
 *
 * Use this function when the error is not the result of a syscall failure. Do
 * append a newline to the message.
 *
 * \param message The message to print.
 *
 * \see throw()
 */
void terminate(const char *message) __attribute__((noreturn));

/**
 * Prints a message to stderr.
 *
 * param message The message to print.
 */
void print_error(const char *message);

/**
 * Prints "Warning: <message>".
 *
 * \param message The warning message.
 */
void warn(const char *message);

int generate_key(const char *path);

void nsleep(int nanoseconds);

int current_milliseconds();
int timeval_to_milliseconds(const struct timeval *time);

void pin_thread(int where);

#ifdef __cplusplus
}
#endif

#endif /* SHM_SYNC_COMMON_H */
