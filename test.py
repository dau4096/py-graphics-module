"test.py"
#Used to test the module.

import gl;




def main() -> None:
    print("[PY ] Running test python script;")
    gl.init(); #Test init func
    #gl.init(name="My Window", resolution=(800, 600), version=(4, 6));
    print("[PY ] Module initialised!");

    gl.terminate(); #Test shutdown
    print("[PY ] Shut down module!");



if (__name__ == "__main__"):
    main();