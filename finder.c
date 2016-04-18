#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// we will need these headers for traversing the filesystem
#include <sys/types.h>
#include <dirent.h>

#define MAX_DIR 512
#define TRUE 1
#define FALSE 0
#define MAX_FOUND 50000

// use this wrapper, just for better reading
#define pop() stack[--ptr];

// in case our system doesn't have this macro defined
#ifndef DT_DIR
#define DT_DIR 8
#endif

char stack[MAX_FOUND][MAX_DIR];
int ptr = 0;

// use a function to make it cleaner
static void push(char x[], char y[]) {
    strcpy(stack[ptr], x);
    strcat(stack[ptr], "/");
    strcat(stack[ptr++], y);
}

// first goes the match function
int match(char a[], char b[]) {
    // we suppose, that we search for a in b, so that |a| <= |b|
    // get the length of a and b
    int len_a = strlen(a);
    int len_b = strlen(b);
    // shortcur if we search for one character
    if (len_a == 1) {
        int i;
        for (i = 0; i < len_b; i++) {
            if (b[i] == a[0])
                return TRUE;
        }
        return FALSE;
    }
    int i;
    int last = len_a - 1; // index of last in a[]
    for (i = 0; i <= len_b - len_a; i++) {
        // we will always compare the first and last characters of a
        // unless a has only one character + '\0'
        if ((b[i] == a[0]) && (b[i+last] == a[last])) {
            // Here we probably found a match
            int j;
            int loop_broken = FALSE;
            for (j = 1; j <= last; j++) {
                if (a[j] == '\0') {
                    // parsed it all
                    loop_broken = TRUE;
                    break;
                }
                if (a[j] != b[i+j]) {

                    loop_broken = TRUE;
                    break;
                }
            }
            // if loop not interrupted
            if (loop_broken == FALSE) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// clean the string
void clean(char *what) {
    int i;
    for (i = 0; i < MAX_DIR; i++) {
        *(what+i) = 0;
    }
}

void list(const char *name) {
    DIR *dp;
    struct dirent *ep;
    char current[MAX_DIR];
    strcpy(current, name);
    dp = opendir (name);
    strcpy(stack[ptr++], current);

    while (ptr > 0) {
        if (dp != NULL) {
            printf("%s\n", current);
            while ((ep = readdir (dp))) {
                if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0) {
                    continue; // skip the . and .. folders
                }
                // check for matches
                //if (match(query, ep->d_name)) {
                printf("\t%s/%s\n", current, ep->d_name);
                //}

                if (ep->d_type == DT_DIR) {
                    push(current, ep->d_name);
                //    for (int i = 0; i < ptr; i++)
                //        printf("stack: %i | %s\n", i, stack[i]);
                }
            }
        } else {
            printf("Could not access file: %s\n", current);
        }
        printf("\n");
        // assign dp to another path
        clean(current);
        char *temp = pop();
        strcpy(current, temp);
        if (dp != NULL)
            closedir(dp);
        dp = opendir(current);
        //printf("current -> %s\n", current);
    }
}

int main(int argc, char **argv) {
    if (argc == 2)
        list(argv[1]);
    else
        list("/Users/pentahack/Desktop/learn_and_practice/clang/system_projects");
    return 0;
}
