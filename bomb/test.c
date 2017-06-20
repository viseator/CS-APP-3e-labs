#include <stdio.h>
#include <stdint.h>

int main(void){
    FILE* a = fopen("v","a");
    fprintf(a,"\x09\x0f\x0e\x05\x06\x07\n");
    fclose(a);
    return 0;
}