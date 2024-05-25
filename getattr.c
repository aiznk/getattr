#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static void
usage(void) {
    printf(
        "Get tag attribute value.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    getattr [tag-name] [attr-key]...\n"
        "\n"
        "Example:\n"
        "\n"
        "    cat index.html | getattr h1 id class\n"
        "\n"
    );
    exit(0);
}

typedef struct {
    bool error;
    const char *msg;
} Error;

void
show(int argc, char *argv[], 
    const char *name, const char *key, const char *val) {
    #if defined(DEBUG)
        printf("name[%s] key[%s] val[%s]\n", name, key, val);
    #endif
    if (strcmp(argv[0], name) != 0) {
        return;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], key)) {
            puts(val);
        }
    }
}

void
read_tag(int argc, char *argv[], Error *err) {
    int m = 0;
    char name[1024];  // tag name
    size_t ni = 0;
    char key[1024];  // attr key
    size_t ki = 0;
    char val[1024];  // attr value
    size_t vi = 0;

    #define reset() do {\
        ki = 0;\
        key[ki] = 0;\
        vi = 0;\
        val[vi] = 0;\
    } while (0)\

    #define push(dst, idx, c, errmsg) do {\
        if (idx >= sizeof dst) {\
            err->error = true;\
            err->msg = errmsg;\
            goto fail;\
        }\
        dst[idx++] = c;\
        dst[idx] = 0;\
    } while (0)\

    for (;;) {
        int c = fgetc(stdin);

        #if defined(DEBUG)
            printf("m[%d] c[%c]\n", m, c);
        #endif

        switch (m) {
        case 0:
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                // pass
            } else {
                push(name, ni, c, "Name buffer overflow");
                m = 10;
            }
            break;
        case 10:  // -> name
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                m = 20;
            } else if (c == '=') {
                m = 30;
            } else {
                push(name, ni, c, "Name buffer overflow (2)");
            }
            break;
        case 20:  // -> name -> space
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                // pass
            } else {
                push(key, ki, c, "Key buffer overflow");
                m = 25;
            }
            break;
        case 25:  // -> name -> space -> key
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                m = 27;
            } else if (c == '=') {
                m = 30;
            } else {
                push(key, ki, c, "Key buffer overflow (2)");
            }
            break;
        case 27: // -> name -> space -> key -> space
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                // pass
            } else if (c == '=') {
                m = 30;
            } else {
                err->error = true;
                err->msg = "Unknown state";
            }
            break;
        case 30:  // -> name -> space -> key -> '='
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                m = 40;
            } else if (c == '"') {
                m = 50;
            } else {
                push(val, vi, c, "Value buffer overflow");
                m = 60;
            }
            break;
        case 40:  // -> name -> space -> key -> '=' -> space
            if (c == '>') {
                goto done;
            } else if (isspace(c)) {
                // pass
            } else if (c == '"') {
                m = 50;
            } else {
                push(val, vi, c, "Value buffer overflow (2)");
                m = 60;
            }
            break;
        case 50:  // -> name -> space -> key -> '=' -> '"'
            if (c == '"') {
                show(argc, argv, name, key, val);
                reset();
                m = 20;
            } else if (c == '\\') {
                c = fgetc(stdin);
                push(val, vi, c, "Value buffer overflow (3)");
            } else {
                push(val, vi, c, "Value buffer overflow (4)");
            }
            break;
        case 60:  // -> name -> space -> key -> '=' -> ?
            if (c == '>') {
                show(argc, argv, name, key, val);
                goto done;
            } else if (isspace(c)) {
                show(argc, argv, name, key, val);
                reset();
                m = 20;
            } else {
                push(val, vi, c, "Value buffer overflow (5)");
            }
            break;
        }
    }

done:
    ;
fail:
    ;
}

void
getattr(int argc, char *argv[], Error *err) {
    for (;;) {
        int c = fgetc(stdin);
        if (c == EOF) {
            break;
        }

        if (c == '<') {
            read_tag(argc, argv, err);
            if (err->error) {
                goto fail;
            }
        }
    }

fail:
    ;
}

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
    }

    Error err = {0};
    getattr(argc - 1, argv + 1, &err);
    if (err.error) {
        fprintf(stderr, "Error: %s.\n", err.msg);
        return 1;
    }

    return 0;
}
