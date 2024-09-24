# Week 02 Notes

## Shared Libraries vs Static Libraries

Shared:

- Windows: \*.dll
- Mac: \*dylib
- Linux: \*.so

Application only references that library excatly when it needs to at runtime. Shared lib needs to be there in order for an app to run. This is because all the code lives in the shared library. So app crashes without shared library. Since you dont copy paste the code, this keeps your application really small.

Static:

- Windows: \*.lib
- Mac and Linux: \*.a

Program utilizes static library at compile time. This happens before running. You take exactly what you need from static library, take all the code and move that into the application. Here, the code now lives in your application in contrast to haveing the code reference to the library. If you are depended on alot of libraries, this will make your application very bg very fast.
