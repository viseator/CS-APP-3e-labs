#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int s, E, b;
int verbose = 0;
int hit, miss, evict;

typedef struct node {
    int size;
    unsigned long tag;
    struct node *next;
} Node;

void printlongbits(unsigned long x) {
    for (int i = sizeof(x) << 3; i; i--)
        putchar('0' + ((x >> (i - 1)) & 1));
}

unsigned long build_mask(int n) {
    unsigned long result = 0;
    while (n--) {
        result = (result << 1) | 1;
    }
    return result;
}

unsigned long get_index(unsigned long addr) {
    unsigned long blockAddr = addr >> b;
    blockAddr = blockAddr & build_mask(s);
    return blockAddr;
}

unsigned long get_tag(unsigned long addr) {
    unsigned long mask = -1 ^ build_mask(b + s);
    return addr & mask;
}

Node *cache;

void handle(char c, unsigned long addr) {
    Node *header = cache + get_index(addr);
    Node *cur = header;
    unsigned long tag = get_tag(addr);
    while (cur->next) {
        if (cur->next->tag == tag) {
            printf("hit");
            hit++;
            if (cur == header) {
                return;
            }
            Node *next = cur->next->next;
            cur->next->next = header->next;
            header->next = cur->next;
            cur->next = next;
            return;
        }
        cur = cur->next;
    }
    if (header->size < E) {
        printf("miss");
        miss++;
        Node *new = (Node *)calloc(1, sizeof(Node));
        new->tag = get_tag(addr);
        new->next = header->next;
        header->next = new;
        header->size++;
    } else {
        printf("miss eviction");
        miss++;
        evict++;
        cur = header;
        while (cur->next->next != NULL) {
            cur = cur->next;
        }
        free(cur->next);
        cur->next = NULL;

        Node *new = (Node *)calloc(1, sizeof(Node));
        new->tag = get_tag(addr);
        new->next = header->next;
        header->next = new;
    }
}

int main(int argc, char **argv) {
    char op;
    FILE *fp;
    char *line;
    ssize_t read;
    size_t len = 0;

    while ((op = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        switch (op) {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            fp = fopen(optarg, "r");
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            break;
        }
    }

    cache = (Node *)calloc(1 << s, sizeof(Node));

    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[0] != ' ') {
            continue;
        }
        char *pos;
        if ((pos = strchr(line, '\n')) != NULL) {
            *pos = '\0';
        }
        char c;
        unsigned long addr;
        sscanf(line, " %c %lx,%*d", &c, &addr);
        if (verbose) {
            printf("%s ", line);
        }

        switch (c) {
        case 'L':
            handle(c, addr);
            break;
        case 'S':
            handle(c, addr);
            break;
        default:
            handle(c, addr);
            printf(" ");
            handle(c, addr);
            break;
        }
        if (verbose) {
            printf("\n");
        }
    }
    fclose(fp);
    printSummary(hit, miss, evict);
    return 0;
}
