#include <LocalFile.h>
#include <CgProgram.h>

static int mainWindow;
static CgProgram cgProgram;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glut.h>

#define _USE_MATH_DEFINES   /* Needed in order to be able to access M_PI constants from math.h */
#include <math.h>

/* C code, made for tabs of 8 spaces
* uint8_t is defined in the standard C header stdint.h
*/

/* Define numbers and flags */
#define MAX_CHCOUNT			8	/* Max children count */
#define BONE_ABSOLUTE_ANGLE		0x01	/* Bone angle is absolute or relative to parent */
#define BONE_ABSOLUTE_POSITION		0x02	/* Bone position is absolute in the world or relative to the parent */
#define BONE_ABSOLUTE			(BONE_ABSOLUTE_ANGLE | BONE_ABSOLUTE_POSITION)
#define MAX_BONECOUNT			20
#define MAX_KFCOUNT				30
#define MAX_FRAMES				100

typedef unsigned char uint8_t;
typedef unsigned long int uint32_t;

#define RAD2DEG (180.0/M_PI)

typedef struct
{
	uint32_t time;
	float angle, length;
} Keyframe;

typedef struct _Bone
{
	char name[20];				/* Just for the sake of the example */
	float x,				/* Starting point x */
		y,				/* Starting point y */
		a,				/* Angle, in radians */
		l,				/* Length of the bone */
		offA,			/* Offsets measures for angle and length */
		offL;

	uint8_t flags;				/* Bone flags, 8 bits should be sufficient for now */
	uint8_t childCount;			/* Number of children */

	struct _Bone *child[MAX_CHCOUNT],	/* Pointers to children */
		*parent;			/* Parent bone */
	uint32_t keyframeCount;	/* Number of keyframes */
	Keyframe keyframe[MAX_KFCOUNT];	/* Animation for this bone */
} Bone;

Bone *root;

char *currentName = NULL;
int nameIndex = 0;
char names[MAX_BONECOUNT][20];
int animating = 0;
int frameNum = 0;

/* Create a bone and return it's address */
Bone *boneAddChild(Bone *root, float x, float y, float a, float l, unsigned int flags, char *name)
{
	Bone *t;
	int i;

	if (!root) /* If there is no root, create one */
	{
		if (!(root = (Bone *)malloc(sizeof(Bone))))
			return NULL;
		root->parent = NULL;
	}
	else if (root->childCount < MAX_CHCOUNT) /* If there is space for another child */
	{
		/* Allocate the child */
		if (!(t = (Bone *)malloc(sizeof(Bone))))
			return NULL; /* Error! */

		t->parent = root; /* Set it's parent */
		root->child[root->childCount++] = t; /* Increment the childCounter and set the pointer */
		root = t; /* Change the root */
	}
	else /* Can't add a child */
		return NULL;

	/* Set data */
	root->x = x;
	root->y = y;
	root->a = a;
	root->l = l;
	root->flags = flags;
	root->childCount = 0;
	root->keyframeCount = 0;
	root->offA = 0;
	root->offL = 0;

	if (name)
		strcpy(root->name, name);
	else
		strcpy(root->name, "Bone");

	for (i = 0; i < MAX_CHCOUNT; i++)
		root->child[i] = NULL;

	return root;
}

/* Free the bones */
Bone *boneFreeTree(Bone *root)
{
	int i;

	if (!root)
		return NULL;

	/* Recursively call this function to free subtrees */
	for (i = 0; i < root->childCount; i++)
		boneFreeTree(root->child[i]);

	free(root);

	return NULL;
}

/* Dump on stdout the bone structure. Root of the tree should have level 1 */
void boneDumpTree(Bone *root, uint8_t level)
{
	uint32_t i;

	if (!root)
		return;

	for (i = 0; i < level; i++)
		printf("#"); /* We print # to signal the level of this bone. */

	printf(" %4.4f %4.4f %4.4f %4.4f %d %s\n", root->x, root->y, root->a, root->l, root->flags, root->name);

	/* Now print animation info */
	for (i = 0; i < root->keyframeCount; i++)
		printf(" %d %4.4f %4.4f", root->keyframe[i].time, root->keyframe[i].angle, root->keyframe[i].length);
	printf("\n");

	/* Recursively call this on my children */
	for (i = 0; i < root->childCount; i++)
		boneDumpTree(root->child[i], level + 1);
}

