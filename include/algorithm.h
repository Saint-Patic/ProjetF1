#define SEM_KEY 1234
#define SHM_KEY 12345 // Clé pour la mémoire partagée
#define SEM_NAME_MUTEX "/mutex_sem"
#define SEM_NAME_MUTLECT "/mutlect_sem"

void init_semaphore();
void destroy_semaphore();
void enter_critical_section_reader();
void exit_critical_section_reader();
void enter_critical_section_writer();
void exit_critical_section_writer();