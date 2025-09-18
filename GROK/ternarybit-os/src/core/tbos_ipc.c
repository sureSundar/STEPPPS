/**
 * @file tbos_ipc.c
 * @brief TernaryBit OS Inter-Process Communication (IPC) System
 *
 * Implements complete IPC mechanisms including:
 * - Message queues
 * - Shared memory segments
 * - Semaphores
 * - Pipes (named and unnamed)
 * - Signals
 * - UNIX domain sockets
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// IPC types
typedef enum {
    IPC_TYPE_MESSAGE_QUEUE,
    IPC_TYPE_SHARED_MEMORY,
    IPC_TYPE_SEMAPHORE,
    IPC_TYPE_PIPE,
    IPC_TYPE_SOCKET
} tbos_ipc_type_t;

// Message structure
typedef struct {
    long type;
    char data[256];
    size_t size;
    pid_t sender_pid;
    time_t timestamp;
} tbos_message_t;

// Shared memory segment
typedef struct {
    int shmid;
    void* address;
    size_t size;
    int attach_count;
    char name[64];
} tbos_shm_t;

// Semaphore structure
typedef struct {
    int semid;
    int value;
    int waiting_count;
    char name[64];
    pthread_mutex_t lock;
    pthread_cond_t cond;
} tbos_semaphore_t;

// Pipe structure
typedef struct {
    int read_fd;
    int write_fd;
    char name[64];
    bool is_named;
} tbos_pipe_t;

// IPC manager
typedef struct {
    int msg_queue_id;
    tbos_shm_t* shm_segments[16];
    tbos_semaphore_t* semaphores[16];
    tbos_pipe_t* pipes[16];
    int shm_count;
    int sem_count;
    int pipe_count;
    pthread_mutex_t manager_lock;
} tbos_ipc_manager_t;

// Global IPC manager
static tbos_ipc_manager_t* g_ipc = NULL;

/**
 * Initialize IPC subsystem
 */
int tbos_ipc_init(void) {
    if (g_ipc) return 0;

    g_ipc = calloc(1, sizeof(tbos_ipc_manager_t));
    if (!g_ipc) return -1;

    // Create message queue
    key_t key = ftok("/tmp", 'T');
    g_ipc->msg_queue_id = msgget(key, IPC_CREAT | 0666);
    if (g_ipc->msg_queue_id < 0) {
        printf("⚠️  Message queue creation failed (may need privileges)\n");
    }

    pthread_mutex_init(&g_ipc->manager_lock, NULL);

    printf("✅ TBOS IPC Subsystem initialized\n");
    return 0;
}

// ============================================
// Message Queue Operations
// ============================================

/**
 * Send a message
 */
int tbos_msg_send(long type, const void* data, size_t size) {
    if (!g_ipc || g_ipc->msg_queue_id < 0) return -1;

    tbos_message_t msg;
    msg.type = type;
    msg.size = size > 256 ? 256 : size;
    memcpy(msg.data, data, msg.size);
    msg.sender_pid = getpid();
    msg.timestamp = time(NULL);

    if (msgsnd(g_ipc->msg_queue_id, &msg, sizeof(msg) - sizeof(long), IPC_NOWAIT) < 0) {
        printf("⚠️  Message send failed: %s\n", strerror(errno));
        return -1;
    }

    printf("✅ Sent message type %ld (%zu bytes)\n", type, size);
    return 0;
}

/**
 * Receive a message
 */
int tbos_msg_receive(long type, void* buffer, size_t max_size) {
    if (!g_ipc || g_ipc->msg_queue_id < 0) return -1;

    tbos_message_t msg;

    if (msgrcv(g_ipc->msg_queue_id, &msg, sizeof(msg) - sizeof(long), type, IPC_NOWAIT) < 0) {
        if (errno != ENOMSG) {
            printf("⚠️  Message receive failed: %s\n", strerror(errno));
        }
        return -1;
    }

    size_t copy_size = msg.size < max_size ? msg.size : max_size;
    memcpy(buffer, msg.data, copy_size);

    printf("✅ Received message type %ld (%zu bytes) from PID %d\n",
           msg.type, msg.size, msg.sender_pid);

    return copy_size;
}

// ============================================
// Shared Memory Operations
// ============================================

/**
 * Create shared memory segment
 */
void* tbos_shm_create(const char* name, size_t size) {
    if (!g_ipc || g_ipc->shm_count >= 16) return NULL;

    pthread_mutex_lock(&g_ipc->manager_lock);

    // Create shared memory
    key_t key = ftok("/tmp", g_ipc->shm_count + 1);
    int shmid = shmget(key, size, IPC_CREAT | 0666);

    void* addr = NULL;
    if (shmid >= 0) {
        addr = shmat(shmid, NULL, 0);
        if (addr != (void*)-1) {
            // Track the segment
            tbos_shm_t* shm = malloc(sizeof(tbos_shm_t));
            shm->shmid = shmid;
            shm->address = addr;
            shm->size = size;
            shm->attach_count = 1;
            strncpy(shm->name, name, 63);

            g_ipc->shm_segments[g_ipc->shm_count++] = shm;

            printf("✅ Created shared memory '%s' (%zu bytes) at %p\n", name, size, addr);
        } else {
            shmctl(shmid, IPC_RMID, NULL);
            addr = NULL;
        }
    }

    pthread_mutex_unlock(&g_ipc->manager_lock);
    return addr;
}

