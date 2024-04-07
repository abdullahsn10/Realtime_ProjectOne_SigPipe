

files:
	gcc main.c  -o main 
	gcc team.c  -o team
	gcc opengl.c -o opengl -lglut -lGLU -lGL -lm


run:
	./opengl


clean:
	rm -f main 
	rm -f team
	rm -f teamplayer1.txt
	rm -f teamplayer2.txt
	rm -f opengl


