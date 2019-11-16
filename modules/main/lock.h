#ifndef _LOCK_H_
#define _LOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif
	

extern int pthread_lock_init(void);
extern int pthread_lock(void);
extern int pthread_unlock(void);

//
extern void* thread_routine (void *arg);
extern int pool_destroy (void);
extern void pool_init (int max_thread_num);



#ifdef __cplusplus
}
#endif

#endif /* _LOCK_H_ */

