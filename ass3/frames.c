#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct Data
{
    int pfn;
    int access;
};

struct Stats {
    int access;
    int misses;
    int writes;
    int drops;
} stats;

struct pte {
    int pfn;
    int valid;
    int dirty;
    int cacheId;
    int last_used;
    int used;
    int fif;  
};

struct pte all[(1 << 20) + 5];
struct Data data[10000005];
int size = 0;

void get_data(FILE *in) {
    while(1) {
        char access;
        unsigned vpn;

        if(EOF == fscanf(in, "%x %c", &vpn, &access))
            break;

        data[size].pfn = (vpn >> 12);

        if(access == 'R') data[size].access = 0;
        else data[size].access = 1;

        size++;
    }
}

void do_random(int n, int verbose) {

    srand(5635);

    struct pte cache[n];
    
    int s = 0;

    int accesses = 0;
    int drops = 0;
    int misses = 0;
    int writes = 0;

    for(int i = 0; i < size; i++) {
        // check if exists in cache
        // if yes, increase hit
        // if no, check if cache has space 
            // if yes, add to cache
            // no, evict

        accesses++;
        int pfn = data[i].pfn;
        int access = data[i].access;

        if(all[pfn].valid == 1) {
            if(access == 1) {
                all[pfn].dirty = 1;
                cache[all[pfn].cacheId].dirty = 1;
            }
            continue;
        }

        struct pte temp;
        temp.pfn = pfn;
        temp.dirty = access;

        all[pfn].valid = 1;
        all[pfn].dirty = access;
        
        misses++;
        if(s == n) {
            int r = (rand() % n);

            if(cache[r].dirty == 0) {
                drops++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", temp.pfn, cache[r].pfn);
                }
            }
            else {
                writes++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", temp.pfn, cache[r].pfn);
                }
            }

            all[cache[r].pfn].valid = 0;
            all[pfn].cacheId = r;
            cache[r] = temp;
        } else {
            cache[s] = temp;
            all[pfn].cacheId = s;
            s++;
        }
    }

    stats.access = accesses;
    stats.drops = drops;
    stats.misses = misses;
    stats.writes = writes;
}

void do_fifo(int n, int verbose) {
    struct pte cache[n];

    int s = 0, f = 0;

    int accesses = 0;
    int drops = 0;
    int misses = 0;
    int writes = 0;

    for(int i = 0; i < size; i++) {
        accesses++;

        int pfn = data[i].pfn;
        int access = data[i].access;

        struct pte temp;
        temp.pfn = pfn;
        temp.dirty = access;


        if(all[pfn].valid == 1) {
            if(access == 1) {
                all[pfn].dirty = 1;
                cache[all[pfn].cacheId].dirty = 1;
            }

            continue;
        }

        all[pfn].valid = 1;
        all[pfn].dirty = access;
        misses++;
        if(s == n) {
            if(cache[f].dirty == 0) {
                drops++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", temp.pfn, cache[f].pfn);
                }
            }
            else {
                writes++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", temp.pfn, cache[f].pfn);
                }
            }

            all[cache[f].pfn].valid = 0;
            all[pfn].cacheId = f;
            cache[f] = temp;

            f = (f + 1) % n;
        } else {
            cache[s] = temp;
            all[pfn].cacheId = s;
            s++;
        }
    }

    stats.access = accesses;
    stats.drops = drops;
    stats.misses = misses;
    stats.writes = writes;
}

void do_lru(int n, int verbose) {
    struct pte cache[n];

    int s = 0;

    int accesses = 0;
    int drops = 0;
    int misses = 0;
    int writes = 0;

    for(int i = 0; i < size; i++) {
        accesses++;

        int pfn = data[i].pfn;
        int access = data[i].access;


        if(all[pfn].valid == 1) {
            if(access == 1) {
                all[pfn].dirty = 1;
                cache[all[pfn].cacheId].dirty = 1;
            }
            
            cache[all[pfn].cacheId].last_used = i;

            continue;
        }

        struct pte temp;
        temp.pfn = pfn;
        temp.dirty = access;
        temp.last_used = i;

        all[pfn].valid = 1;
        all[pfn].dirty = access;
        misses++;

        if(s == n) {
            int f = -1;
            int M = INT_MAX;

            for(int j = 0; j < n; j++) {
                if(cache[j].last_used < M) {
                    M = cache[j].last_used;
                    f = j;
                }
            }

            if(cache[f].dirty == 0) {
                drops++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", temp.pfn, cache[f].pfn);
                }
            }
            else {
                writes++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", temp.pfn, cache[f].pfn);
                }
            }

            all[cache[f].pfn].valid = 0;
            all[pfn].cacheId = f;
            cache[f] = temp;

        } else {
            cache[s] = temp;
            all[pfn].cacheId = s;
            s++;
        }
    }

    stats.access = accesses;
    stats.drops = drops;
    stats.misses = misses;
    stats.writes = writes;
}

