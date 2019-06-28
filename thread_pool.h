#ifndef _LOCK_H_
#define _LOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif
	

extern int pthread_lock_init(void);
extern int pthread_lock(void);
extern int pthread_unlock(void);




#ifdef __cplusplus
}
#endif

#endif /* _LOCK_H_ */

