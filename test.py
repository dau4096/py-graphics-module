"test.py"
#Used to test the module.

import glm;
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








######## GENERAL TESTING ########


cam:dict[str, glm.vec3] = {
	"position": glm.vec3(0.0, -1.0, 0.0),
	"angle": glm.vec3(0.0, 0.0, 0.0)
};
CURSOR_SPEED:float = 0.0025;
MOVE_SPEED:float = 0.1;

def updateCamera(cameraID:int) -> None:
	#Get cursor & keyboard inputs and upd the camera.
	forward:glm.vec3 = glm.vec3(gl.get_camera_direction(cameraID, gl.CD_FORWARD));
	right:glm.vec3 = glm.vec3(gl.get_camera_direction(cameraID, gl.CD_RIGHT));
	up:glm.vec3 = glm.vec3(gl.get_camera_direction(cameraID, gl.CD_UP));
	fb:int = 0; lr:int = 0; ud:int = 0;

	if (gl.is_key_held(gl.KEY_W)): fb += 1;
	if (gl.is_key_held(gl.KEY_S)): fb -= 1;
	if (gl.is_key_held(gl.KEY_D)): lr += 1;
	if (gl.is_key_held(gl.KEY_A)): lr -= 1;
	if (gl.is_key_held(gl.KEY_E)): ud += 1;
	if (gl.is_key_held(gl.KEY_Q)): ud -= 1;


	cam["position"] += forward * MOVE_SPEED * fb;
	cam["position"] += right * MOVE_SPEED * lr;
	cam["position"] += up * MOVE_SPEED * ud;


	if (gl.is_key_held(gl.KEY_1)): gl.show_cursor();
	else:
		gl.hide_cursor();
		cursorDelta:glm.vec2 = glm.vec2(gl.get_cursor_movement());
		delta:glm.vec3 = glm.vec3(cursorDelta.x, -cursorDelta.y, 0.0);
		cam["angle"] += delta * CURSOR_SPEED;


	gl.set_new_camera_position(cameraID, cam["position"]);
	gl.set_new_camera_angle(cameraID, cam["angle"]);



def doExample3D() -> None:
	#Create a new window & camera;
	gl.init(name="Example 3D scene", resolution=(800, 600), version=(4, 6));
	cameraID:int = gl.create_camera(fov_deg=70.0, near_z=0.1, far_z=100.0);

	#Load the shader.
	shaderID:int = gl.load_shader(gl.ST_WORLDSPACE, "shaders/worldspace.vert", "shaders/uv.3D.frag");

	#Bind a VAO with positions & 2D UV.
	vertices:list[float] = [
		#Example dataset of vertices.
		-1.0,  1.0,  0.0,	 0.0, 0.0,
		 1.0,  1.0,  0.0,    1.0, 1.0,
		 0.0,  1.0,  1.0,    1.0, 0.0,
		 0.0,  1.0, -1.0,    0.0, 1.0,
	];
	indices:list[int] = [
		#Singular triangle.
		0, 1, 2,
		0, 1, 3,
	];
	#Add the VAO to the shader;
	gl.add_vao(shaderID, gl.VAO_POS_UV2D, vertices, indices);


	#Load the texture to be used;
	textureID:int = gl.load_texture("textures/a.png", "loaded_texture");
	gl.add_texture(shaderID, textureID, 0);


	#Create unchanging matrices;
	projMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_PERSPECTIVE, cameraID));
	modlMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_IDENTITY)); #Identity for now.


	gl.verbose(gl.V_SILENT);
	while (gl.is_window_open() and (not gl.is_key_held(gl.KEY_ESCAPE))): #Example Main program loop.
		gl.poll_events(); #Look for events

		#Run stages of frame
		#Update the camera
		updateCamera(cameraID);

		#Matrix creation
		viewMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_VIEW, cameraID));
		pvmMatrix:glm.mat4 = projMat * viewMat * modlMat;
		gl.add_uniform_value(shaderID, "pvmMatrix", pvmMatrix);

		gl.run(shaderID);

		gl.update_window(); #Update the screen with the next frame.


	gl.delete_camera(cameraID);
	gl.delete_texture(textureID);
	gl.terminate(); #Close after.

######## GENERAL TESTING ########








######## UNIT TESTS ########


def screenspaceShader() -> None:
	print(f"{Colours.MAJOR}[PY ] Testing Screenspace Shader;{Colours.MINOR}");
	gl.configure(gl.ST_SCREENSPACE); #Set to use settings for screenspace shaders

	#Load shader from file.
	shaderID:int = gl.load_shader(gl.ST_SCREENSPACE, "shaders/uv.2D.frag");
	assert (shaderID != -1), "Screenspace Shader failed to be loaded.";

	#Attempt to bind some unifom values
	successUniformScalar:bool = gl.add_uniform_value(shaderID, "testsca", 0);
	assert successUniformScalar, "Failed to assign scalar value to shader uniform";
	successUniformVector:bool = gl.add_uniform_value(shaderID, "testvec", glm.vec3(1, 1, 1));
	assert successUniformVector, "Failed to assign vector value to shader uniform";

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
	assert (shaderID != -1), "Compute Shader failed to be loaded.";

	#Test different dispatch sizes
	success1:bool = gl.run(shaderID, [1, 1, 1]);
	assert success1, "Failed to run Compute Shader [1, 1, 1].";
	success16:bool = gl.run(shaderID, [16, 16, 16]);
	assert success16, "Failed to run Compute Shader [16, 16, 16].";
	success0:bool = gl.run(shaderID, [0, 0, 0]);
	assert success0, "Failed to run Compute Shader [0, 0, 0].";


	print(f"{Colours.SUCCESS}[PY ] Compute Shader Tests Passed{Colours.MINOR}");


