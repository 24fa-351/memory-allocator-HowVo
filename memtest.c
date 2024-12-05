#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifdef SYSTEM_MALLOC
#define my_free free
#define my_malloc malloc
#define my_realloc realloc
#else
#include "malloc.h"
#endif

int rand_between(int min, int max) { return rand() % (max - min + 1) + min; }

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define TEST_SIZE 10

// 1000 chars paragraph to test memory allocation
char *test_string = "Technology has transformed human communication in profound ways, connecting people instantly across vast distances. The rise of the internet, smartphones, and social media has made it easier than ever to maintain relationships, collaborate, and access information. However, these advancements have also introduced challenges, such as the erosion of privacy, superficial online interactions, and the spread of misinformation. While digital platforms enable rapid communication, they can also lead to feelings of isolation, as virtual connections may lack the depth of face-to-face conversations. Furthermore, the anonymity of online interactions has sometimes encouraged harmful behaviors, like cyberbullying. Despite these concerns, technology continues to shape how we interact, requiring us to find a balance between embracing its benefits and preserving meaningful human connections. As technology evolves, its impact on communication will remain a central issue, prompting us to consider its effects on relationships, society, and individual well-being.";

void test_allocations()
{
    char *ptrs[TEST_SIZE];

    for (int i = 0; i < TEST_SIZE; i++)
    {
        int size = rand_between(1, strlen(test_string) + 1);
        fprintf(stderr, "\n\n\n[%d] size: %d\n", i, size);

        ptrs[i] = my_malloc(size);
        if (ptrs[i] == NULL)
        {
            printf("[%d] malloc failed\n", i);
            fprintf(stderr, "test_allocations() FAILED\n");
            exit(1);
        }

        int len_to_copy = MIN(strlen(test_string), size - 1);

        fprintf(stderr, "[%d] ptrs[%d]: %p, going to copy %d chars\n", i, i,
                ptrs[i], len_to_copy);

        strncpy(ptrs[i], test_string, len_to_copy);
        ptrs[i][len_to_copy] = '\0';

        fprintf(stderr, "[%d] '%s'\n", i, ptrs[i]);
    }
    fprintf(stderr, "test_allocations() PASSED\n");
}

void test_interspersed_frees()
{
    char *ptrs[TEST_SIZE];

    for (int ix = 0; ix < TEST_SIZE; ix++)
    {
        int size = rand_between(1, strlen(test_string) + 1);
        fprintf(stderr, "\n\n\n[%d] size: %d\n", ix, size);

        ptrs[ix] = my_malloc(size);
        if (ptrs[ix] == NULL)
        {
            printf("[%d] malloc failed\n", ix);
            fprintf(stderr, "test_interspersed_frees() FAILED\n");
            exit(1);
        }

        int len_to_copy = MIN(strlen(test_string), size - 1);

        fprintf(stderr, "[%d] ptrs[%d]: %p, going to copy %d chars\n", ix, ix,
                ptrs[ix], len_to_copy);

        strncpy(ptrs[ix], test_string, len_to_copy);
        ptrs[ix][len_to_copy] = '\0';

        fprintf(stderr, "[%d] '%s'\n", ix, ptrs[ix]);

        int index_to_free = rand_between(0, ix);
        if (ptrs[index_to_free])
        {
            fprintf(stderr, "\n[%d] randomly freeing %p ('%s')\n", index_to_free,
                    ptrs[index_to_free], ptrs[index_to_free]);
            my_free(ptrs[index_to_free]);
            fprintf(stderr, "[%d] freed %p\n", index_to_free, ptrs[index_to_free]);
            ptrs[index_to_free] = NULL;
        }
    }

    for (int ix = 0; ix < TEST_SIZE; ix++)
    {
        if (ptrs[ix])
        {
            fprintf(stderr, "[%d] freeing %p (%s)\n", ix, ptrs[ix], ptrs[ix]);
            my_free(ptrs[ix]);
            fprintf(stderr, "[%d] freed %p\n", ix, ptrs[ix]);
        }
        else
        {
            fprintf(stderr, "[%d] already freed\n", ix);
        }
    }
    fprintf(stderr, "test_interspersed_frees() PASSED\n");
}

void test_reallocations()
{
    char *ptrs[TEST_SIZE];

    for (int i = 0; i < TEST_SIZE; i++)
    {
        int size = rand_between(1, strlen(test_string) + 1);
        fprintf(stderr, "\n\n\n[%d] size: %d\n", i, size);

        ptrs[i] = my_malloc(size);
        if (ptrs[i] == NULL)
        {
            printf("[%d] malloc failed\n", i);
            fprintf(stderr, "test_reallocations() FAILED\n");
            exit(1);
        }

        int len_to_copy = MIN(strlen(test_string), size - 1);

        fprintf(stderr, "[%d] ptrs[%d]: %p, going to copy %d chars\n", i, i,
                ptrs[i], len_to_copy);

        strncpy(ptrs[i], test_string, len_to_copy);
        ptrs[i][len_to_copy] = '\0';

        fprintf(stderr, "[%d] '%s'\n", i, ptrs[i]);

        int new_size = rand_between(1, strlen(test_string) + 1);
        fprintf(stderr, "[%d] reallocating to size: %d\n", i, new_size);

        ptrs[i] = my_realloc(ptrs[i], new_size);
        if (ptrs[i] == NULL)
                {
            printf("[%d] realloc failed\n", i);
            fprintf(stderr, "test_reallocations() FAILED\n");
            exit(1);
        }

        len_to_copy = MIN(strlen(test_string), new_size - 1);

        fprintf(stderr, "[%d] ptrs[%d]: %p, going to copy %d chars\n", i, i,
                ptrs[i], len_to_copy);

        strncpy(ptrs[i], test_string, len_to_copy);
        ptrs[i][len_to_copy] = '\0';

        fprintf(stderr, "[%d] '%s'\n", i, ptrs[i]);
    }

    for (int ix = 0; ix < TEST_SIZE; ix++)
    {
        if (ptrs[ix])
        {
            fprintf(stderr, "[%d] freeing %p (%s)\n", ix, ptrs[ix], ptrs[ix]);
            my_free(ptrs[ix]);
            fprintf(stderr, "[%d] freed %p\n", ix, ptrs[ix]);
        }
        else
        {
            fprintf(stderr, "[%d] already freed\n", ix);
        }
    }
    fprintf(stderr, "\n\ntest_reallocations() PASSED\n");
}

int main(int argc, char *argv[])
{

    srand(time(NULL));

    if (argc == 3 && strcmp(argv[1], "-t") == 0)
    {
        int test_num = atoi(argv[2]);
        switch (test_num)
        {
        case 1:
            test_allocations();
            break;
        case 2:
            test_interspersed_frees();
            break;
        case 3:
            test_reallocations();
            break;
        default:
            printf("Invalid test number: %d\n", test_num);
            return 1;
        }
    }
    else
    {
        test_allocations();
        test_interspersed_frees();
        test_reallocations();
    }

    return 0;
}