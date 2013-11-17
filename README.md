hexin
=====

The Hexin terminal, is used to inject hexadecimal code, directly to the program input.

For example when we are using gdb, we can inject directly the hexadecimal code. In both cases, the program is receiving the same input:
```
 whats@x61s:/tmp/svn-hexin$ gdb -q /bin/ls <br>
 (no debugging symbols found)
 Using host libthread_db library "/lib/libthread_db.so.1".
 (gdb) run $(printf '\x41\x42\x43\x44')

 whats@x61s:/tmp/svn-hexin$ ./hexin 
 Hexin Terminal v0.2 - z0nKT1g3r & whats
 [whats@x61s svn-hexin]# gdb -q /bin/ls
 (no debugging symbols found)
 Using host libthread_db library "/lib/libthread_db.so.1".
 (gdb) run \x41\x42\x43\x44
```

This is useful when you cant use thinks like $(printf ....), to test for format strings vulnerabilities between some standard imput, etc.

You can download the program from the subversion repository.

For any questions, please contact to whats[@t]wekk.net or wantstar[@t]hotmail.com