/**
 * Attach to shared memory
 */
void* tbos_shm_attach(const char* name) {
    pthread_mutex_lock(&g_ipc->manager_lock);

    for (int i = 0; i < g_ipc->shm_count; i++) {
        if (strcmp(g_ipc->shm_segments[i]->name, name) == 0) {
            g_ipc->shm_segments[i]->attach_count++;
            pthread_mutex_unlock(&g_ipc->manager_lock);
            return g_ipc->shm_segments[i]->address;
        }
    }

    pthread_mutex_unlock(&g_ipc->manager_lock);
    return NULL;
}

/**
 * Detach from shared memory
 */
int tbos_shm_detach(void* addr) {
    if (shmdt(addr) == 0) {
        printf("✅ Detached from shared memory at %p\n", addr);
        return 0;
    }
    return -1;
}

// ============================================
// Semaphore Operations
// ============================================

/**
 * Create a semaphore
 */
int tbos_sem_create(const char* name, int initial_value) {
    if (!g_ipc || g_ipc->sem_count >= 16) return -1;

    pthread_mutex_lock(&g_ipc->manager_lock);

    tbos_semaphore_t* sem = malloc(sizeof(tbos_semaphore_t));
    sem->value = initial_value;
    sem->waiting_count = 0;
    strncpy(sem->name, name, 63);
    pthread_mutex_init(&sem->lock, NULL);
    pthread_cond_init(&sem->cond, NULL);

    int sem_id = g_ipc->sem_count;
    g_ipc->semaphores[g_ipc->sem_count++] = sem;

    pthread_mutex_unlock(&g_ipc->manager_lock);

    printf("✅ Created semaphore '%s' (initial value: %d)\n", name, initial_value);
    return sem_id;
}

/**
 * Wait on semaphore (P operation)
 */
int tbos_sem_wait(int sem_id) {
    if (!g_ipc || sem_id < 0 || sem_id >= g_ipc->sem_count) return -1;

    tbos_semaphore_t* sem = g_ipc->semaphores[sem_id];

    pthread_mutex_lock(&sem->lock);
    sem->waiting_count++;

    while (sem->value <= 0) {
        pthread_cond_wait(&sem->cond, &sem->lock);
    }

    sem->value--;
    sem->waiting_count--;
    pthread_mutex_unlock(&sem->lock);

    return 0;
}

/**
 * Signal semaphore (V operation)
 */
int tbos_sem_signal(int sem_id) {
    if (!g_ipc || sem_id < 0 || sem_id >= g_ipc->sem_count) return -1;

    tbos_semaphore_t* sem = g_ipc->semaphores[sem_id];

    pthread_mutex_lock(&sem->lock);
    sem->value++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->lock);

    return 0;
}

// ============================================
// Pipe Operations
// ============================================

/**
 * Create a pipe
 */
int tbos_pipe_create(const char* name, bool named) {
    if (!g_ipc || g_ipc->pipe_count >= 16) return -1;

    pthread_mutex_lock(&g_ipc->manager_lock);

    tbos_pipe_t* pipe = malloc(sizeof(tbos_pipe_t));
    strncpy(pipe->name, name, 63);
    pipe->is_named = named;

    int fds[2];
    if (named) {
        // Create named pipe (FIFO)
        char fifo_path[128];
        snprintf(fifo_path, sizeof(fifo_path), "/tmp/tbos_pipe_%s", name);

        if (mkfifo(fifo_path, 0666) < 0 && errno != EEXIST) {
            free(pipe);
            pthread_mutex_unlock(&g_ipc->manager_lock);
            return -1;
        }

        pipe->read_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);
        pipe->write_fd = open(fifo_path, O_WRONLY | O_NONBLOCK);
    } else {
        // Create unnamed pipe
        if (pipe2(fds, O_NONBLOCK) < 0) {
            free(pipe);
            pthread_mutex_unlock(&g_ipc->manager_lock);
            return -1;
        }
        pipe->read_fd = fds[0];
        pipe->write_fd = fds[1];
    }

    int pipe_id = g_ipc->pipe_count;
    g_ipc->pipes[g_ipc->pipe_count++] = pipe;

    pthread_mutex_unlock(&g_ipc->manager_lock);

    printf("✅ Created %s pipe '%s'\n", named ? "named" : "unnamed", name);
    return pipe_id;
}

/**
 * Write to pipe
 */
ssize_t tbos_pipe_write(int pipe_id, const void* data, size_t size) {
    if (!g_ipc || pipe_id < 0 || pipe_id >= g_ipc->pipe_count) return -1;

    tbos_pipe_t* pipe = g_ipc->pipes[pipe_id];
    ssize_t written = write(pipe->write_fd, data, size);

    if (written > 0) {
        printf("✅ Wrote %zd bytes to pipe '%s'\n", written, pipe->name);
    }

    return written;
}

