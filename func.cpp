extern "C" void print(const char* message);

extern "C" void HelloWorld(){
    const char* message = "Hello from C++!";
    print(message);
}
