// lib_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "xmlparse.h"
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>

void fatal_err(int err = -1) {
    std::cout << "Fatal error: " << err << std::endl;
    exit(err);
}

void print_trace() {
    DWORD type = IMAGE_FILE_MACHINE_AMD64;
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    CONTEXT context;
    STACKFRAME64 stack;
    PREAD_PROCESS_MEMORY_ROUTINE64 read_mem=nullptr;

    //SymInitialize(hProcess, NULL, true);
    //RtlCaptureContext(&context);
    ZeroMemory(&stack, sizeof(stack));

    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rsp;
    stack.AddrFrame.Mode = AddrModeFlat;
    return;
    while (1) {
        bool r = StackWalk64(type, hProcess, hThread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
        if (!r){
            fatal_err(-2);
        }
        int o = stack.AddrPC.Offset;
        if (o) {
            std::cout << o << '\n';
        }
        else {
            break;
        }
    }

        
}

void test5() {
    /* todo */
}

void test4() {
    test5();
}

void test3() {
    test4();
}

void test2() {
    test3();
}

void test1() {
    test2();
}



void xml_test(const char *fn) {
    auto r = XML::fromfn(fn);
    XML::dump(r);
}

int main(int argc, char **argv)
{
    try
    {
        xml_test(argv[1]);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what();
    }
    return 0;
}

