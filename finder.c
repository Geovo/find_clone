#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// we will need these headers for traversing the filesystem
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

#define MAX_DIR 512
#define MAX_FOUND 50000
#define MAX_LOCAL 1000

// use this wrapper, just for better reading
#define pop() stack[--ptr];

// in case our system doesn't have this macro defined
#ifndef DT_DIR
#define DT_DIR 8
#endif

char *query;
char stack[MAX_FOUND][MAX_DIR];
int ptr = 0;

// use a function to make it cleaner
static void push(char x[], char y[]) {
    strcpy(stack[ptr], x);
    strcat(stack[ptr], "/");
    strcat(stack[ptr++], y);
}

// M | A | M | A | -> Test with AM
//----------------
// A |   |   |   | -> NOT_MATCHING
//   | A |   |   | -> MATCHING
//   | A | M |   | -> MATCHING
//   | A | M |\0 | -> MATCHING, but we got the end of b
// return true

// M | A | M | A | -> Test with AN
//----------------
// A |   |   |   | -> NOT_MATCHING
//   | A |   |   | -> MATCHING
//   | A | N |   | -> NOT_MATCHING
//   |   | A |   | -> NOT_MATCHING
//   |   |   | A | -> NOT_MATCHING
//   |   |   |   | -> NOT_MATCHING

/* Could break earlier: */
// M | A | M | A | -> Test with AN
//----------------
// A |   |   |   | -> NOT_MATCHING
//   | A |   |   | -> MATCHING
//   | A | N |   | -> NOT_MATCHING
// no match found at len(a) - len(b) + 1 = 3
// That means there can be no more substrings of len(b) in a

// first goes the match function
bool match(char a[], char b[]) {
    // we suppose, that we search for b in a, so that |a| >= |b|
    //printf("checking %s and %s\n", a, b);
    // get the length of a and b
    int len_a = strlen(a);
    int len_b = strlen(b);
    // if you messed up with the order, here's a fix
    if (len_b > len_a)
        return false;
        //return match(b, a);
    int i;
    int j = 0;
    int match_dest = -1;
    // define states
    enum state {MATCHING, NOT_MATCHING} current_state;
    current_state = NOT_MATCHING;

    for (i = 0; i < len_a; i++) {
        //printf("state: %s | i at: %c | j at: %c\n", current_state == MATCHING ? "MATCHING" : "NOT_MATCHING", a[i], b[j]);
        if (b[j] == '\0') {
            //printf("true: j at %s | %d | len: %d\n", b, j, len_b);
            //printf(" | true\n");
            return true;
        }
        if (current_state == MATCHING) {
            // got to the end of b and found match
            if (a[i] != b[j]) {
                current_state = NOT_MATCHING;
                j = 0;
                match_dest = -1;
                continue;
            }
            // by default, a[i] == b[j], so we simply increase j
            j++;
        } else {
            // pre-kill :)
            if (i > len_a  - len_b) {
                //printf("returning false\n");
                return false;
            }
            // check for matches
            if (a[i] == b[j]) {
                current_state = MATCHING;
                j++;
            }
        }
    }
    // we got to the end of a, but there can be a match on the last chars
    if (b[j] == '\0') {
        printf("true at end\n");
        return true;
    }
    //printf("ended at i: %c | j: %c\n", a[i], b[j]);
    //printf(" | false\n");
    return false;
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
            //printf("%s\n", current);
            // create a local 'stack' to fill it with results of folder
            char local[MAX_LOCAL][MAX_DIR];
            int local_ptr = 0;
            while ((ep = readdir (dp))) {
                if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0) {
                    continue; // skip the . and .. folders
                }
                // check for matches
                if (match(ep->d_name, query)) {
                    printf("find: %s/%s\n", current, ep->d_name);
                }

                if (ep->d_type == DT_DIR) {
                    strcpy(local[local_ptr++], ep->d_name);
                //    for (int i = 0; i < ptr; i++)
                //        printf("stack: %i | %s\n", i, stack[i]);
                }
            }
            // push the new names back on the big stack
            for (--local_ptr; local_ptr >= 0; local_ptr--)
                push(current, local[local_ptr]);
        } else {
            printf("Could not access file: %s\n", current);
        }
        //printf("\n");
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
    if (argc == 3) {
        query = malloc(sizeof(char) * strlen(argv[2]));
        strcpy(query, argv[2]);
        list(argv[1]);
        //printf("argv[2] = %s\n", argv[2]);
    } else {
        query = malloc(sizeof(char) * 3);
        strcpy(query, ".c");
        list("/Users/pentahack/Desktop/learn_and_practice/clang/system_projects");
    }
    if (query != NULL)
        free(query);

    return 0;
}
