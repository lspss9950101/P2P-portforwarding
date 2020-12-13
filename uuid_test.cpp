#include <uuid.h>

int main() {
    for(int i = 0; i < 16; i++) {
        UUID uuid;
        uuid.print();
    }
}