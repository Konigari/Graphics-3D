#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
float tilel = 1;
float tilew= 1;
float tileh= 0.2;

  int width = 600;
  int height = 600;
float mousex, mousey, startx, starty;
struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;

  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao)
  {
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed */


// /* Executed when a regular key is released */

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */

/* Executed when the mouse moves to position ('x', 'y') */
  void mouseMotion (int x, int y)
  {
    mousex=x;
    mousey=y;
  }


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
  void reshapeWindow (int width, int height)
  {
   GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
   glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
       Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 1.0f, 500.0f);

 }

//VAO *triangle, *rectangle;

VAO* Cuboid (float l, float h, float b, float color[2][3])
{
    // GL3 accepts only Triangles. Quads are not supported
    float x=l/2, y=h/2, z=b/2;
    const GLfloat vertex_buffer_data [] = {
        -x,-y,-z, // triangle 1 : begin
        -x,-y, z,
        -x, y, z,
        -x,-y,-z,//
        -x, y, z,
        -x, y,-z,
        
        x, y, z,//
        x,-y,-z,
        x, y,-z,
        x,-y,-z,//
        x, y, z,
        x,-y, z,
        
        x, y,-z,//
        x,-y,-z,
        -x,-y,-z,
        x, y,-z, // triangle 2 : begin
        -x,-y,-z,
        -x, y,-z,
        
        -x, y, z,//
        -x,-y, z,
        x,-y, z,
        x, y, z,//
        -x, y, z,
        x,-y, z,
        
        x,-y, z,//
        -x,-y,-z,
        x,-y,-z,
        x,-y, z,//
        -x,-y, z,
        -x,-y,-z,
        
        x, y, z,//
        x, y,-z,
        -x, y,-z,
        x, y, z,//
        -x, y,-z,
        -x, y, z,
    };

    const GLfloat color_buffer_data [] = {
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 2
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 4
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 2
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 4
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        
    };
    return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

  class Movable {
  public:
    bool destroyed=false;
    VAO *obj;
    float x;
    float y;
    float z;
    int color[2][3];
    float anglex,angley,anglez;

    void place(float x, float y,float z,float anglex,float angley,float anglez){
    	this->x = x;
      this->y = y;
      this->z = z;
      this->anglex = anglex ;
      this->angley = angley ;
      this->anglez = anglez ;
    }
    void rotate(float anglex,float angley,float anglez)
    {
     this->anglex += anglex ;
     this->angley += angley ;
     this->anglez += anglez ;
   }
   void draw(){
    if(!destroyed){
      Matrices.model = glm::mat4(1.0f);

      /* Render your scene */

      glm::mat4 transform = glm::translate (glm::vec3(this->x, this->y,this->z)); // glTranslatef
      glm::mat4 rotatea = glm::rotate((float)(this->anglez*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      glm::mat4 rotateb = glm::rotate((float)(this->angley*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
      glm::mat4 rotatec = glm::rotate((float)(this->anglex*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
      Matrices.model *= ((transform * rotatea) * rotateb) * rotatec;

      glm::mat4 MVP;

      glm::mat4 VP = Matrices.projection * Matrices.view;
      MVP = VP * Matrices.model; // MVP = p * V * M

      //  Don't change unless you are sure!!
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

      // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(this->obj);
    }
  }
};
class Tile: public Movable {
public:
  void first(int type){
    float color[2][3] = {{255,255,0},{0.1,0,0}};
    if(type==0){
      destroyed=true;
    }
    VAO* temp = Cuboid(tilew,tilel,tileh,color);
	  this->obj = temp;
  }
};
class Player: public Movable {
  public:
  void first(){
    float color[2][3] = {{0.5,0.5,0.5},{0.5,0.5,0.5}};
    this->obj = Cuboid(1,1,0.5,color);
  }
};

class Game{
public:
  int cam=1;
  Player player;
  Tile tiles[10][10];
  int score=0;
  void first(){
    int map[10][10] = {
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,0,0,0,0,0,0,0},
      {0,0,1,1,1,1,1,0,0,0},
      {0,0,0,0,0,0,0,0,0,0}
    };
    for( unsigned int a = 0; a < 10; a = a + 1 ){
      for(unsigned int  b = 0; b < 10; b=b+1){
        switch(map[a][b]){
          case 1:
            this->addTile(1,a,b);
            break;
          case 0:
            this->addTile(0,a,b);
            break;
        }
      }
    }
    this->player = Player();
    this->player.first();
  }
  void addTile(int type, int x, int y){
    tiles[x][y]=Tile();
    tiles[x][y].first(type);  
    tiles[x][y].place(x*tilew,y*tilel,0,0,0,0);
  };
  void draw(){
    for( unsigned int a = 0; a < 10; a = a + 1 ){
      for(unsigned int  b = 0; b < 10; b=b+1){
        this->tiles[a][b].draw();
      }
    }
    this->player.draw();
  }
  void end(){
    exit(0);
  }
};
Game game;


float camera_rotation_angle = 90;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void hold(int key){
  switch(key){
    case '=':
      break;
  }
}

bool held[256];
void holdRun(){
  for(int i =0 ; i< 265; i ++){
    if (held[i])
    {
      hold(i);
    }
  }
}
int convCoordx(int x){
  return x/640.0*8.0 - 4.0;
}
int convCoordy(int x){
  return x/480.0*8.0 - 4.0;
}
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_UP)
              //if (canon.x == x && canon.y == y)
              startx = x;
            starty = y;

                //buckets[0].x =convCoordx(x);
              //  if (y<470)canon.y = -convCoordy(y);
                //break;
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
               //if (canon.x == x && canon.y == y)
                //canon.angle--;
                break;

            }
            break;
        default:
            break;
    }
}

void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 static int prev; //not declared initially
 static float xdiff = 0;
    static float ydiff = M_PI/4;
  int now = glutGet(GLUT_ELAPSED_TIME); 
   if(!prev){
  prev = now;
 }
  int timeelapsed = now - prev;
  if (timeelapsed > 5)
  {
    //printf("Score: %d\n", game.score);
    prev = now;
    holdRun();
    
    /* code */
  }
  game.draw();
  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
//  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  //glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  //glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
float eyex=0,eyey=0,eyez =0,tarx=0,tary=0,tarz=0;
  switch (game.cam) {
      case 0:
          // cout << "TOWER VIEW" << endl;
          // Eye - Location of camera. Don't change unless you are sure!!
          eyex=10*cos(camera_rotation_angle*M_PI/180.0f);
          eyey=-15;
          eyez=10*sin(camera_rotation_angle*M_PI/180.0f);
          // Target - Where is the camera looking at.  Don't change unless you are sure!!
          tarx=0;
          tary=0;
          tarz=0;
          // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
          break;
      case 1:
          // cout << "TOP VIEW" << endl;
          // Eye - Location of camera. Don't change unless you are sure!!
          eyex=0;
          eyey=0;
          eyez=10;            // Target - Where is the camera looking at.  Don't change unless you are sure!!
          tarx=0;
          tary=0;
          tarz=0;
          break;
      case 2:
          eyex=game.player.x;
          eyey=game.player.y;
          eyez=game.player.z;
          tarx=eyex + 1*sin(game.player.angley*M_PI/180.0f);
          tarz=eyez + 1*cos(game.player.angley*M_PI/180.0f);
          tary=eyey + 10;
          break;
      case 3:
          tarx=game.player.x;
          tary=game.player.y;
          tarz=game.player.z;
          eyex=tarx - 3*sin(game.player.angley*M_PI/180.0f);
          eyez=tarz - 3*cos(game.player.angley*M_PI/180.0f);
          eyey=tary + 10;
          break;
      case 4:
          double mos_x,mos_y;
             xdiff += mousex -  startx;
             ydiff += mousey - starty;
          tarx=0;
          tary=0;
          tarz=0;
          float heliheight = 10;
          eyex=heliheight*sin(ydiff/height)*sin(xdiff/width);
          eyez=heliheight*sin(ydiff/height)*cos(xdiff/width);
          eyey=heliheight*cos(ydiff/height);
          break;

  }
  
  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye (eyex, eyey, eyez);
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (tarx,tary,tarz);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);
  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D

  //  Don't change unless you are sure!!
  //  Swap the frame buffers
  glutSwapBuffers ();
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
  }
