"example.py"
import gl #My custom module

resX = 256
resY = 256
resVec = (resX, resY)

#Values
DEFAULT_TEMPERATURE = 288
DEFAULT_PRESSURE = 1
DEFAULT_HUMIDITY = 500
DEFAULT_VAPOR = 1
default_fill_colour = (DEFAULT_TEMPERATURE, DEFAULT_PRESSURE, DEFAULT_HUMIDITY, DEFAULT_VAPOR)


def main():
	gl.verbose(gl.V_SILENT) #Tell it to not write to the console. Not required, default is V_SILENT, but its more clear.
	gl.init(name="simfile4", resolution=(resX, resY), version=(4,6)) #Resolution [resX, resY] and OpenGL version 4.60 core.

	cellShader = gl.load_shader(gl.ST_COMPUTE, "shaders/cell.example.comp") #Load cell compute shader
	displayShader = gl.load_shader(gl.ST_SCREENSPACE, "shaders/display.example.frag") #Load the display shader


	terrain = gl.load_texture("textures/ground.example.png") #Load the terrain map
	cWorld = gl.create_texture(resVec, fill_colour=default_fill_colour) #Create the 2 grids (must be seperate so they dont share the exact same data)
	fWorld = gl.create_texture(resVec, fill_colour=default_fill_colour) #Also sets their initial values to be the defaults.


	#Tell the shaders to use the textures;
	gl.add_texture(cellShader, terrain, 0)
	gl.add_texture(cellShader, cWorld, 1)
	gl.add_texture(cellShader, fWorld, 2)

	gl.add_texture(displayShader, cWorld, 0) #Show the current grid onscreen.


	#Tell the shader what the grid size is
	gl.add_uniform_value(cellShader, "gridSize", resVec)

	while (gl.is_window_open() and (not gl.is_key_held(gl.KEY_ESCAPE))): #Stays open until the X or ESC key are pressed.
		gl.poll_events() #Check for keypresses and whatnot.

		gl.configure(gl.ST_COMPUTE) #Set settings to compute shader style.
		gl.run(cellShader) #Do the physics stuff

		gl.configure(gl.ST_SCREENSPACE) #Set settings to fullscreen shader style.
		gl.run(displayShader) #Show onscreen

		gl.update_window() #Update the window with the new visuals


	#[OPTIONAL] save current world thing to an image file
	gl.save_texture(cWorld, "textures/cWorld.example.png")

	#[OPTIONAL] delete textures because they arent used
	#If not done, then gl.terminate() does it, but its probably clearer TO do it than not.
	gl.delete_texture(terrain)
	gl.delete_texture(cWorld)
	gl.delete_texture(fWorld)

	gl.terminate() #Close everything down.



if (__name__ == "__main__"):
	main()

