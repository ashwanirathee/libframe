#include <stdio.h>
#include "libframe.h"

int main() {
    // Test transform.c
    int transform_result = transform_hello_world();
    printf("Transform result: %d\n", transform_result);

    // Test neural.c
    int nn_result = nn_hello_world();
    printf("NN result: %d\n", nn_result);

    return 0;
}
