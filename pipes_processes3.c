
/*
 * pipes_processes3.c
 * Build a pipeline equivalent to: cat scores | grep <arg> | sort
 *
 * Usage: ./pipes_processes3 <pattern>
 * Expects a file named "scores" in the current directory.
 *
 * Build: gcc -Wall -Wextra -O2 -o pipes_processes3 pipes_processes3.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pattern>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *pattern = argv[1];

    int p_cat_to_grep[2];
    int p_grep_to_sort[2];
    if (pipe(p_cat_to_grep) == -1) die("pipe1");
    if (pipe(p_grep_to_sort) == -1) die("pipe2");

    pid_t pid_cat = fork();
    if (pid_cat < 0) die("fork cat");

    if (pid_cat == 0) {
        // cat scores: stdout -> p_cat_to_grep[1]
        if (dup2(p_cat_to_grep[1], STDOUT_FILENO) == -1) die("dup2 cat stdout");
        // Close unused ends
        close(p_cat_to_grep[0]);
        close(p_cat_to_grep[1]);
        close(p_grep_to_sort[0]);
        close(p_grep_to_sort[1]);
        execlp("cat", "cat", "scores", (char*)NULL);
        die("execlp cat");
    }

    pid_t pid_grep = fork();
    if (pid_grep < 0) die("fork grep");

    if (pid_grep == 0) {
        // grep pattern: stdin <- p_cat_to_grep[0], stdout -> p_grep_to_sort[1]
        if (dup2(p_cat_to_grep[0], STDIN_FILENO) == -1) die("dup2 grep stdin");
        if (dup2(p_grep_to_sort[1], STDOUT_FILENO) == -1) die("dup2 grep stdout");
        // Close unused ends
        close(p_cat_to_grep[0]);
        close(p_cat_to_grep[1]);
        close(p_grep_to_sort[0]);
        close(p_grep_to_sort[1]);
        execlp("grep", "grep", pattern, (char*)NULL);
        die("execlp grep");
    }

    pid_t pid_sort = fork();
    if (pid_sort < 0) die("fork sort");

    if (pid_sort == 0) {
        // sort: stdin <- p_grep_to_sort[0]
        if (dup2(p_grep_to_sort[0], STDIN_FILENO) == -1) die("dup2 sort stdin");
        // Close unused ends
        close(p_cat_to_grep[0]);
        close(p_cat_to_grep[1]);
        close(p_grep_to_sort[0]);
        close(p_grep_to_sort[1]);
        execlp("sort", "sort", (char*)NULL);
        die("execlp sort");
    }

    // Parent: close all pipe fds and wait
    close(p_cat_to_grep[0]);
    close(p_cat_to_grep[1]);
    close(p_grep_to_sort[0]);
    close(p_grep_to_sort[1]);

    int status;
    waitpid(pid_cat, &status, 0);
    waitpid(pid_grep, &status, 0);
    waitpid(pid_sort, &status, 0);
    return 0;
}
