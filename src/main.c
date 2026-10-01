#include <stdio.h>
#include <string.h>

#define BUF_SIZE 256

int main(void) {
    char buf[BUF_SIZE];
    unsigned long line_no = 0;

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        line_no++;

        /* No newline in the buffer: the line was cut off, or it is the last
           line of the file with no trailing newline. Discard the rest. */
        int too_long = 0;
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
                too_long = 1;
            }
        }
        if (too_long) {
            printf("ERROR line=%lu reason=line_too_long\n", line_no);
            continue;
        }

        buf[strcspn(buf, "\r\n")] = '\0';

        char word[16];
        if (sscanf(buf, "%15s", word) != 1) {
            continue;                      /* blank line */
        }
        if (strcmp(word, "QUIT") == 0) {
            break;
        }
        printf("ERROR line=%lu reason=bad_command\n", line_no);
    }

    fflush(stdout);
    return 0;
}
