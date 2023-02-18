#include <GL/freeglut.h>
#include "global.h"

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
        
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                // Access to vertex
                index_t idx = shapes[s].mesh.indices[index_offset + v];
                real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                glVertex3f(vx, vy, vz);
            }
            index_offset += fv;
        }
    }
    glEnd();
    glutSwapBuffers();
    
}

void output(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(camera.width, camera.height);
	glutCreateWindow("Monte Carlo Path Tracer");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera.fovy, double(camera.width) / camera.height, 0, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera.eye_x, camera.eye_y, camera.eye_z, camera.lookat_x, camera.lookat_y, camera.lookat_z, camera.up_x, camera.up_y, camera.up_z);

    glutDisplayFunc(display);
    glutMainLoop();
}