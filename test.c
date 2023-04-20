


#define AUDIT_IMPLEMENTATION


#include "audit.h"
int soma(int a, int b) {
        return a + b;
}


audit("Description of test (group of checks, or asserts)") {
        int r = soma(1, 2);
        check_eq(r, 3, "%i", "Description of check");
}

audit("Test of test") {
        char a ='a';
        check_eq(a, 'a', "%c", "Description of check");
}