int clock = 0;
void do_clock(int n, int verbose) {
    struct pte cache[n];

    int s = 0;

    int accesses = 0;
    int drops = 0;
    int misses = 0;
    int writes = 0;

    for(int i = 0; i < size; i++) {
        accesses++;

        int pfn = data[i].pfn;
        int access = data[i].access;

        if(all[pfn].valid == 1) {
            if(access == 1) {
                all[pfn].dirty = 1;
                cache[all[pfn].cacheId].dirty = 1;
            }

            cache[all[pfn].cacheId].used = 1;

            continue;
        }

        struct pte temp;
        temp.pfn = pfn;
        temp.dirty = access;
        temp.used = 1;

        all[pfn].valid = 1;
        all[pfn].dirty = access;
        misses++;

        if(s == n) {
            // clock algo
            do {
                if(cache[clock].used == 1) {
                    cache[clock].used = 0;
                    clock = (clock + 1) % n;
                    continue;
                }

                break;
            } while(1);

            int f = clock;
            clock = (clock + 1) % n;
            //---------

            if(cache[f].dirty == 0) {
                drops++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", temp.pfn, cache[f].pfn);
                }
            }
            else {
                writes++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", temp.pfn, cache[f].pfn);
                }
            }

            all[cache[f].pfn].valid = 0;
            all[pfn].cacheId = f;
            cache[f] = temp;
        } else {
            cache[s] = temp;
            all[pfn].cacheId = s;
            s++;
        }
    }

    stats.access = accesses;
    stats.drops = drops;
    stats.misses = misses;
    stats.writes = writes;
}

// int indices[(1 << 20) + 5];

// void help_opt(int fif[]) {
//     for(int i = 0; i < size; i++) {
//         if(indices[data[i].pfn] != -1) {
//             fif[indices[data[i].pfn]] = i;
//         }

//         indices[data[i].pfn] = i;
//     }
// }

void do_opt(int n, int verbose) {
    // //-----------
    // int fif[size];
    // memset(fif, -1, sizeof(fif));
    // memset(indices, -1, sizeof(indices));
    // help_opt(fif);
    // for(int i = 0; i < size; i++) {
    //     printf("%d ", fif[i]);
    // }

    // return;
    // //-------------
    struct pte cache[n];

    int s = 0;

    int accesses = 0;
    int drops = 0;
    int misses = 0;
    int writes = 0;

    for(int i = 0; i < size; i++) {
        accesses++;
        // if(i % 500 == 0) printf("%d\n", i);

        int pfn = data[i].pfn;
        int access = data[i].access;

        if(all[pfn].valid == 1) {
            if(access == 1) {
                all[pfn].dirty = 1;
                cache[all[pfn].cacheId].dirty = 1;
            }

            continue;
        }

        struct pte temp;
        temp.pfn = pfn;
        temp.dirty = access;


        all[pfn].valid = 1;
        all[pfn].dirty = access;
        misses++;

        if(s == n) {
            // find f
            int f = -1, M = INT_MIN;
            for(int j = 0; j < n; j++) {
                all[cache[j].pfn].fif = -1;
            }

            int count = 0;
            for(int j = i+1; j < size; j++) {
                int pp = data[j].pfn;
                if(all[pp].valid == 1) {
                    if(all[pp].fif == -1) {
                        all[pp].fif = j;
                        count++;
                    }
                }

                if(count == n) break;
            }

            for(int j = 0; j < n; j++) {
                if(all[cache[j].pfn].fif > M) {
                    M = all[cache[j].pfn].fif;
                    f = j;
                }
            }

            if(cache[f].dirty == 0) {
                drops++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", temp.pfn, cache[f].pfn);
                }
            }
            else {
                writes++;
                if(verbose) {
                    printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", temp.pfn, cache[f].pfn);
                }
            }

            all[cache[f].pfn].valid = 0;
            all[pfn].cacheId = f;
            cache[f] = temp;
        } else {
            cache[s] = temp;
            all[pfn].cacheId = s;
            s++;
        }
    }

    stats.access = accesses;
    stats.drops = drops;
    stats.misses = misses;
    stats.writes = writes;
}

void print_stats() {
    printf("Number of memory accesses: %d\n", stats.access);
    printf("Number of misses: %d\n", stats.misses);
    printf("Number of writes: %d\n", stats.writes);
    printf("Number of drops: %d\n", stats.drops);
}

// void print_data() {
//     printf("%d\n", size);

//     for(int i = 0; i < size; i++) {
//         printf("%x %d %d\n", data[i].vpn, data[i].vpn, data[i].access);
//     }
// }

int main(int argc, char *argv[])
{
    // getting cmd args
    FILE *in = fopen(argv[1], "r");
    int frames = atoi(argv[2]);
    int strategy; 
    int verbose = 0;

    if(argc == 5)
        verbose = 1;

    char* s = argv[3];
    if(strcmp(s, "OPT") == 0)
        strategy = 0;
    if(strcmp(s, "FIFO") == 0)
        strategy = 1;
    if(strcmp(s, "CLOCK") == 0)
        strategy = 2;
    if(strcmp(s, "LRU") == 0)
        strategy = 3;
    if(strcmp(s, "RANDOM") == 0)
        strategy = 4;
    // ------------

    get_data(in);
    // print_data();
    //-----------

    if(strategy == 0) do_opt(frames, verbose);
    if(strategy == 1) do_fifo(frames, verbose);
    if(strategy == 2) do_clock(frames, verbose);
    if(strategy == 3) do_lru(frames, verbose);
    if(strategy == 4) do_random(frames, verbose);
    print_stats();
    return 0;
}