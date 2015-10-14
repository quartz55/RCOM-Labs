#include "IPA.h"

#include <stdio.h>

int main(int argc, char* argv[]) {
    if (llopen(0, 3) > 0) printf("Success\n");
    else printf("Not successful\n");

    return 0;
}