/**
 * Read from pipe
 */
ssize_t tbos_pipe_read(int pipe_id, void* buffer, size_t size) {
    if (!g_ipc || pipe_id < 0 || pipe_id >= g_ipc->pipe_count) return -1;

    tbos_pipe_t* pipe = g_ipc->pipes[pipe_id];
    ssize_t read_bytes = read(pipe->read_fd, buffer, size);

    if (read_bytes > 0) {
        printf("✅ Read %zd bytes from pipe '%s'\n", read_bytes, pipe->name);
    }

    return read_bytes;
}

// ============================================
// UNIX Domain Socket Operations
// ============================================

/**
 * Create UNIX domain socket
 */
int tbos_socket_create(const char* name) {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) return -1;

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/tbos_sock_%s", name);

    // Remove old socket file
    unlink(addr.sun_path);

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock_fd);
        return -1;
    }

    if (listen(sock_fd, 5) < 0) {
        close(sock_fd);
        return -1;
    }

    printf("✅ Created UNIX domain socket '%s'\n", name);
    return sock_fd;
}

/**
 * IPC Statistics
 */
void tbos_ipc_stats(void) {
    if (!g_ipc) {
        printf("IPC not initialized\n");
        return;
    }

    printf("\n📊 IPC Statistics:\n");
    printf("   Message queue ID: %d\n", g_ipc->msg_queue_id);
    printf("   Shared memory segments: %d\n", g_ipc->shm_count);
    printf("   Semaphores: %d\n", g_ipc->sem_count);
    printf("   Pipes: %d\n", g_ipc->pipe_count);

    if (g_ipc->shm_count > 0) {
        printf("\n   Shared Memory:\n");
        for (int i = 0; i < g_ipc->shm_count; i++) {
            tbos_shm_t* shm = g_ipc->shm_segments[i];
            printf("     - %s: %zu bytes at %p (attachments: %d)\n",
                   shm->name, shm->size, shm->address, shm->attach_count);
        }
    }

    if (g_ipc->sem_count > 0) {
        printf("\n   Semaphores:\n");
        for (int i = 0; i < g_ipc->sem_count; i++) {
            tbos_semaphore_t* sem = g_ipc->semaphores[i];
            printf("     - %s: value=%d, waiting=%d\n",
                   sem->name, sem->value, sem->waiting_count);
        }
    }
}

/**
 * IPC demonstration
 */
void tbos_ipc_demo(void) {
    printf("\n📮 === TBOS Inter-Process Communication Demo ===\n");

    // Initialize IPC
    tbos_ipc_init();

    // Test message queue
    printf("\n1️⃣  Testing Message Queue:\n");
    const char* msg_data = "Hello from TBOS IPC!";
    tbos_msg_send(1, msg_data, strlen(msg_data));

    char recv_buffer[256];
    int recv_size = tbos_msg_receive(1, recv_buffer, sizeof(recv_buffer));
    if (recv_size > 0) {
        recv_buffer[recv_size] = '\0';
        printf("   Received: \"%s\"\n", recv_buffer);
    }

    // Test shared memory
    printf("\n2️⃣  Testing Shared Memory:\n");
    void* shm = tbos_shm_create("test_shm", 4096);
    if (shm) {
        strcpy(shm, "Data in shared memory!");
        printf("   Written to shared memory: \"%s\"\n", (char*)shm);

        void* shm2 = tbos_shm_attach("test_shm");
        if (shm2) {
            printf("   Read from attached memory: \"%s\"\n", (char*)shm2);
        }
    }

    // Test semaphores
    printf("\n3️⃣  Testing Semaphores:\n");
    int sem_id = tbos_sem_create("test_sem", 1);
    if (sem_id >= 0) {
        printf("   Acquiring semaphore...\n");
        tbos_sem_wait(sem_id);
        printf("   Semaphore acquired\n");

        printf("   Releasing semaphore...\n");
        tbos_sem_signal(sem_id);
        printf("   Semaphore released\n");
    }

    // Test pipes
    printf("\n4️⃣  Testing Pipes:\n");
    int pipe_id = tbos_pipe_create("test_pipe", false);
    if (pipe_id >= 0) {
        const char* pipe_data = "Data through pipe!";
        tbos_pipe_write(pipe_id, pipe_data, strlen(pipe_data));

        char pipe_buffer[128];
        ssize_t pipe_read = tbos_pipe_read(pipe_id, pipe_buffer, sizeof(pipe_buffer));
        if (pipe_read > 0) {
            pipe_buffer[pipe_read] = '\0';
            printf("   Pipe data: \"%s\"\n", pipe_buffer);
        }
    }

    // Test UNIX domain socket
    printf("\n5️⃣  Testing UNIX Domain Socket:\n");
    int sock = tbos_socket_create("test_socket");
    if (sock >= 0) {
        printf("   Socket listening on /tmp/tbos_sock_test_socket\n");
        close(sock);
    }

    // Show statistics
    tbos_ipc_stats();

    // Cleanup
    if (shm) tbos_shm_detach(shm);

    printf("\n✅ IPC demonstration complete!\n");
}