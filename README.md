# WHY?

Because i have to know C/C++

# What you are going to create?

Well, it would be cool if i was able to create the whole messenger, lets see
what it will be.

But i think, i can make a program, that will contain the whole level 1 
of top 100 redteam projects: [[https://github.com/kurogai/100-redteam-projects/tree/master]]

# The plan

level 1:

+ [0] TCP or UDP server just to receive messages
+ [1] TCP chat server
+ [2] UDP chat server
+ [3] Multi-threaded UDP or TCP char server (main target)
+ [4] Server for file transfers (i like it)
+ [5] Caesar Cipher tool (only for beginning stage)
+ [6] TCP char server -> The messages should be encoded with Caesar Cipher
+ [7] ROT13 Cipher
+ [8] UDP Chat server -> The messages should be encoded with ROT13 Cipher
+ [9] Remote Command Execution (RCE, also main feature, i would like to create a Trojan)
+ [10] Recreate the Netcat tool (LETS FUCKING GO)

# How to run it?
1. You have to move on to src/client/ and src/server/ dirs\
2. then You have to make build dir: mkdir build\
3. Windows: Now, You move on build dir and run this 2 commands:
 + cmake -G "MinGW Makefiles" ..
 + cmake --build .
4. Now You can run client.exe or server.exe, but server.exe should be first...

