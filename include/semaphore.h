#ifndef SEM_M
#define SEM_M
#define SEM_KEY 1234
#define SHM_KEY 12345
#define SEM_NAME "/car_sim_semaphore"

void init_semaphore();
void destroy_semaphore();
void enter_critical_section(int i);
void exit_critical_section(int i);

#endif