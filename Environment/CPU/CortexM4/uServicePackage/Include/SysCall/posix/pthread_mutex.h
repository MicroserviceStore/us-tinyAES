
#ifndef PORTABLE_PTHREAD_MUTEX_H
#define PORTABLE_PTHREAD_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *internal; // internal RTOS or pool-based lock
} pthread_mutex_t;

typedef struct {
    int unused;
} pthread_mutexattr_t;

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif // PORTABLE_PTHREAD_MUTEX_H
