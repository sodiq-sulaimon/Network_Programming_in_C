#include <stdio.h>
#include <time.h>

int main() {
time_t currentTime;
time(&currentTime);

printf("The current time is: %s\n", ctime(&currentTime));

return 0;
}



