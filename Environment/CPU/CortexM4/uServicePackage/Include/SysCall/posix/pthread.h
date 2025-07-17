
#ifndef _SYS__PTHREADTYPES_H_
#define _SYS__PTHREADTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#define PTHREAD_CREATE_DETACHED     0
#define PTHREAD_CREATE_JOINABLE     1

typedef unsigned int pthread_t;          /* identify a thread */

struct sched_param
{
  int sched_priority;           /* Process execution scheduling priority */
};

typedef struct pthread_attr_s
{
  int contentionscope;
  struct sched_param schedparam;
  int  detachstate;
  void *stackaddr;
  size_t stacksize;
} pthread_attr_t;

typedef unsigned int pthread_mutex_t;

typedef struct
{
    int unused;
} pthread_mutexattr_t;

// Initialise attribute object to default values
int pthread_attr_init(pthread_attr_t *attr);

// Destroy attribute object (no-op here)
int pthread_attr_destroy(pthread_attr_t *attr);

// Set/get stack base pointer and size
int pthread_attr_setstack(pthread_attr_t *attr, void *stack, size_t stacksize);
int pthread_attr_getstack(const pthread_attr_t *attr, void **stack, size_t *stacksize);

// Set/get detach state
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

// Optionally: set/get thread priority
int pthread_attr_setpriority(pthread_attr_t *attr, int priority);
int pthread_attr_getpriority(const pthread_attr_t *attr, int *priority);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
void pthread_exit(void *retval);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif /* _POSIX_THREADS */
