# 3.Signals

- One process raise a signal and have it delivered to another process. The destination - process's handler is invoked and the process can handle it.
- One process can stop another process by sending the `SIGSTOP` to that process. To continue, the process has to receive signal `SIGCONT`.
- Many signals are predefined and the process has a default signal handler to deal with it.
- Interrupt signal `SIGINT` = `^c`.  `SIGTERM` = `kill xxx`. Kill signal `SIGKILL` = `kill -9 xxx`. `SIGKILL`、`SIGSTOP` can't add signal handler for.
- `SIGUSR1`, `SIGUSER2` for whatever you want.

## 3.1Catching Signals for Fun

- Use `kill` command or `kill()` to send signals to a process. `raise()` = `kill(getpid(), sig)`.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig)
{
    write(0, "Ahhh! SIGINT!\n", 14);
}

int main(void)
{
    void sigint_handler(int sig); /* prototype */
    char s[200];
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("Enter a string:\n");

    if (fgets(s, sizeof s, stdin) == NULL)
        perror("fgets");
    else 
        printf("You entered: %s\n", s);

    return 0;
}
```

```
Enter a string:
the quick brown fox jum^CAhhh! SIGINT!
fgets: Interrupted system call
```
`fgets()` is interrupt by signal `^c` then return an error.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig)
{
    write(0, "Ahhh! SIGINT!\n", 14);
}

int main(void)
{
    void sigint_handler(int sig); /* prototype */
    char s[200];
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART; // or SA_RESTART
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("Enter a string:\n");

    if (fgets(s, sizeof s, stdin) == NULL)
        perror("fgets");
    else 
        printf("You entered: %s\n", s);

    return 0;
}
```

```
Enter a string:
Hello^CAhhh! SIGINT!
Er, hello!^CAhhh! SIGINT!
This time fer sure!
You entered: This time fer sure!
```
`sa.sa_flags = SA_RESTART` can restart the interrupt system call `fgets()`.

## 3.2 Async signal safe function

- It cant be safely called from within a signal handler.
- `write()` is async-safe, while `printf()` is not.
- Can't alter any shared(e.g. global) data, with one exception: variables that are declared to be of storage class and type `volatile sig_atomic_t`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t got_usr1;

void sigusr1_handler(int sig)
{
    got_usr1 = 1;
}

int main(void)
{
    struct sigaction sa;

    got_usr1 = 0;

    sa.sa_handler = sigusr1_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while (!got_usr1) {
        printf("PID %d: working hard...\n", getpid());
        sleep(1);
    }

    printf("Done in by SIGUSR1!\n");

    return 0;
}
```

`kill -USR1 931605`

```
PID 931605: working hard...
PID 931605: working hard...
PID 931605: working hard...
PID 931605: working hard...
PID 931605: working hard...
Done in by SIGUSR1!
```

## 3.3 signal()
- Discouraged.