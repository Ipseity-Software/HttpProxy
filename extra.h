#ifndef X_H
#define X_H

struct list_s *addThread(struct list_s *lst, struct list_s *node, pthread_t tid)
struct list_s *createThread(int fd)
struct list_s *removeThread(struct list_s *lst, pthread_t tid)


#endif
