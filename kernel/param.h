#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       300  // open files per system !! Увеличиваем с 100 до 300 для поддержки большего количества открытых файлов
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*4)  // size of disk block cache !! Увеличиваем для лучшей производительности при работе с большими файлами
#define FSSIZE       20000  // size of file system in blocks !! Увеличиваем с 2000 до 20000, чтобы хватило места для больших файлов
#define MAXPATH      128   // maximum file path name
#define USERSTACK    1     // user stack pages