Bone *boneLoadStructure(char *path)
{
	Bone *root,		/* The root of the tree to load */
		*temp;		/* A temporary root */

	FILE *file;		/* File to load */

	float x,		/* Bone data */
		y,
		angle,
		length;

	int depth,		/* Depth retrieved from file */
		actualLevel,	/* Actual depth level */
		flags;		/* Bone flags */
		//count;

	uint32_t time;

	char name[20],		/* Buffers for strings */
		depthStr[20],
		animBuf[1024],
		buffer[512],
		*ptr,
		*token;

	Keyframe *k;

	if (!(file = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file %s for reading\n", path);
		return NULL;
	}

	root = NULL;
	temp = NULL;
	actualLevel = 0;

	while (!feof(file))
	{
		memset(animBuf, 0, 1024);

		/* Read a row from the file (I hope that 1024 characters are sufficient for a row) */
		fgets(buffer, 1024, file);

		/* Get the info about this bone*/
		sscanf(buffer, "%s %f %f %f %f %d %s %[^\n]", depthStr, &x, &y,
			&angle, &length, &flags, name, animBuf);

		/* Avoid empty strings, but this is ineffective for invalid strings */
		if (strlen(buffer) < 3)
			continue;

		/* Calculate the depth */
		depth = strlen(depthStr) - 1;
		if (depth < 0 || depth > MAX_CHCOUNT)
		{
			fclose(file);
			fprintf(stderr, "Wrong bone depth (%s)\n", depthStr);
			return NULL;
		}

		/* If actual level is too high, go down */
		for (; actualLevel > depth; actualLevel--)
			temp = temp->parent;

		/* If no root is defined, make one at level 0 */
		if (!root && !depth)
		{
			root = boneAddChild(NULL, x, y, angle, length, flags, name);
			temp = root;
		}
		else
			temp = boneAddChild(temp, x, y, angle, length, flags, name);

		/* Now check for animation data */
		if (strlen(animBuf) > 3)
		{
			ptr = animBuf;
			while ((token = strtok(ptr, " ")))
			{
				ptr = NULL;
				sscanf(token, "%d", &time);

				token = strtok(ptr, " ");
				sscanf(token, "%f", &angle);

				token = strtok(ptr, " ");
				sscanf(token, "%f", &length);

				printf("Read %d %f %f\n", time, angle, length);

				if (temp->keyframeCount >= MAX_KFCOUNT)
				{
					fprintf(stderr, "Can't add more keyframes\n");
					continue;
				}

				k = &(temp->keyframe[temp->keyframeCount]);

				k->time = time;
				k->angle = angle;
				k->length = length;

				temp->keyframeCount++;
			}
		}

		
		/* Since the boneAddChild returns child's address, we go up a level in the hierarchy */
		actualLevel++;
	}

	fclose(file);

	return root;
}

void boneAnimate(Bone *root, int time)
{
	uint32_t i;

	float ang,
		len,
		tim;

	/* Check for keyframes */
	for (i = 0; i < root->keyframeCount; i++)			
		if (root->keyframe[i].time == time)
		{
			/* Find the index for the interpolation */
			if (i != root->keyframeCount - 1)
			{
				tim = root->keyframe[i + 1].time - root->keyframe[i].time;
				ang = root->keyframe[i + 1].angle - root->keyframe[i].angle;
				len = root->keyframe[i + 1].length - root->keyframe[i].length;

				root->offA = ang / tim;
				root->offL = len / tim;
			}
			else
			{
				root->offA = 0;
				root->offL = 0;
			}
		}

	/* Change animation */
	root->a += root->offA;
	root->l += root->offL;

	/* Call on other bones */
	for (i = 0; i < root->childCount; i++)
		boneAnimate(root->child[i], time);
}

/* TODO: Actually this doesn't handle absolute bones */
void boneDraw(Bone *root)
{
	int i;

	glPushMatrix();

	/* Draw this bone
	* 1. Translate to coords
	* 2. Rotate the matrix
	* 3. Draw the line
	* 4. Reach the end position (translate again)
	*/
	glTranslatef(root->x, root->y, 0.0);
	glRotatef((GLfloat)(RAD2DEG*(root->a)), 0.0, 0.0, 1.0);

	glBegin(GL_LINES);

	if (!strcmp(root->name, currentName))
		glColor3f(0.0, 0.0, 1.0);
	else
		glColor3f(1.0, 0.0, 0.0);

	glVertex2f(0, 0);

	if (!strcmp(root->name, currentName))
		glColor3f(1.0, 1.0, 0.0);
	else
		glColor3f(0.0, 1.0, 0.0);

	glVertex2f(root->l, 0);

	glEnd();

	/* Translate to reach the new starting position */
	glTranslatef(root->l, 0.0, 0.0);

	/* Call function on my children */
	for (i = 0; i < root->childCount; i++)
		boneDraw(root->child[i]);

	glPopMatrix();
}

void reshape(int w, int h)
{
	glViewport(0, 0, 400, 400);
	glMatrixMode(GL_PROJECTION);
	glOrtho(-200, 200, -200, 200, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

void increaseFrameNum()
{
	if(frameNum < MAX_FRAMES)
		frameNum++;
	else
		frameNum = 0;
}

void drawScene()
{
	
	glLoadIdentity();
	
	boneDraw(root);
	if (animating)
	{
		boneAnimate(root, frameNum);
		increaseFrameNum();
	}
	glutPostRedisplay();
}

// functia de display
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawScene();

	glutSwapBuffers();
}

void processMouse(int button, int state, int x, int y)
{
	printf("mouse processed\n");
	if(button == GLUT_LEFT_BUTTON)
	{
		/* We have to translate the click since the
		*  (0, 0) point is in the middle of the screen
		* and we have to flip Y coords because in SDL
		*  it grows inversely to the OpenGL
		*/
		root->x = (float)x - 200.0;
		root->y = 200.0 - (float)y;
		glutPostRedisplay();

	}

}

Bone *boneFindByName(Bone *root, char *name)
{
	int i;
	Bone *p;

	/* No bone */
	if (!root)
		return NULL;

	/* Check this name */
	if (!strcmp(root->name, name))
		return root;

	for (i = 0; i < root->childCount; i++)
	{
		/* Search recursively */
		p = boneFindByName(root->child[i], name);

		/* Found a bone in this subtree! */
		if (p)
			return p;
	}

	/* No such bone */
	return NULL;
}

void boneListNames(Bone *root, char names[MAX_BONECOUNT][20])
{
	int i, present;

	if (!root)
		return;

	/* Check if this name is already in the list */
	present = 0;
	for (i = 0; (i < MAX_BONECOUNT) && (names[i][0] != '\0'); i++)
		if (!strcmp(names[i], root->name))
		{
			present = 1;
			break;
		}

	/* If itsn't present and if there is space in list */
	if (!present && (i < MAX_BONECOUNT))
	{
		strcpy(names[i], root->name);

		if (i + 1 < MAX_BONECOUNT)
			names[i + 1][0] = '\0';
	}

	/* Now fill the list with subtree's names */
	for (i = 0; i < root->childCount; i++)
		boneListNames(root->child[i], names);		
}

void processNormalKeys(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'n':
		if ((nameIndex < MAX_BONECOUNT) && (names[nameIndex][0] != 0))
			nameIndex++;
		else
			nameIndex = 0;
		break;

	case 'p':
		if (nameIndex > 0)
			nameIndex--;
		break;

	case 'd':
		printf("[FRAME]\n");
		boneDumpTree(root, 1);
		break;

	case 'a':
		animating = !animating;
		if(animating)
			printf("Animation ON\n");
		else
			printf("Animation OFF\n");	
		break;

	default:
		break;
	}
	currentName = names[nameIndex];
	glutPostRedisplay();
}

void inputKey(int key, int x, int y)
{
	Bone *p;
	switch (key) {
		case GLUT_KEY_LEFT :
			p = boneFindByName(root, currentName);
			if (p)
				p->a += 0.1;
			break;
		case GLUT_KEY_RIGHT : 
			p = boneFindByName(root, currentName);
			if (p)
				p->a -= 0.1;
			break;
		case GLUT_KEY_UP : 
			p = boneFindByName(root, currentName);
			printf("up: %f\n", p->l);
			if (p)
				p->l += 1;
			printf("up: %f\n", p->l);
			break;
		case GLUT_KEY_DOWN : 
			p = boneFindByName(root, currentName);
			if (p)
				p->l -= 1;
			break;
	}
	glutPostRedisplay();
}


int main(int argc, char **argv)
{
	int i;
	names[0][0] = '\0';

	
	//SDL_Event sdlEv;
	//Uint32 sdlVideoFlags = SDL_OPENGL;
	//uint8_t quit;

	/* We need one parameter: the structure file */
	if (argc < 2)
	{
		fprintf(stderr, "This program require a filename as parameter\n");
		return EXIT_FAILURE;
	}

	/* Initialize */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Animatie");


	/*
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "SDL_Init: %d\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	atexit(SDL_Quit);
	*/

	/* Start graphic system with OGL */
	/*
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (!SDL_SetVideoMode(400, 400, 0, sdlVideoFlags))
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	*/
	root = boneLoadStructure("human.txt");

	boneListNames(root, names);

	for (i = 0; (i < MAX_BONECOUNT) && (names[i][0] != '\0'); i++)
		printf("Bone name: %s\n", names[i]);

	currentName = names[nameIndex];


	glShadeModel(GL_SMOOTH);


	glutDisplayFunc(display);
//	glutIdleFunc(display);
	glutReshapeFunc(reshape);

	glutMouseFunc(processMouse);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(inputKey);




	

	/* Application Initialization */
	

	/* Main loop */
	glutMainLoop();
	

	return EXIT_SUCCESS;
}