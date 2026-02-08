"test.py"
#Used to test the module.

import gl;




def main() -> None:
	print("[PY ] Running test python script;")
	gl.verbose(); #Let it output to console.


	#gl.init(); #Test init func
	gl.init(name="Test Window", resolution=(800, 600), version=(4, 6));
	print("[PY ] Module initialised!");


	shaderID:int = gl.screenspace_shader("shaders/screenspace.frag");
	print(f"[PY ] Screenspace shader [{shaderID}] created!");


	gl.terminate(); #Test shutdown
	print("[PY ] Shut down module!");



if (__name__ == "__main__"):
	try:
		main();
	except Exception as e:
		print(e);