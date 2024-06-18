/*

extern "C" void print(const char* message);

extern "C" void HelloWorld(){
    const char* message = "Hello from C++!";
    print(message);
}


#include <iostream>

// Declare the external assembly function
extern "C" int AddNumbers(int a, int b);

int main() {
    int result = AddNumbers(5, 7);
    std::cout << "Result: " << result << std::endl;
    return 0;
}



section .text
    global AddNumbers

AddNumbers:

    add rcx, rdx
	mov rax, rcx
    ret
*/


