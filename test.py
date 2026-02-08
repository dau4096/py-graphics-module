"test.py"
#Used to test the module.

import gl;




def main() -> None:
	print("[PY ] Running test python script;")
	gl.verbose(); #Let it output to console.


	#gl.init(version=(4,6)); #Test init func
	gl.init(name="Test Window", resolution=(800, 600), version=(4, 6));
	print("[PY ] Module initialised!");


	sSpaceShaderID:int = gl.load_shader(gl.ST_SCREENSPACE, "shaders/uv.2D.frag");
	print(f"[PY ] Screenspace shader [ID: {sSpaceShaderID}] created!");


	wSpaceShaderID:int = gl.load_shader(gl.ST_WORLDSPACE, "shaders/worldspace.vert", "shaders/uv.3D.frag");
	print(f"[PY ] Worldspace shader  [ID: {wSpaceShaderID}] created!");


	computeShaderID:int = gl.load_shader(gl.ST_COMPUTE, "shaders/compute.comp");
	print(f"[PY ] Compute shader  [ID: {computeShaderID}] created!");



	while (gl.window_open() and (not gl.is_key_held(gl.KEY_ESCAPE))): #Main program loop.
		gl.poll_events(); #Look for events
		gl.update_window(); #Update the screen with the next frame.


	gl.terminate(); #Test shutdown
	print("[PY ] Shut down module!");



if (__name__ == "__main__"):
	try:
		main();
	except Exception as e:
		print(e);