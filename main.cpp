#include "yano.h"

int main() {
    yano::Yano yano = yano::Yano(2560, 1600, "boxxy", 3);
    yano.run();
    return 0;
}