// void move(bool alt,int key){
// 	if(alt){
// 		if(key = )
// 	}
// }



void keyboardDown (unsigned char key, int x, int y)
{
	printf("%c\n", key);
    
    held[key] = true;
    switch (key) {
        case 'l':
        //	buckets[0].x = buckets[0].x+1;
        // case 's':
        // case 'S':
        // 	y=y-10        // case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);

        default:
            break;
    }
}

 void keyboardUp (unsigned char key, int x, int y)
{
    held[key] = false;
    if (key>='1' && key<='9')
    {
      game.cam=key-'1';
    }
    switch (key) {
        case 'c':
        case 'C':
            break;
        case 'p':
        case 'P':
            break;
         case 'x':

             // do something
             break;
         default:
             break;
    }
}

// /* Executed when a special key is pressed */
// void keyboardSpecialDown (int key, int x, int y)
// {
// }

//  Executed when a special key is released 
// void keyboardSpecialUp (int key, int x, int y)
// {
// }


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
  void initGLUT (int& argc, char** argv, int width, int height)
  {
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
      exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    // glutSpecialFunc (keyboardSpecialDown);
    // glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (true); // Ignore keys held down
  }

/* Process menu option 'op' */
  void menu(int op)
  {
    switch(op)
    {
      case 'Q':
      case 'q':
      exit(0);
    }
  }

  void addGLUTMenus ()
  {
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
  }


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
  void initGL (int width, int height)
  {
	// Create the models
	// Create and compile our GLSL program from the shaders
   programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
   Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


   reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (1.0f, 0.5f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
int main (int argc, char** argv)
{

 srand (time(NULL));

 initGLUT (argc, argv, width, height);

 addGLUTMenus ();

 initGL (width, height);
//		cout << brick.x;

int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
game.first();
glutMainLoop();
        
return 0;
}