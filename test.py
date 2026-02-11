"test.py"
#Used to test the module.

import gl;



######## ANSI COLOURS ########


class Colours:
	ERROR:str 		= "\033[1;31m"; #Red
	SUCCESS:str		= "\033[1;32m"; #Green
	WARNING:str 	= "\033[1;33m"; #Yellow
	MINOR:str 		= "\033[0;34m"; #Dark-grey
	MAJOR:str 		= "\033[0;36m"; #White
	VALUE:str 		= "\033[0;35m"; #Purple
	DEFAULT:str 	= "\033[0;39m"; #Reset


######## ANSI COLOURS ########





######## UNIT TESTS ########


def screenspaceShader() -> None:
	print(f"{Colours.MAJOR}[PY ] Testing Screenspace Shader;{Colours.MINOR}");
	gl.configure(gl.ST_SCREENSPACE); #Set to use settings for screenspace shaders

	#Load shader from file.
	shaderID:int = gl.load_shader(gl.ST_SCREENSPACE, "shaders/uv.2D.frag");
	assert shaderID != -1, "Screenspace Shader failed to be loaded.";

	#Attempt to bind a unifom value
	successUniform:bool = gl.add_uniform_value(shaderID, "test", 0);
	assert successUniform, "Failed to assign value to shader uniform";

	#Check for shader run-errors.
	successRun:bool = gl.run(shaderID);
	assert successRun, "Failed to run Screenspace Shader.";
	gl.update_window();


	print(f"{Colours.SUCCESS}[PY ] Screenspace Shader Tests Passed{Colours.MINOR}");


########            ########


def computeShader() -> None:
	print(f"{Colours.MAJOR}[PY ] Testing Compute Shader;{Colours.MINOR}");
	gl.configure(gl.ST_COMPUTE); #Set to use settings for compute shaders

	#Load shader from file.
	shaderID:int = gl.load_shader(gl.ST_COMPUTE, "shaders/compute.comp");
	assert shaderID != -1, "Compute Shader failed to be loaded.";

	#Test different dispatch sizes
	success1:bool = gl.run(shaderID, [1, 1, 1]);
	assert success1, "Failed to run Compute Shader [1, 1, 1].";
	success16:bool = gl.run(shaderID, [16, 16, 16]);
	assert success16, "Failed to run Compute Shader [16, 16, 16].";
	success0:bool = gl.run(shaderID, [0, 0, 0]);
	assert success0, "Failed to run Compute Shader [0, 0, 0].";


	print(f"{Colours.SUCCESS}[PY ] Compute Shader Tests Passed{Colours.MINOR}");


########            ########


def worldspaceShader(pvmMatrix:list[float]) -> None:
	print(f"{Colours.MAJOR}[PY ] Testing Worldspace Shader;{Colours.MINOR}");
	gl.configure(gl.ST_WORLDSPACE); #Set to use settings for worldspace shaders

	#Load shader from files.
	shaderID:int = gl.load_shader(gl.ST_WORLDSPACE, "shaders/worldspace.vert", "shaders/uv.3D.frag");
	assert shaderID != -1, "Worldspace Shader failed to be loaded.";

	#Bind a VAO with positions & 2D UV.
	vertices:list[float] = [
		#Example dataset of vertices.
		-1.0,  1.0,  0.0,	 0.0, 0.0,
		 1.0,  1.0,  0.0,    1.0, 0.0,
		 0.0,  1.0,  1.0,    0.0, 1.0,
	];
	gl.add_vao(shaderID, gl.VAO_POS_UV2D, vertices);

	#Check for shader run-errors.
	successRun:bool = gl.run(shaderID);
	assert successRun, "Failed to run Worldspace Shader.";
	gl.update_window();


	print(f"{Colours.SUCCESS}[PY ] Worldspace Shader Tests Passed{Colours.MINOR}");


######## UNIT TESTS ########






def main() -> None:
	print(f"{Colours.WARNING}[PY ] Running test python script;{Colours.MINOR}");
	gl.verbose(); #Let it output to console.


	#Test init func
	gl.init(name="Test Window", resolution=(800, 600), version=(4, 6));
	print(f"{Colours.SUCCESS}[PY ] Module initialised.{Colours.MINOR}");
	assert gl.window_open(), "Window failed to initialise.";


	#Test running different shader types
	screenspaceShader();
	computeShader();
	worldspaceShader([]);
	

	"""
	while (gl.window_open() and (not gl.is_key_held(gl.KEY_ESCAPE))): #Example Main program loop.
		gl.poll_events(); #Look for events

		#Run stages of frame
		gl.run(worldspaceShaderID);
		gl.run(screenspaceShaderID);

		gl.update_window(); #Update the screen with the next frame.
	"""


	gl.terminate(); #Test shutdown
	print(f"{Colours.WARNING}[PY ] Module terminated {Colours.DEFAULT}:{Colours.SUCCESS} All tests pass.{Colours.DEFAULT}");



if (__name__ == "__main__"):
	try:
		main();
	except Exception as e:
		print(f"{Colours.ERROR}[ERR] {e}{Colours.DEFAULT}");