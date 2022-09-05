# gruser

## Description
Lists all of the groups a user belongs to. Also lists all of the users that belong to a group.

## Usage
gcc -o gruser gruser.c
./gruser

## Motivation
I read chapter 8 of `The Linux Programming Interface` by Michael Kerrisk. The chapter discusses users, groups, and the information contained in `/etc/passwd`, `/etc/shadow` and `/etc/group`. He points out that `/etc/passwd` contains the gid of one group that a user belongs to and that the rest can be found in `/etc/group`. This is kind of annoying. I wanted a way to get all of the groups a user belongs to in one place. While a bash script and some tokenizing with awk would have done the trick, I wanted to practice using the C functions and structs introduced in the chapter. Since C lacks in built in HashMaps, I also got to practice creating and using lists.  