########            ########


def worldspaceShader(cameraID:int) -> None:
	#Test matrices work;
	print(f"{Colours.MAJOR}[PY ] Testing matrices{Colours.MINOR}");
	projMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_PERSPECTIVE, cameraID));
	viewMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_VIEW, cameraID));
	modlMat:glm.mat4 = glm.mat4(gl.get_matrix(gl.MAT_IDENTITY)); #Identity for now.
	pvmMatrix:glm.mat4 = projMat * viewMat * modlMat;


	print(f"{Colours.MAJOR}[PY ] Testing Worldspace Shader;{Colours.MINOR}");
	gl.configure(gl.ST_WORLDSPACE); #Set to use settings for worldspace shaders

	#Load shader from files.
	shaderID:int = gl.load_shader(gl.ST_WORLDSPACE, "shaders/worldspace.vert", "shaders/uv.3D.frag");
	assert (shaderID != -1), "Worldspace Shader failed to be loaded.";

	#Bind a VAO with positions & 2D UV.
	vertices:list[float] = [
		#Example dataset of vertices.
		-1.0,  1.0,  0.0,	 0.0, 0.0,
		 1.0,  1.0,  0.0,    1.0, 0.0,
		 0.0,  1.0,  1.0,    0.0, 1.0,
	];
	gl.add_vao(shaderID, gl.VAO_POS_UV2D, vertices);


	#Add the PVM matrix as a uniform value
	successUniform:bool = gl.add_uniform_value(shaderID, "pvmMatrix", pvmMatrix);
	assert successUniform, "Failed to assign value to shader uniform";

	#Check for shader run-errors.
	successRun:bool = gl.run(shaderID);
	assert successRun, "Failed to run Worldspace Shader.";
	gl.update_window();


	print(f"{Colours.SUCCESS}[PY ] Worldspace Shader Tests Passed{Colours.MINOR}");


######## UNIT TESTS ########






def main() -> None:
	print(f"{Colours.WARNING}[PY ] Running test python script;{Colours.MINOR}");
	gl.verbose(gl.V_MINIMAL); #Let it output to console.


	#Test init func
	gl.init(name="Test Window", resolution=(800, 600), version=(4, 6));
	print(f"{Colours.SUCCESS}[PY ] Module initialised.{Colours.MINOR}");
	assert gl.is_window_open(), "Window failed to initialise.";

	cameraID:int = gl.create_camera(fov_deg=70.0, near_z=0.1, far_z=100.0); #Create new camera
	assert (cameraID != -1), "Camera failed to be created";

	

	#Testing changing camera values
	gl.set_new_camera_position(cameraID, glm.vec3(0.0, 0.0, 0.0));
	gl.set_new_camera_angle(cameraID, glm.vec3(0.0, 0.0, 0.0));
	gl.set_new_camera_fov(cameraID, fov_deg=50.0); #Using degrees
	gl.set_new_camera_fov(cameraID, fov_rad=1.57); #Using radians
	gl.set_new_camera_clip(cameraID, z_near=0.01, z_far=10.0);



	#Test loading a texture & creating an image;
	print(f"{Colours.MAJOR}[PY ] Testing loading an image from file{Colours.MINOR}");
	s2D:int = gl.load_texture("textures/a.png");
	assert (s2D != -1), "Failed to load image from file";

	print(f"{Colours.MAJOR}[PY ] Testing creating an image2D{Colours.MINOR}");
	i2D:int = gl.create_texture(glm.ivec2(128, 128));
	assert (i2D != -1), "Failed to create image2D";

	print(f"{Colours.MAJOR}[PY ] Testing texture deletion{Colours.MINOR}");
	gl.delete_texture(s2D); #Delete the loaded image.

	print(f"{Colours.MAJOR}[PY ] Testing texture saving to file{Colours.MINOR}");
	gl.save_texture(i2D, "textures/test.out.png"); #Save the image2D to a file.



	#Test running different shader types
	screenspaceShader();
	computeShader();
	worldspaceShader(cameraID);

	#Trying to delete the camera.
	gl.delete_camera(cameraID);

	print(f"{Colours.SUCCESS}[PY ] Success : All tests pass.{Colours.MINOR}");

	gl.terminate(); #Test shutdown
	print(f"{Colours.WARNING}[PY ] Module testing terminated {Colours.DEFAULT}");
	



	print(f"{Colours.MAJOR}[PY ] Testing controllable, realtime 3D scene{Colours.MINOR}");
	doExample3D();
	print(f"{Colours.SUCCESS}[PY ] Worldspace Shader Tests Passed{Colours.MINOR}");





if (__name__ == "__main__"):
	#try:
		main();
	#except Exception as e:
		#print(f"{Colours.ERROR}[ERR] {e}{Colours.DEFAULT}");
