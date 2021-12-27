package com.nomone.std_lib;

public class StdLibNatives {

    static { System.loadLibrary("NOMoneStdLib"); }

    // Necessary to trigger running the static code. Could also use Class.forName instead of calling a method.
    public static void start() {}
}
