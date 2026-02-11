"example.py"
import gl

resX = 256
resY = 256

def main():
	gl.init(name="simfile4", resolution=(resX, resY), version=(4,6)) #Resolution [resX, resY] and OpenGL version 4.60 core.

	cellShader = gl.load_shader(gl.ST_COMPUTE, "shaders/cell.comp") #Load cell compute shader
	displayShader = gl.load_shader(gl.ST_SCREENSPACE, "shader/display.frag") #Load the display shader

	while (gl.window_open()):
		gl.poll_events() #Check for keypresses and whatnot.

		gl.configure(gl.ST_COMPUTE) #Set settings to compute shader style.
		gl.run(cellShader)

		gl.configure(gl.ST_SCREENSPACE) #Set settings to fullscreen shader style.
		gl.run(displayShader)

		gl.update_window() #Update the window with the new visuals


	gl.terminate() #Close everything down.



if (__name__ == "__main__"):
	main()

