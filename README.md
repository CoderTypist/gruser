# gruser

## Description
Lists all of the groups a user belongs to. Also lists all of the users that belong to a group.

## Usage
gcc -o gruser gruser.c
./gruser

## Motivation
I read chapter 8 of `The Linux Programming Interface` by Michael Kerrisk. The chapter discusses users, groups, and the information contained in `/etc/passwd`, `/etc/shadow` and `/etc/group`. He points out that `/etc/passwd` contains the gid of one group that a user belongs to and that the rest can be found in `/etc/group`. This is kind of annoying. I wanted a way to list all of the groups that a user belongs to. While this task could have been accomplished with a script, I wanted to practice using the C functions and structs introduced in the chapter.
