//
//  Nitro Patcher.cpp
//  Nitro Patcher, patching Redline 1.0.5
//
//  Created by maride on 25.10.14.
//
//  Distributed under the PIZZA LICENSE:
//    Do whatever you want to do with this code. It's free of charge and without any copyright.
//    Use it, compile it, modify it, simply do whatever you want.
//    However, if you enjoy this code, you can buy me a pizza as "thank-you". ;)
//      -- maride
//  
//  Simply NOOP-ing 0x359E2 0...+4
//
//  Compile:
//    $ g++ "Nitro Patcher.cpp" -o "Nitro Patcher"
//  Execute:
//    $ ./Nitro\ Patcher /Applications/Redline.app

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

using namespace std;

int main(int argc, const char * argv[]) {
    setuid(0);
    
    if(argc<2) {
        printf("err: argv[0] needs to be set to Redline.app path.\n");
        printf("     Simply drag this executable and the Redline.app into the Terminal. :)\n");
        printf("     A correct call *could* look like this:\n");
        printf("       $ /Users/someone/Downloads/Nitro\\ Patcher /Applications/Redline.app\n");
        return 2;
    }

    printf("Nitro Patcher by GK\nhttps://github.com/RedlineLegacy/Nitro\n\n");

    // Stage I: Copying and Stripping
    string cmd = "cd '";
    cmd += argv[1];
    cmd += "/Contents/MacOS'; if [ ! -f \"Vanilla\" ]; then cp Redline Redline.vanilla; fi; lipo Redline -extract i386 -output Redline";
    /*
     Extended information:
     Redline is an Universal Binary, meaning it contains code for PPC and Intel.
     Needless to say that Mavericks doesn't support PPC. That means we can
     remove the PPC code from the executable, which reduces the file size from
     5MB to ~1MB. That's also nice for patching purposes. `lipo` strips the i386
     (Intel) code out of the old binary.
    */
    system(cmd.c_str());
    
    
    // Stage II: Searching and Patching
    long buffer;
    long offset = 0x359E7;
    long proof  = 0x3C698BE8;
    long repl   = 0x9090909090;
    
    string path = argv[1];
    path += "/Contents/MacOS/Redline";
    FILE *redlineBinary = fopen(path.c_str(),"r+");
    fseek(redlineBinary,offset,SEEK_SET);
    fread(&buffer,4,1,redlineBinary);
    if(buffer == proof) {
        printf("Found unpatched code at 0x359E7, patching...\n");
        fseek(redlineBinary,offset,SEEK_SET);
        fwrite(&repl,5,1,redlineBinary);
    } else if(buffer == 0x90909090) {
        printf("This Redline is already patched.\n");
        return 0;
    } else {
        printf("At 0x359E7 0...+4: searched %li, found %li\nExiting.\n",proof,buffer);
        return 3;
    }
    fclose(redlineBinary);
    /*
     Extended information:
     This replaces the call for two functions.
     The first displays the error message, the
     second terminates Redline.
     0x90 is the op-code for NOOP, so the CPU
     does exactly nothing at this position.
    */
    
    
    // Stage III: Signing the patched code
    cmd  = "codesign --deep -f -s - '";
    cmd += argv[1];
    cmd += "/Contents/MacOS/Redline'";
    system(cmd.c_str());
    /*
     Extended information:
     Codesigning exists since... Good question.
     It exists some very long time. However,
     since 10.9, Apple *requires* running code
     to be signed (thx Gatekeeper).
     We altered our code, so we need to re-sign
     the code.
    */

     printf("\nDone! Happy racing. 😊\n");
}
