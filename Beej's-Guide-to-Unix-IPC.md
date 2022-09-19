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