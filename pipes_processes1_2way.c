
/*
 * pipes_processes1_2way.c
 * Two-way communication via pipes between parent (P1) and child (P2).
 *
 * Flow:
 *   P1: prompt -> send string to P2
 *   P2: receive -> concat "howard.edu" -> print -> prompt for second string
 *       -> append second string -> send back to P1
 *   P1: receive -> append "gobison.org" -> print final
 *
 * Build:  gcc -Wall -Wextra -O2 -o pipes_processes1_2way pipes_processes1_2way.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLEN 4096

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    int p1_to_p2[2]; // parent writes, child reads
    int p2_to_p1[2]; // child writes, parent reads

    if (pipe(p1_to_p2) == -1) die("pipe p1_to_p2");
    if (pipe(p2_to_p1) == -1) die("pipe p2_to_p1");

    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        // --- Child (P2) ---
        // Close unused ends
        if (close(p1_to_p2[1]) == -1) die("close child p1_to_p2[1]");
        if (close(p2_to_p1[0]) == -1) die("close child p2_to_p1[0]");

        // Wrap fds with FILE* for convenient fgets/fprintf
        FILE *in  = fdopen(p1_to_p2[0], "r");
        FILE *out = fdopen(p2_to_p1[1], "w");
        if (!in || !out) die("fdopen");

        char buf[MAXLEN];
        if (!fgets(buf, sizeof(buf), in)) {
            // parent closed pipe or error
            if (ferror(in)) die("child fgets");
            // EOF without data: exit cleanly
            fclose(in);
            fclose(out);
            _exit(EXIT_SUCCESS);
        }
        // Strip newline
        size_t n = strlen(buf);
        if (n && buf[n-1] == '\n') buf[n-1] = '\0';

        // Concatenate "howard.edu"
        const char *suffix1 = "howard.edu";
        char tmp[MAXLEN];
        if (snprintf(tmp, sizeof(tmp), "%s%s", buf, suffix1) >= (int)sizeof(tmp)) {
            fprintf(stderr, "Input too long after first concat\n");
            fclose(in); fclose(out);
            _exit(EXIT_FAILURE);
        }
        printf("Other string is: %s\n", suffix1);
        printf("Output : %s\n", tmp);
        fflush(stdout);

        // Prompt for second input
        printf("Input : ");
        fflush(stdout);
        char extra[MAXLEN];
        if (!fgets(extra, sizeof(extra), stdin)) {
            if (ferror(stdin)) die("child stdin fgets");
            // EOF: treat as empty
            extra[0] = '\0';
        } else {
            size_t m = strlen(extra);
            if (m && extra[m-1] == '\n') extra[m-1] = '\0';
        }

        // Append second input
        char tmp2[MAXLEN];
        if (snprintf(tmp2, sizeof(tmp2), "%s%s", tmp, extra) >= (int)sizeof(tmp2)) {
            fprintf(stderr, "Combined string too long after second append\n");
            fclose(in); fclose(out);
            _exit(EXIT_FAILURE);
        }

        // Send back to parent
        fprintf(out, "%s\n", tmp2);
        fflush(out);
        fclose(in);
        fclose(out);
        _exit(EXIT_SUCCESS);
    } else {
        // --- Parent (P1) ---
        if (close(p1_to_p2[0]) == -1) die("close parent p1_to_p2[0]");
        if (close(p2_to_p1[1]) == -1) die("close parent p2_to_p1[1]");

        FILE *out = fdopen(p1_to_p2[1], "w");
        FILE *in  = fdopen(p2_to_p1[0], "r");
        if (!in || !out) die("fdopen");

        // Prompt for first input
        printf("Input : ");
        fflush(stdout);
        char buf[MAXLEN];
        if (!fgets(buf, sizeof(buf), stdin)) {
            if (ferror(stdin)) die("parent fgets");
            // EOF -> send empty
            buf[0] = '\0';
        }
        // Send to child
        fputs(buf, out);
        fflush(out); // ensure data is sent

        // Read back combined string from child
        char ret[MAXLEN];
        if (!fgets(ret, sizeof(ret), in)) {
            if (ferror(in)) die("parent read from child");
            fprintf(stderr, "Child closed pipe unexpectedly\n");
            fclose(in); fclose(out);
            wait(NULL);
            return EXIT_FAILURE;
        }
        size_t r = strlen(ret);
        if (r && ret[r-1] == '\n') ret[r-1] = '\0';

        // Append "gobison.org"
        const char *suffix2 = "gobison.org";
        char final[MAXLEN];
        if (snprintf(final, sizeof(final), "%s%s", ret, suffix2) >= (int)sizeof(final)) {
            fprintf(stderr, "Final string too long after gobison.org\n");
            fclose(in); fclose(out);
            wait(NULL);
            return EXIT_FAILURE;
        }
        printf("Output : %s\n", final);

        fclose(in);
        fclose(out);
        int status = 0;
        waitpid(pid, &status, 0);
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
