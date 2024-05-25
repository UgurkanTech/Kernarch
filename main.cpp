#include <iostream>

// Declare the external assembly function
extern "C" int AddNumbers(int a, int b);

int main() {
    int result = AddNumbers(5, 7);
    std::cout << "Result: " << result << std::endl;
    return 0;
}
