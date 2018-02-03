bin_PROGRAMS = main
main_SOURCES = renderer.c renderer_image.c renderer_buffer.c \
			   renderer_tools.c renderer_mesh.c game.c main.c
main_CFLAGS  = -g -Wall -Wextra -Wpedantic
main_LDADD = -lglfw3 -lm -ldl -lXinerama -lXrandr -lXcursor -lX11 -lXxf86vm \
			 -lpthread -lvulkan -L/usr/lib -lassimp
