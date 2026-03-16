#include <stdio.h>

int main() {
    FILE *f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    int temp_mC;
    fscanf(f, "%d", &temp_mC);
    fclose(f);
    printf("CPU temp: %.2f C\n", temp_mC / 1000.0);
    return 0;
}
