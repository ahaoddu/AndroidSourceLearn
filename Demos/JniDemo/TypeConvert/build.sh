#!/bin/bash
javac -h . HelloJNI.java
g++ -fpic -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libhello.so HelloJNI.cpp
java -Djava.library.path=. HelloJNI